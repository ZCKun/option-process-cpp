//
// Created by x2h1z on 4/24/21.
//

#include <iostream>
#include <iv.h>
#include "cppkafka/cppkafka.h"
#include "process.h"

inline double
m_round(double value)
{
    return std::round(value * 10000) / 10000;
}

std::vector<std::string_view>
splitSV(std::string_view strv, std::string_view delims = " ")
{
    std::vector<std::string_view> output;
    size_t first = 0;

    while (first < strv.size()) {
        const auto second = strv.find_first_of(delims, first);

        if (first != second)
            output.emplace_back(strv.substr(first, second - first));

        if (second == std::string_view::npos)
            break;

        first = second + 1;
    }

    return output;
}

/**
 * 解析 option chain
 * @param option_chain_ele option chain element 对象
 * @param month 月份
 * @param call_put 认沽/认购 (call/put)
 * @return
 */
std::vector<OptionChain>
option_chain_parser(const simdjson::dom::element &option_chain_ele,
                    const std::string &month,
                    const std::string &call_put)
{
    std::vector<OptionChain> option_chains{};

    simdjson::dom::array res;
    if (auto err = option_chain_ele[month][call_put].get(res)) {
        std::cerr << "clout not access keys, err:" << err << std::endl;
        return option_chains;
    }

    double min_expire_date = 10000000;

    for (auto ele: res) {
        auto obj = ele.get_object();

        // 找出当月合约
        double expire_date = obj["expiredate_t"].get_double();
        if (expire_date < min_expire_date)
            min_expire_date = expire_date;

        if (expire_date > min_expire_date)
            break;

        std::string cp(obj["call_put"].get_c_str());
        std::string option_code(obj["option_code"].get_c_str());
        std::string us_code(obj["us_code"].get_c_str());

        auto oc_vec = splitSV(option_code, ".");
        std::string code(oc_vec[0]);

        auto strike_price = m_round(obj["strike_price"].get_double());

        option_chains.emplace_back((cp == "认购") ? CP::CALL : CP::PUT,
                                   us_code,
                                   code,
                                   strike_price,
                                   expire_date);
    }

    return option_chains;
}

std::map<int64_t, int64_t>
get_date_time(const rapidcsv::Document &doc, int idx, bool has_ms)
{
    auto date_time_vec = doc.GetColumn<int64_t>(idx);
    std::map<int64_t, int64_t> date_time_indexes{};

    int i = 0;
    for (auto t: date_time_vec) {
        if (has_ms)
            t /= 100;
        date_time_indexes[t % 1000000] = i;
        i++;
    }

    return date_time_indexes;
}

void fetch_option_quote(const std::string &year, const std::string &month,
                        const std::vector<OptionChain> &option_chains,
                        std::map<std::string,
                                ska::flat_hash_map<int, OptionKlineItem>> &option_quote)
{
    for (const auto &option_chain : option_chains) {
        const auto &code = option_chain.option_code;

        auto sql = fmt::format("select * from option_kline_{}.`{}` where date = '{}';",
                               year, code, month);
        auto result = Sql::instance().execute_sql_command(sql);
        auto row = result.fetchAll();

        ska::flat_hash_map<int, OptionKlineItem> kline_item;

        for (const auto &item: row) {
            std::string kline_time(item[3]);
            auto k = std::stoi(kline_time);

            if (kline_item.find(k) != kline_item.end())
                continue;

            OptionKlineItem i{
                    std::string(item[1]),
                    std::string(item[2]),
                    kline_time,
                    item[6], // open price
                    item[7], // high price
                    item[8], // low price
                    item[9], // close price
                    item[10], // total position
                    item[11], // volume
                    item[12] // amount
            };
            kline_item.insert({k, i});
        }

        option_quote.insert({code, kline_item});
    }
}

Process::Process(const std::string &stock_quote_path, const std::string &year, const std::string &date,
                 const simdjson::dom::element &option_chain_ele,
                 const std::shared_ptr<cppkafka::Producer> &producer_ptr)
        : stock_quote_fp_(stock_quote_path),
          year_(year),
          date_(date),
          producer_ptr_(producer_ptr)
{
    call_option_chains_ = option_chain_parser(option_chain_ele, date_, "call");
    put_option_chains_ = option_chain_parser(option_chain_ele, date_, "put");
    if (call_option_chains_.empty() || put_option_chains_.empty()) {
        fmt::print("{} call or put option chains empty. call: {}, put:{}\n",
                   date_, call_option_chains_.size(), put_option_chains_.size());
        delete this;
    }

    if (call_option_chains_.size() != put_option_chains_.size()) {
        fmt::print("{} call option chains size not equal put. call:{}, put:{}",
                   date_, call_option_chains_.size(), put_option_chains_.size());
        delete this;
    }

    // 获取行情
    fetch_option_quote(year_, date_, call_option_chains_, call_option_quote_);
    fetch_option_quote(year_, date_, put_option_chains_, put_option_quote_);

    if (call_option_quote_.empty() || put_option_quote_.empty()) {
        fmt::print("{} call or put quote empty. call option:{}, put quote:{}\n",
                   date_, call_option_quote_.size(), put_option_quote_.size());
        delete this;
    }

    check_schema();
    option_count_ = (int) call_option_chains_.size();
}

void Process::check_schema()
{
    try {
        Sql::instance().get_schema(fmt::format("iv_{}", year_)).getTable(date_, true);
    } catch (...) {
        auto query = fmt::format("create table iv_{}.{} (id int auto_increment, Date varchar(8) null, "
                                 "Time varchar(6) null, BeginTime varchar(6) null, "
                                 "EndTime varchar(8) null, CP varchar(5) null, InstrumentID varchar(15) null, "
                                 "Strike double null, Term double null, LastPrice double null, TodayVolume double null,"
                                 "Position double null, TodayTurnover double null, Open double null, High double null,"
                                 "Low double null, Close double null, Volume double null, Turnover double null,"
                                 "RemainingDay double null, IV double null, Delta double null, Gamma double null,"
                                 "Vega double null, Theta double null, Rho double null, constraint {}_pk primary key (id) );",
                                 year_, date_, date_);
        Sql::instance().execute_sql_command(query);
    }
}

void Process::process()
{
    fmt::print("process {}\n", stock_quote_fp_);
    const auto stock_quote_doc = read_csv(stock_quote_fp_);
    auto stock_date_time_indexes = get_date_time(stock_quote_doc, 1, true);

    ska::flat_hash_map<std::string, MFPCache> mfp_cache{};
    for (int i = 0; i < option_count_; i++) {
        mfp_cache.insert(
                {call_option_chains_[i].option_code, MFPCache{0.0, 0.0}}
        );
    }

    const auto time_range = get_time_range();
    for (const auto &kline_time: time_range) {
        // 通过时间找到对应数据的索引
        auto idx = find_idx(stock_date_time_indexes, kline_time);
        assert(idx != -1);

        int hour = kline_time / 10000;
        int minute = kline_time / 100 % 100;
        int seconds = kline_time % 100;
        int kline_time_seconds = hour * 3600 + minute * 60 + seconds;

        auto stock_row = stock_quote_doc.GetRow<std::string>(idx);
        auto stock_close_price = std::stod(stock_row[2]); // close_price
        stock_close_price = m_round(stock_close_price);

        // TODO 计算合成期货价
        std::vector<double> merge_future_price_list = merge_future_price_calc(mfp_cache, kline_time);
        auto merge_future_price = merge_future_price_calc(stock_close_price, merge_future_price_list);

        auto maturity_time = effective_time_etf(kline_time_seconds) + call_option_chains_[0].expire_date;
        maturity_time /= 243;

        calc_and_save(stock_close_price, merge_future_price, maturity_time, kline_time);
    }
}

void Process::calc_and_save(double stock_close_price,
                            double merge_future_price,
                            double maturity_time,
                            int kline_time)
{

    auto sql_template = FMT_STRING(
            "('{}', '{}', '{}', '{}', {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, "
            "{}, {}),");

    for (int i = 0; i < option_count_; i++) {
        const auto &call_option_chain = call_option_chains_[i];
        const auto &put_option_chain = put_option_chains_[i];

        const auto &call_close_price = call_option_chain.close_price;
        const auto &call_strike_price = call_option_chain.strike_price;

        const auto &put_close_price = put_option_chain.close_price;
        const auto &put_strike_price = put_option_chain.strike_price;

        // iv
        auto call_iv = option_iv_calc(call_option_chain, merge_future_price, maturity_time, "c");
        auto put_iv = option_iv_calc(put_option_chain, merge_future_price, maturity_time, "c");

        // 希腊字母
        auto[delta_, gamma_, theta_, vega_] = greek_alphabet_calc(call_strike_price,
                                                                  call_close_price,
                                                                  merge_future_price,
                                                                  maturity_time,
                                                                  call_iv,
                                                                  put_iv);

        auto call_sql = fmt::format(sql_template,
                                    date_,
                                    kline_time,
                                    call_option_chain.option_code,
                                    "call",
                                    call_strike_price,
                                    call_option_chain.expire_date,
                                    call_close_price, // last price
                                    call_option_chain.total_trade_volume, // today volume
                                    call_option_chain.total_position, // position
                                    call_option_chain.total_trade_amount, // today turnover
                                    call_option_chain.open_price, // open
                                    call_option_chain.high_price, // high
                                    call_option_chain.low_price, // low
                                    call_close_price, // close
                                    call_option_chain.trade_volume, // volume
                                    call_option_chain.trade_amount, // turnover
                                    call_option_chain.expire_date, // remaining day
                                    m_round(call_iv),
                                    m_round(delta_),
                                    m_round(gamma_),
                                    m_round(theta_),
                                    m_round(vega_)
        );
        producer_ptr_->produce(cppkafka::MessageBuilder("option-data")
                                       .partition(0)
                                       .payload(call_sql));
//        sql_ += call_sql;

        auto put_sql = fmt::format(sql_template,
                                   date_,
                                   kline_time,
                                   put_option_chain.option_code,
                                   "put",
                                   put_strike_price,
                                   put_option_chain.expire_date,
                                   put_close_price, // last price
                                   put_option_chain.total_trade_volume, // today volume
                                   put_option_chain.total_position, // position
                                   put_option_chain.total_trade_amount, // today turnover
                                   put_option_chain.open_price, // open
                                   put_option_chain.high_price, // high
                                   put_option_chain.low_price, // low
                                   put_close_price, // close
                                   put_option_chain.trade_volume, // volume
                                   put_option_chain.trade_amount, // turnover
                                   put_option_chain.expire_date, // remaining day
                                   put_iv,
                                   delta_,
                                   gamma_,
                                   theta_,
                                   vega_
        );
        producer_ptr_->produce(cppkafka::MessageBuilder("option-data")
                                       .partition(0)
                                       .payload(put_sql));
//        sql_ += put_sql;

        fmt::print(
                "DATE:{}, TIME:{}, C_CODE:{}, P_CODE:{}, SPOT_PRICE:{}, CALL_IV:{}%, PUT_IV:{}%, delta:{}, gamma:{}, theta:{}, vega:{}\n",
                date_,
                kline_time,
                call_option_chain.option_code,
                put_option_chain.option_code,
                stock_close_price,
                call_iv,
                put_iv,
                delta_,
                gamma_,
                theta_,
                vega_
        );
    }
//    sql_ = sql_.substr(0, sql_.size() - 1);
//    sql_ += ";";
//
//    try {
//        Sql::instance().execute_sql_command(sql_);
//    } catch (...) {
//        fmt::print("[{}] error sql: \n{}\n", __LINE__, sql_);
//        exit(1);
//    }
}

std::vector<double>
Process::merge_future_price_calc(ska::flat_hash_map<std::string, MFPCache> &mfp_cache, const int &kline_time)
{
    std::vector<double> merge_future_price_list{};
    // TODO 如果call或put其中一个在当前时间没有kline数据，用上一根bar的数据

    for (int i = 0; i < option_count_; i++) {
        auto &call_option_chain = call_option_chains_[i];
        auto &put_option_chain = put_option_chains_[i];

        // call kline
        const auto &call_option_code = call_option_chain.option_code;
        const auto &call_kline_items = call_option_quote_.at(call_option_code);

        auto &mc = mfp_cache.at(call_option_code);

        // put kline
        const auto &put_option_code = put_option_chain.option_code; // a
        const auto &put_kline_items = put_option_quote_.at(put_option_code); // b

        if (!call_kline_items.empty() && !put_kline_items.empty()) {
            try {
                const auto &call_kline_item = call_kline_items.at(kline_time);
                const auto &put_kline_item = put_kline_items.at(kline_time);

                mc.call_open_price = call_kline_item.open_price;
                mc.call_high_price = call_kline_item.high_price;
                mc.call_low_price = call_kline_item.low_price;
                mc.call_close_price = call_kline_item.close_price;
                mc.call_trade_volume = call_kline_item.trade_volume;
                mc.call_trade_amount = call_kline_item.trade_amount;
                mc.call_total_position = call_kline_item.total_position;
                mc.call_total_trade_volume += mc.call_trade_volume;

                mc.put_open_price = put_kline_item.open_price;
                mc.put_high_price = put_kline_item.high_price;
                mc.put_low_price = put_kline_item.low_price;
                mc.put_close_price = put_kline_item.close_price;
                mc.put_trade_volume = put_kline_item.trade_volume;
                mc.put_trade_amount = put_kline_item.trade_amount;
                mc.put_total_trade_volume += mc.put_trade_volume;
                mc.put_total_position = put_kline_item.total_position;

            } catch (std::out_of_range &e) {
                fmt::print("ERROR: 合约 {} 没有 {} 的数据. ", call_option_code, kline_time);
            }
        }

        // TODO 合成期货价格 = 认购中间价 - 认沽中间价 + 认购行权价
        auto merge_future_price = mc.call_close_price - mc.put_close_price + call_option_chain.strike_price;
        merge_future_price = std::round(merge_future_price * 100000) / 100000;
        merge_future_price_list.push_back(merge_future_price);

        call_option_chain.open_price = mc.call_open_price;
        call_option_chain.high_price = mc.call_high_price;
        call_option_chain.low_price = mc.call_low_price;
        call_option_chain.close_price = mc.call_close_price;
        call_option_chain.trade_volume = mc.call_trade_volume;
        call_option_chain.trade_amount = mc.call_trade_amount;
        call_option_chain.total_trade_amount = mc.call_total_trade_volume;
        call_option_chain.total_position = mc.call_total_position;
        put_option_chain.open_price = mc.put_open_price;
        put_option_chain.high_price = mc.put_high_price;
        put_option_chain.low_price = mc.put_low_price;
        put_option_chain.close_price = mc.put_close_price;
        put_option_chain.trade_volume = mc.put_trade_volume;
        put_option_chain.trade_amount = mc.put_trade_amount;
        put_option_chain.total_trade_amount = mc.put_total_trade_volume;
        put_option_chain.total_position = mc.put_total_position;
    }

    std::cout << "\n";

    return merge_future_price_list;
}

double
Process::merge_future_price_calc(double stock_close_price, const std::vector<double> &merge_future_price_list)
{
    std::vector<double> less_list{}, greater_list{};

    for (int i = 0; i < option_count_; i++) {
        const auto &item = call_option_chains_[i];
        const auto &strike_price = item.strike_price;
        // double strike_price = std::round(item.strike_price * 100000) / 100000;
        if (strike_price < stock_close_price) {
            less_list.push_back(merge_future_price_list[i]);
        } else if (strike_price > stock_close_price) {
            greater_list.push_back(merge_future_price_list[i]);
        }
    }

    double v1 = 0.0, v2 = 0.0, v3 = 0.0, v4 = 0.0;
    if (!less_list.empty()) {
        v1 = *(less_list.end() - 1);
        v2 = *(less_list.end() - 2);
    }
    if (!greater_list.empty()) {
        v3 = greater_list[0];
        v4 = greater_list[1];
    }

    // 合成期货价
    auto merge_future_price = (v1 + v2 + v3 + v4) / 4;

    return m_round(merge_future_price);
}

double
Process::option_iv_calc(const OptionChain &option,
                        double merge_future_price,
                        double maturity_time,
                        const std::string &call_put)
{
    double close_price = m_round(option.close_price);

    auto iv = implied_volatility_calc(merge_future_price, // s
                                      option.strike_price, // k
                                      maturity_time, // t
                                      .0,
                                      .0,
                                      close_price,  // target
                                      call_put);

    iv = m_round(iv * 100);
    return iv;
}

std::tuple<double &, double &, double &, double &>
Process::greek_alphabet_calc(double call_strike_price,
                             double call_close_price,
                             double merge_future_price,
                             double maturity_time,
                             double call_iv,
                             double put_iv)
{
    // 虚值波动率
    auto vol = (call_strike_price > merge_future_price) ? call_iv : put_iv;

    // 希腊字母
    auto c_delta = m_round(delta(merge_future_price, call_strike_price, maturity_time, .0, .0, vol, "c"));
    auto c_gamma = m_round(gamma(merge_future_price, call_strike_price, maturity_time, .0, .0, vol));
    auto c_theta = m_round(
            theta(merge_future_price, call_strike_price, maturity_time, .0, .0, vol, call_close_price, "c"));
    auto c_vega = m_round(vega(merge_future_price, call_strike_price, maturity_time, .0, .0, vol));

    return std::tie(c_delta, c_gamma, c_theta, c_vega);
}


inline int64_t
Process::find_idx(std::map<int64_t, int64_t> &m, int value)
{
    for (auto it = m.rbegin(); it != m.rend(); it++) {
        if (it->first < value)
            return it->second;
    }
    return -1;
}

Process::~Process()
{
    producer_ptr_->flush();
}


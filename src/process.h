//
// Created by x2h1z on 4/24/21.
//

#ifndef OPTION_PROCESS_PROCESS_H
#define OPTION_PROCESS_PROCESS_H

#include <cmath>
#include <vector>
#include <string>
#include <map>
#include <fmt/format.h>
#include "simdjson.h"
#include "helper/Sql.h"
#include "flat_hash_map/flat_hash_map.hpp"
#include "data_struct.h"
#include "utils.h"

using namespace op;
using namespace op::util;

class Process
{
public:
    Process(const std::string &spot_file_path, const std::string &year, const std::string &date,
            const simdjson::dom::element &option);

    ~Process();

    static inline double
    m_round(double value)
    {
        return std::round(value * 10000.0) / 10000.0;
    }

    static double
    option_iv_calc(const OptionChain &option,
                   double merge_future_price,
                   double maturity_time,
                   const std::string &call_put);

    static inline std::tuple<double &, double &, double &, double &>
    greek_alphabet_calc(double call_strike_price,
                        double call_close_price,
                        double merge_future_price,
                        double maturity_time,
                        double call_iv,
                        double put_iv);

    /**
     * 根据时间查找索引
     * @tparam T
     * @param m
     * @param value
     * @return
     */
    static inline int64_t
    find_idx(std::map<int64_t, int64_t> &m, int value);

    void start();

private:
    void process();

    bool init();

    void check_schema();

    void calc_and_save(double stock_close_price,
                       double merge_future_price,
                       double maturity_time,
                       int kline_time);

    std::vector<double>
    merge_future_price_calc(ska::flat_hash_map<std::string, MFPCache> &mfp_cache, const int &kline_time);

    double merge_future_price_calc(double stock_close_price, const std::vector<double> &merge_future_price_list);

    void fetch_option_quote(const std::vector<OptionChain> &option_chains,
                            std::map<std::string, ska::flat_hash_map<int, OptionKlineItem>> &option_quote);

    std::string stock_quote_fp_, year_, date_;
    std::vector<OptionChain> call_option_chains_, put_option_chains_;
    std::map<std::string, ska::flat_hash_map<int, OptionKlineItem>>
            call_option_quote_, put_option_quote_;

    int option_count_;
    std::string sql_ = fmt::format(FMT_STRING(
                                           "insert into iv_{}.`{}` (`Date`,`Time`,`InstrumentID`,`CP`,`Strike`,`Term`,`LastPrice`,`TodayVolume`,"
                                           "`Position`,`TodayTurnover`,`Open`,`High`,`Low`,`Close`,`Volume`,`Turnover`,`RemainingDay`,`IV`,`Delta`,`Gamma`,`theta`,"
                                           "`vega`) values "), year_, date_);
    std::string sql_template_;
    std::mutex mutex_;
};


#endif //OPTION_PROCESS_PROCESS_H

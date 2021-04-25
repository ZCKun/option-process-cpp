//
// Created by x2h1z on 4/24/21.
//

#ifndef OPTION_PROCESS_DATA_STRUCT_H
#define OPTION_PROCESS_DATA_STRUCT_H

namespace op {
    enum class CP
    {
        CALL, PUT
    };

    struct OptionChain
    {
        // 认购/认沽
        CP call_put;
        // 标的代码
        std::string us_code{};
        // 期权代码
        std::string option_code{};

        // 行权价
        double strike_price{};
        // 当日收盘价
        double close_price{};
        double open_price{};
        double high_price{};
        double low_price{};
        double total_position{};
        double total_trade_volume{};
        double trade_volume{};
        double trade_amount{};
        double total_trade_amount{};

        // 距离到期剩余天数
        double expire_date{};

        OptionChain(const CP &cp, std::string us_code, std::string option_code,
                    double strike_price, double expire_date)
                : call_put(cp),
                  us_code(std::move(us_code)),
                  option_code(std::move(option_code)),
                  strike_price(strike_price),
                  expire_date(expire_date)
        {}
    };

    struct OptionKlineItem
    {
        std::string symbol;
        std::string date;
        std::string k_time;
        double open_price;
        double high_price;
        double low_price;
        double close_price;
        double total_position;
        double trade_volume;
        double trade_amount;

        OptionKlineItem(std::string symbol, std::string date, std::string k_time,
                        double open, double high, double low, double close,
                        double total_position, double volume, double amount)
                : symbol(std::move(symbol)),
                  date(std::move(date)),
                  k_time(std::move(k_time)),
                  open_price(open),
                  high_price(high),
                  low_price(low),
                  close_price(close),
                  total_position(total_position),
                  trade_volume(volume),
                  trade_amount(amount)
        {}
    };


    struct MFPCache
    {
        int time{};
        double call_close_price{};
        double call_open_price{};
        double call_high_price{};
        double call_low_price{};
        double call_total_position{};
        double call_total_trade_volume{};
        double call_trade_volume{};
        double call_trade_amount{};
        double put_close_price{};
        double put_open_price{};
        double put_high_price{};
        double put_low_price{};
        double put_total_position{};
        double put_total_trade_volume{};
        double put_trade_volume{};
        double put_trade_amount{};

        MFPCache(double call_close_price, double put_close_price)
                : call_close_price(call_close_price),
                  put_close_price(put_close_price)
        {}
    };

}
#endif //OPTION_PROCESS_DATA_STRUCT_H

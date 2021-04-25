//
// Created by x2h1z on 4/14/21.
//

#ifndef OPTION_PROCESS_IV_H
#define OPTION_PROCESS_IV_H

#include <cmath>

inline double
get_d2(double t, double vol, double d1)
{
    auto d2 = d1 - vol * sqrt(t);
    return d2;
}

inline double
get_d1(double s, double k, double t, double b, double vol)
{
    auto d1 = (log(s / k) + (b + std::pow(vol, 2) / 2) * t) / (std::max(vol, 0.0000000000001) * sqrt(t));
    return d1;
}

inline double
normal_cdf(double value)
{
    return 0.5 * std::erfc(-value * M_SQRT1_2);
}

/**
 * 标准正太分布概率密度
 * @param value
 * @return
 */
inline double
nd(double value)
{
    return (1.0 / std::sqrt(2.0 * M_PI)) * std::exp(-std::pow(value, 2) / 2.0);
}

/**
 * black scholes 模型
 * @param spot_price 标的资产价格
 * @param strike_price 行权价
 * @param maturity_time 合约到期时间
 * @param risk_free_rate 无风险利率
 * @param dividend 股票红利率
 * @param vol
 * @param call_put_flag 认沽/认购
 * @return
 */
inline double
black_scholes_calc(double spot_price, double strike_price, double maturity_time, double risk_free_rate,
                   double dividend, double vol, const std::string &call_put_flag)
{
    double bs = 0.0;

    auto d1 = get_d1(spot_price, strike_price, maturity_time, dividend, vol);
    auto d2 = get_d2(maturity_time, vol, d1);

    if (call_put_flag == "c") {
        bs = spot_price * std::exp((dividend - risk_free_rate) * maturity_time) * normal_cdf(d1)
             - strike_price * std::exp(-risk_free_rate * maturity_time) * normal_cdf(d2);
    } else if (call_put_flag == "p") {
        bs = strike_price * std::exp(-risk_free_rate * maturity_time) * normal_cdf(-d2)
             - spot_price * std::exp((dividend - risk_free_rate) * maturity_time) * normal_cdf(-d1);
    }

    return bs;
}

/**
 * 隐含波动率计算
 * @param spot_price 标的资产价格
 * @param strike_price 行权价
 * @param maturity_time 合约到期时间
 * @param risk_free_rate 无分析利率
 * @param dividend 股票红利率
 * @param target 中间价
 * @param call_put_flag 认沽/认购
 * @return
 */
double implied_volatility_calc(double spot_price, double strike_price, double maturity_time, double risk_free_rate,
                               double dividend, double target, const std::string &call_put_flag)
{
    double high = 5.0;
    double low = 0.0;

    while (high - low > 0.00000001) {
        auto vol = (high + low) / 2;
        auto bs = black_scholes_calc(spot_price, strike_price, maturity_time, risk_free_rate, dividend, vol,
                                     call_put_flag);
        if (bs > target)
            high = vol;
        else
            low = vol;
    }

    return (high + low) / 2;
}

/**
 * delta
 * @param s
 * @param k
 * @param t
 * @param r
 * @param b
 * @param vol
 * @param call_put
 * @return
 */
inline double
delta(double s, double k, double t, double r, double b, double vol, const std::string &call_put)
{
    double delta = 0.0;

    auto d1 = get_d1(s, k, t, b, vol);
    if (call_put == "c") {
        delta = std::exp((b - r) * t) * normal_cdf(d1);
    } else if (call_put == "p") {
        delta = std::exp((b - r) * t) * (normal_cdf(d1) - 1);
    }

    return delta;
}

/**
 * gamma
 * @param s
 * @param k
 * @param t
 * @param r
 * @param b
 * @param vol
 * @return
 */
inline double
gamma(double s, double k, double t, double r, double b, double vol)
{
    auto d1 = get_d1(s, k, t, b, vol);
    return std::exp((b - r) * t) * nd(d1) / (s * std::max(vol, 0.0000000000001) * std::sqrt(t));
}

/**
 * vega
 * @param s
 * @param k
 * @param t
 * @param r
 * @param b
 * @param vol
 * @return
 */
inline double
vega(double s, double k, double t, double r, double b, double vol)
{
    auto d1 = get_d1(s, k, t, b, vol);
    return s * std::exp((b - r) * t) * std::sqrt(t) * nd(d1);
}

/**
 * theta
 * @param s
 * @param k
 * @param t
 * @param r
 * @param b
 * @param vol
 * @param target
 * @param cp
 * @return
 */
inline double
theta(double s, double k, double t, double r, double b, double vol, double target, const std::string &cp)
{
    double theta = .0;
    double d1 = get_d1(s, k, t, b, vol);
    double d2 = get_d2(t, vol, d1);
    if (cp == "c")
        theta = -s * std::exp((b - r) * t) * nd(d1) * vol / (2 * sqrt(t)) -
                (b - r) * s * std::exp((b - r) * t) * normal_cdf(d1) - r * k * std::exp(-r * t) * normal_cdf(d2);
    else if (cp == "p")
        theta = -s * std::exp(-b * t) * nd(d1) * vol / (2 * sqrt(t)) -
                (b - r) * s * std::exp((b - r) * t) * normal_cdf(-d1) - r * k * std::exp(-r * t) * normal_cdf(-d2);

    if ((theta * t + target) > 0)
        return theta;
    int i = 1 / 246;
    return (black_scholes_calc(s, k, t - i, r, b, vol, cp) - black_scholes_calc(s, k, t, r, b, vol, cp)) * 246;
}

/**
 * rho
 * @param s
 * @param k
 * @param t
 * @param r
 * @param b
 * @param vol
 * @param cp
 * @return
 */
inline double
rho(double s, double k, double t, double r, double b, double vol, const std::string &cp)
{
    double rho = .0;

    auto d1 = get_d1(s, k, t, b, vol);
    auto d2 = get_d2(t, vol, d1);

    if (cp == "c")
        rho = k * t * std::exp(-r * t) * normal_cdf(d2);
    else if (cp == "p")
        rho = -k * t * std::exp(-r * t) * normal_cdf(-d2);

    return rho;
}

inline double
effective_time_etf(double t)
{
    double ret = .0;
    t /= 1000;

    auto t1 = 9 * 3.6 + 30 * 0.06;
    auto t2 = 11 * 3.6 + 30 * 0.06;
    auto t3 = 13 * 3.6;
    auto t4 = 15 * 3.6;

    auto total_time = 4 * 3.6;

    if (t >= t1 && t < t3)
        ret = 1 - std::min(t - t1, t2 - t1) / total_time;
    else if (t >= t3 && t < t4)
        ret = 1 - (t - t3 + 2 * 3.6) / total_time;
    else if (t >= t4)
        ret = 0;
    else if (t < t1)
        ret = 1;

    return ret;
}


#endif //OPTION_PROCESS_IV_H

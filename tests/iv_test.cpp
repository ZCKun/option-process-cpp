//
// Created by x2h1z on 4/19/21.
//
#include <gtest/gtest.h>
#include "iv.h"

inline double
m_round(double value)
{
    return std::round(value * 10000) / 10000;
}

TEST(IVTest, IndexTest)
{
    double s = 3.44516;
    double k = 3.1;
    double t = 0.040323788;
    double r = .0;
    double b = .0;
    double target = 0.34645;
    double vol = 27.65;

    EXPECT_EQ(m_round(implied_volatility_calc(s, k, t, r, b, target, "c")), 0.2577);
    EXPECT_EQ(m_round(implied_volatility_calc(s, k, t, r, b, target, "p")), 1.9215);

    EXPECT_EQ(m_round(rho(s, k, t, 0, 0, vol, "c")), 0.0004);
    EXPECT_EQ(m_round(rho(s, k, t, 0, 0, vol, "p")), -0.1246);

    EXPECT_EQ(m_round(delta(s, k, t, 0, 0, vol, "c")), 0.9974);
    EXPECT_EQ(m_round(delta(s, k, t, 0, 0, vol, "p")), -0.0026);

    EXPECT_EQ(m_round(theta(s, k, t, r, b, vol, target, "c")), -1.9029);
    EXPECT_EQ(m_round(theta(s, k, t, r, b, vol, target, "p")), -1.9029);

    EXPECT_EQ(m_round(vega(s, k, t, 0, 0, vol)),  0.0056);
    EXPECT_EQ(m_round(gamma(s, k, t, 0, 0, vol)),  0.0004);

    EXPECT_EQ(m_round(effective_time_etf(47721.0)), 0.4360);
}

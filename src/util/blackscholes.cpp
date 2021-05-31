/**
 * @file blackscholes.cpp
 *
 * @copyright Copyright (C) 2021 Randy Blankley. All rights reserved.
 *
 * @section LICENSE
 *
 * This file is part of mofo.
 *
 * Money4Options is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * @note
 * This code adapted from libmetaoptions - A collection of option-related functions.
 * Copyright (C) 2000-2004 B. Augestad, bjorn.augestad@gmail.com
 */

#include "bisection.h"
#include "blackscholes.h"

#include <cmath>

static const double one_div_sqrt2pi = 0.39894228040143270286;

#define pow2(n) ((n) * (n))
#define normdist(x) ( one_div_sqrt2pi * exp(-(((x) * (x))/ 2.0)))

double cnd( double x );

///////////////////////////////////////////////////////////////////////////////////////////////////
BlackScholes::BlackScholes( double S, double r, double b, double sigma, double T ) :
    _Mybase( S, r, b, sigma, T )
{
    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BlackScholes::optionPrice( OptionType type, double X ) const
{
    const double d1( (log( S_ / X ) + (b_ + pow2( sigma_ ) / 2.0) * T_) / vst_ );
    const double d2( d1 - vst_ );

    double result;

    if ( OptionType::Call == type )
        result = S_ * ebrt_ * cnd( d1 )
               - X  * ert_  * cnd( d2 );
    else
        result = X  * ert_  * cnd( -d2 )
               - S_ * ebrt_ * cnd( -d1 );

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BlackScholes::partials( OptionType type, double X, double& delta, double& gamma, double& theta, double& vega, double& rho ) const
{
    const double d1( (log( S_ / X ) + (b_ + pow2( sigma_ ) / 2.0) * T_) / vst_ );
    const double d2( d1 - vst_ );

    const double normdist_d1( normdist( d1 ) );

    // ---- //

    gamma = ebrt_ * normdist_d1 / (S_ * vst_);

    vega = S_ * ebrt_ * normdist_d1 * st_;

    if ( OptionType::Call == type )
    {
        const double cnd_d1( cnd( d1 ) );
        const double cnd_d2( cnd( d2 ) );

        delta = ebrt_ * cnd_d1;

        theta = -sbrt_ * normdist_d1 * sigma_ / (2.0 * st_)
              - (b_ - r_) * sbrt_ * cnd_d1
              - r_ * X * ert_ * cnd_d2;

        rho = T_ * X * ert_ * cnd_d2;
    }
    else
    {
        const double cnd_d2( cnd( -d2 ) );

        delta = ebrt_ * (cnd( d1 ) - 1.0);

        theta = -sbrt_ * normdist_d1 * sigma_ / (2.0 * st_)
              + (b_ - r_) * sbrt_ * cnd( -d1 )
              + r_ * X * ert_ * cnd_d2;

        rho = -T_ * X * ert_ * cnd_d2;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BlackScholes::setSigma( double value )
{
    sigma_ = value;

    // re-calc vst
    vst_ = sigma_ * st_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BlackScholes::vega( OptionType /*type*/, double X ) const
{
    const double d1( (log( S_ / X ) + (b_ + pow2( sigma_ ) / 2.0) * T_) / vst_ );

    const double normdist_d1( normdist( d1 ) );

    // ---- //

    return (S_ * ebrt_ * normdist_d1 * st_);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BlackScholes::copy( const _Myt& other )
{
    _Mybase::copy( other );

    st_ = other.st_;
    vst_ = other.vst_;

    ebrt_ = other.ebrt_;
    sbrt_ = other.sbrt_;

    ert_ = other.ert_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BlackScholes::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    st_ = std::move( other.st_ );
    vst_ = std::move( other.vst_ );

    ebrt_ = std::move( other.ebrt_ );
    sbrt_ = std::move( other.sbrt_ );

    ert_ = std::move( other.ert_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BlackScholes::init()
{
    st_ = sqrt( T_ );
    vst_ = sigma_ * st_;

    ebrt_ = exp( (b_ - r_) * T_ );
    sbrt_ = S_ * ebrt_;

    ert_ = exp( -r_ * T_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.0001 <= result && result <= v+0.0001 );}

void BlackScholes::validate()
{
    double delta, gamma, theta, vega, rho;

    // price
    _Myt test1( 75.0, 0.10, 0.05, 0.35, 0.5 );
    _Myt test2( 60.0, 0.08, 0.08, 0.30, 0.25 );
    _Myt test3( 96.1469, 0.10, 0.10, 0.25, 0.75 );

    Q_ASSERT_DOUBLE( test1.optionPrice( OptionType::Put, 70.0 ), 4.0870 );
    Q_ASSERT_DOUBLE( test2.optionPrice( OptionType::Call, 65.0 ), 2.1334 );
    Q_ASSERT_DOUBLE( test3.optionPrice( OptionType::Call, 90.0 ), 15.6465 );

    // delta
    _Myt d_test1( 105.0, 0.10, 0.0, 0.36, 0.5 );
    _Myt d_test2( 10.0, 0.04, 0.04, 0.2, 0.75 );

    d_test1.partials( OptionType::Call, 100.0, delta, gamma, theta, vega, rho );
    Q_ASSERT_DOUBLE( delta, 0.5946 );

    d_test1.partials( OptionType::Put, 100.0, delta, gamma, theta, vega, rho );
    Q_ASSERT_DOUBLE( delta, -0.3566 );

    d_test2.partials( OptionType::Put, 100.0, delta, gamma, theta, vega, rho );
    Q_ASSERT_DOUBLE( delta, -1.0 );

    // gamma
    _Myt g_test1( 55.0, 0.10, 0.10, 0.30, 0.75 );

    g_test1.partials( OptionType::Call, 60.0, delta, gamma, theta, vega, rho );
    Q_ASSERT_DOUBLE( gamma, 0.0278 );

    g_test1.partials( OptionType::Put, 60.0, delta, gamma, theta, vega, rho );
    Q_ASSERT_DOUBLE( gamma, 0.0278 );

    // theta
    _Myt t_test1( 430.0, 0.07, 0.02, 0.20, 1.0 / 12.0 );

    t_test1.partials( OptionType::Put, 405.0, delta, gamma, theta, vega, rho );
    Q_ASSERT_DOUBLE( theta, -31.1924 );

    // vega
    _Myt v_test1( 55.0, 0.10, 0.10, 0.30, 0.75 );

    v_test1.partials( OptionType::Call, 60.0, delta, gamma, theta, vega, rho );
    Q_ASSERT_DOUBLE( vega, 18.9358 );

    v_test1.partials( OptionType::Put, 60.0, delta, gamma, theta, vega, rho );
    Q_ASSERT_DOUBLE( vega, 18.9358 );

    // rho
    _Myt r_test1( 72.0, 0.09, 0.09, 0.19, 1.0 );

    r_test1.partials( OptionType::Call, 75.0, delta, gamma, theta, vega, rho );
    Q_ASSERT_DOUBLE( rho, 38.7325 );

    // futures
    _Myt fut_test1( 70.0, 0.05, 0.0, 0.28, 3.0 / 12.0 );
    _Myt fut_test2( 19.0, 0.10, 0.0, 0.28, 0.75 );

    Q_ASSERT_DOUBLE( fut_test1.optionPrice( OptionType::Put, 70.0 ), 3.8579 );

    Q_ASSERT_DOUBLE( fut_test2.optionPrice( OptionType::Call, 19.0 ), 1.7011 );
    Q_ASSERT_DOUBLE( fut_test2.optionPrice( OptionType::Put, 19.0 ), 1.7011 );

    // imp vol bisections
    const double bisect_vi = 0.35;

    _Myt bisect_test1( 70.0, 0.10, 0.05, bisect_vi, 0.5 );
    const double bisect_price = bisect_test1.optionPrice( OptionType::Put, 70.0 );

    Q_ASSERT_DOUBLE( Bisection::calcImplVol( bisect_test1, OptionType::Put, 70.0, bisect_price ), bisect_vi );
}
#endif


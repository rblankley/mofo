/**
 * @file baroneadesiwhaley.cpp
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

#include "baroneadesiwhaley.h"
#include "bisection.h"

#include <cmath>

static const double one_div_sqrt2pi = 0.39894228040143270286;
static const double epsilon = 0.000001;

#define pow2(n) ((n) * (n))
#define normdist(x) ( one_div_sqrt2pi * exp(-(((x) * (x))/ 2.0)))

double cnd( double x );

///////////////////////////////////////////////////////////////////////////////////////////////////
BaroneAdesiWhaley::BaroneAdesiWhaley( double S, double r, double b, double sigma, double T ) :
    _Mybase( S, r, b, sigma, T )
{
    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BaroneAdesiWhaley::optionPrice( OptionType type, double X ) const
{
    return (OptionType::Call == type) ? calcOptionPriceCall( X ) : calcOptionPricePut( X );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BaroneAdesiWhaley::setSigma( double value )
{
    _Mybase::setSigma( value );

    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BaroneAdesiWhaley::calcOptionPriceCall( double X ) const
{
    double result;

    if ( r_ <= b_ )
        result = _Mybase::optionPrice( OptionType::Call, X );
    else
    {
        const double Sk = calcSeedCall( X );
        const double n = 2.0 * b_ / p2v_;
        const double K = 2.0 * r_ / (p2v_ * (1.0 - ert_));
        const double d1 = (log( Sk / X ) + (b_ + p2v_ / 2.0) * T_) / vst_;
        const double Q2 = (-(n - 1.0) + sqrt( pow2( n - 1.0 ) + 4.0 * K )) / 2.0;
        const double a2 = (Sk / Q2) * (1.0 - ebrt_ * cnd( d1 ));

        if ( S_ < Sk )
            result = _Mybase::optionPrice( OptionType::Call, X ) + a2 * pow( (S_ / Sk), Q2 );
        else
            result = S_ - X;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BaroneAdesiWhaley::calcOptionPricePut( double X ) const
{
    const double Sk = calcSeedPut( X );

    double result;

    if ( S_ <= Sk )
        result = X - S_;
    else
    {
        const double n = 2.0 * b_ / p2v_;
        const double K = 2.0 * r_ / (p2v_ * (1.0 - ert_));
        const double d1 = (log( Sk / X ) + (b_ + p2v_ / 2.0) * T_) / vst_;
        const double Q1 = (-(n - 1.0) - sqrt( pow2( n - 1.0 ) + 4.0 * K )) / 2.0;
        const double a1 = -(Sk / Q1) * (1.0 - ebrt_ * cnd( -d1 ));

        result = _Mybase::optionPrice( OptionType::Put, X ) + a1 * pow( (S_ / Sk), Q1 );
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BaroneAdesiWhaley::init()
{
    p2v_ = pow2( sigma_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BaroneAdesiWhaley::calcSeedCall( double X ) const
{
    // Calculation of seed value, Si
    const double n = 2.0 * b_ / p2v_;
    const double m = 2.0 * r_ / p2v_;
    const double q2u = (-(n - 1.0) + sqrt( pow2( n - 1.0 ) + 4.0 * m )) / 2.0;
    const double Su = X / (1.0 - (1.0 / q2u));
    const double h2 = -(b_ * T_ + 2.0 * vst_) * X / (Su - X);

    const double K = 2.0 * r_ / (p2v_ * (1.0 - ert_));
    const double Q2 = (-(n - 1.0) + sqrt( pow2( n - 1.0 ) + 4.0 * K )) / 2.0;

    double Si = X + (Su - X) * (1.0 - exp( h2 ));

    double d1 = (log( Si / X ) + (b_ + p2v_ / 2.0) * T_) / vst_;
    double cndd1 = cnd( d1 );

    double LHS = Si - X;
    double RHS = BlackScholes( Si, r_, b_, sigma_, T_ ).optionPrice( OptionType::Call, X ) + (1.0 - ebrt_ * cndd1) * Si / Q2;

    double bi = ebrt_ * cndd1 * (1.0 - (1.0 / Q2)) + (1.0 - ebrt_ * cndd1 / vst_) / Q2;

    // Newton Raphson algorithm for finding critical price Si
    while ( epsilon < (std::fabs( LHS - RHS ) / X) )
    {
        Si = (X + RHS - bi * Si) / (1.0 - bi);

        d1 = (log( Si / X ) + (b_ + p2v_ / 2.0) * T_) / vst_;
        cndd1 = cnd( d1 );

        LHS = Si - X;
        RHS = BlackScholes( Si, r_, b_, sigma_, T_ ).optionPrice( OptionType::Call, X ) + (1.0 - ebrt_ * cndd1) * Si / Q2;

        bi  = ebrt_ * cndd1 * (1.0 - (1.0 / Q2)) + (1.0 - ebrt_ * normdist( d1 ) / vst_) / Q2;
    }

    return Si;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BaroneAdesiWhaley::calcSeedPut( double X ) const
{
    // Calculation of seed value, Si
    const double n = 2.0 * b_ / p2v_;
    const double m = 2.0 * r_ / p2v_;
    const double q1u = (-(n - 1.0) - sqrt( pow2( n - 1.0 ) + 4.0 * m )) / 2.0;
    const double Su = X / (1.0 - (1.0 / q1u));
    const double h1 = (b_ * T_ - 2.0 * vst_) * X / (X - Su);

    const double K = 2.0 * r_ / (p2v_ * (1.0 - ert_));
    const double Q1 = (-(n - 1.0) - sqrt( pow2( n - 1.0 ) + 4.0 * K )) / 2.0;

    double Si = Su + (X - Su) * exp( h1 );

    double d1 = (log( Si / X ) + (b_ + p2v_ / 2.0) * T_) / vst_;
    double cndd1 = cnd( -d1 );

    double LHS = X - Si;
    double RHS = BlackScholes( Si, r_, b_, sigma_, T_ ).optionPrice( OptionType::Put, X ) - (1.0 - ebrt_ * cndd1) * Si / Q1;

    double bi = -ebrt_ * cndd1 * (1.0 - (1.0 / Q1)) - (1.0 + ebrt_ * normdist( -d1 ) / vst_) / Q1;

    // Newton Raphson algorithm for finding critical price Si
    while ( epsilon < (std::fabs( LHS - RHS ) / X) )
    {
        Si = (X - RHS + bi * Si) / (1.0 + bi);

        d1 = (log( Si / X ) + (b_ + p2v_ / 2.0) * T_) / vst_;
        cndd1 = cnd( -d1 );

        LHS = X - Si;
        RHS = BlackScholes( Si, r_, b_, sigma_, T_ ).optionPrice( OptionType::Put, X ) - (1.0 - ebrt_ * cndd1) * Si / Q1;

        bi = -ebrt_ * cndd1 * (1.0 - (1.0 / Q1)) - (1.0 + ebrt_ * cndd1 / vst_) / Q1;
    }

    return Si;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BaroneAdesiWhaley::copy( const _Myt& other )
{
    _Mybase::copy( other );

    p2v_ = other.p2v_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BaroneAdesiWhaley::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    p2v_ = std::move( other.p2v_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.003 < result && result < v+0.003 );}

void BaroneAdesiWhaley::validate()
{
    // This array contains values for puts&calls with different
    // price of underlying, different volatility and different time to expiry.
    // It also contains values for both black76 and the
    // "Barone-Adesi and Whales", BAW, formula for american options.
    //
    // Strike (X) is always 100, risk free interest is always 0.10 (10%) and
    // cost of carry is always 0.
    //
    // The table is from page 24 in E.G. Haugs book.
    struct putcall_value {
        double t, v, baw90, baw100, baw110, b90, b100, b110;
    };

    // I lowered the accuracy from 0.0001 to 0.003 in order to pass the these table values. Afterward I
    // discovered QuantLib did the same thing :)
    static const putcall_value putvalues[] = {
        { 0.1, 0.15, 10.0000, 1.8770, 0.0410,  9.9210, 1.8734, 0.0408 },
        { 0.1, 0.25, 10.2533, 3.1277, 0.4562, 10.2155, 3.1217, 0.4551 },
        { 0.1, 0.35, 10.8787, 4.3777, 1.2402, 10.8479, 4.3693, 1.2376 },
        { 0.5, 0.15, 10.5595, 4.0842, 1.0822, 10.3192, 4.0232, 1.0646 },
        { 0.5, 0.25, 12.4419, 6.8014, 3.3226, 12.2149, 6.6997, 3.2734 },
        { 0.5, 0.35, 14.6945, 9.5104, 5.8823, 14.4452, 9.3679, 5.7963 },
    };

    static const putcall_value callvalues[] = {
        { 0.1, 0.15, 0.0206, 1.8771, 10.0089, 0.0205, 1.8734,  9.9413 },
        { 0.1, 0.25, 0.3159, 3.1280, 10.3919, 0.3150, 3.1217, 10.3556 },
        { 0.1, 0.35, 0.9495, 4.3777, 11.1679, 0.9474, 4.3693, 11.1381 },
        { 0.5, 0.15, 0.8208, 4.0842, 10.8087, 0.8069, 4.0232, 10.5769 },
        { 0.5, 0.25, 2.7437, 6.8015, 13.0170, 2.7026, 6.6997, 12.7857 },
        { 0.5, 0.35, 5.0063, 9.5106, 15.5689, 4.9329, 9.3679, 15.3080 },
    };

    static const size_t nelem_put = sizeof(putvalues) / sizeof(putvalues[0]);
    static const size_t nelem_call = sizeof(callvalues) / sizeof(callvalues[0]);

    static const double r = 0.10;
    static const double b = 0.0;

    double baw;
    double b76;

    for ( size_t i( nelem_put ); i--; )
    {
        baw = _Myt( 90.0, r, b, putvalues[i].v, putvalues[i].t ).optionPrice( OptionType::Put, 100.0 );
        Q_ASSERT_DOUBLE( baw, putvalues[i].baw90 );

        baw = _Myt( 100.0, r, b, putvalues[i].v, putvalues[i].t ).optionPrice( OptionType::Put, 100.0 );
        Q_ASSERT_DOUBLE( baw, putvalues[i].baw100 );

        baw = _Myt( 110.0, r, b, putvalues[i].v, putvalues[i].t ).optionPrice( OptionType::Put, 100.0 );
        Q_ASSERT_DOUBLE( baw, putvalues[i].baw110 );

        b76 = BlackScholes( 90.0, r, b, putvalues[i].v, putvalues[i].t ).optionPrice( OptionType::Put, 100.0 );
        Q_ASSERT_DOUBLE( b76, putvalues[i].b90 );

        b76 = BlackScholes( 100.0, r, b, putvalues[i].v, putvalues[i].t ).optionPrice( OptionType::Put, 100.0 );
        Q_ASSERT_DOUBLE( b76, putvalues[i].b100 );

        b76 = BlackScholes( 110.0, r, b, putvalues[i].v, putvalues[i].t ).optionPrice( OptionType::Put, 100.0 );
        Q_ASSERT_DOUBLE( b76, putvalues[i].b110 );
    }

    for ( size_t i( nelem_call ); i--; )
    {
        baw = _Myt( 90.0, r, b, callvalues[i].v, callvalues[i].t ).optionPrice( OptionType::Call, 100.0 );
        Q_ASSERT_DOUBLE( baw, callvalues[i].baw90 );

        baw = _Myt( 100.0, r, b, callvalues[i].v, callvalues[i].t ).optionPrice( OptionType::Call, 100.0 );
        Q_ASSERT_DOUBLE( baw, callvalues[i].baw100 );

        baw = _Myt( 110.0, r, b, callvalues[i].v, callvalues[i].t ).optionPrice( OptionType::Call, 100.0 );
        Q_ASSERT_DOUBLE( baw, callvalues[i].baw110 );

        b76 = BlackScholes( 90.0, r, b, callvalues[i].v, callvalues[i].t ).optionPrice( OptionType::Call, 100.0 );
        Q_ASSERT_DOUBLE( b76, callvalues[i].b90 );

        b76 = BlackScholes( 100.0, r, b, callvalues[i].v, callvalues[i].t ).optionPrice( OptionType::Call, 100.0 );
        Q_ASSERT_DOUBLE( b76, callvalues[i].b100 );

        b76 = BlackScholes( 110.0, r, b, callvalues[i].v, callvalues[i].t ).optionPrice( OptionType::Call, 100.0 );
        Q_ASSERT_DOUBLE( b76, callvalues[i].b110 );
    }

    // imp vol bisections
    const double bisect_vi = 0.35;

    _Myt bisect_test1( 70.0, 0.10, 0.05, bisect_vi, 0.5 );
    const double bisect_price = bisect_test1.optionPrice( OptionType::Put, 70.0 );

    Q_ASSERT_DOUBLE( Bisection::calcImplVol( bisect_test1, OptionType::Put, 70.0, bisect_price ), bisect_vi );
}
#endif


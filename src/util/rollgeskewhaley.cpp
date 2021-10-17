/**
 * @file rollgeskewhaley.cpp
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

#include "rollgeskewhaley.h"

#include <cmath>

#define pow2(n) ((n) * (n))

double cnd( double x );
double cbnd( double a, double b, double Rho );

///////////////////////////////////////////////////////////////////////////////////////////////////
RollGeskeWhaley::RollGeskeWhaley( double S, double r, double sigma, double T, double d, double DT ) :
    _Mybase( S, r, r, sigma, T ),
    d_( d ),
    DT_( DT )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double RollGeskeWhaley::optionPrice( OptionType type, double X ) const
{
    static const double xinfinity = 100000000.0;
    static const double epsilon = 0.00001;

    // only calls are supported
    if ( OptionType::Call != type )
        return 0.0;

    // the expiry date of the option must be after the dividend payment date
    else if ( DT_ < T_ )
        return 0.0;

    const double Sx = S_ - d_ * ert_;

    // not optimal to exercise
    if ( d_ <= X * (1.0 - exp( -r_ * (DT_ - T_) )) )
        return BlackScholes( Sx, r_, b_, sigma_, DT_ ).optionPrice( OptionType::Call, X );

    double ci = BlackScholes( S_, r_, b_, sigma_, DT_ - T_ ).optionPrice( OptionType::Call, X );
    double HighS = S_;

    while (( 0 < (ci - HighS - d_ + X) ) && ( HighS < xinfinity ))
    {
        HighS = HighS * 2.0;
        ci = BlackScholes( HighS, r_, b_, sigma_, DT_ - T_ ).optionPrice( OptionType::Call, X );
    }

    if ( xinfinity < HighS )
        return BlackScholes( Sx, r_, b_, sigma_, DT_ ).optionPrice( OptionType::Call, X );

    double LowS = 0.0;
    double I = HighS * 0.5;

    ci = BlackScholes( I, r_, b_, sigma_, DT_ - T_ ).optionPrice( OptionType::Call, X );

    // search algorithm to find the critical stock price I
    while (( epsilon < std::fabs( ci - I - d_ + X ) ) && ( epsilon < (HighS - LowS) ))
    {
        if ( (ci - I - d_ + X) < 0 )
            HighS = I;
        else
            LowS = I;

       I = (HighS + LowS) / 2.0;
       ci = BlackScholes( I, r_, b_, sigma_, DT_ - T_ ).optionPrice( OptionType::Call, X );
    }

    const double vst2 = sigma_ * sqrt( DT_ );

    const double a1 = (log( Sx / X ) + (r_ + pow2( sigma_ ) / 2.0) * DT_) / vst2;
    const double a2 = a1 - vst2;
    const double b1 = (log( Sx / I ) + (r_ + pow2( sigma_ ) / 2.0) * T_) / vst_;
    const double b2 = b1 - vst_;

    const double result =
        Sx * cnd( b1 )
            + Sx * cbnd( a1, -b1, -sqrt( T_ / DT_ ) )
            - X * exp( -r_ * DT_ ) * cbnd( a2, -b2, -sqrt( T_ / DT_ ) )
            - (X - d_) * ert_ * cnd( b2 );

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RollGeskeWhaley::copy( const _Myt& other )
{
    _Mybase::copy( other );

    d_ = other.d_;
    DT_ = other.DT_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RollGeskeWhaley::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    d_ = std::move( other.d_ );
    DT_ = std::move( other.DT_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.0001 < result && result < v+0.0001 );}

void RollGeskeWhaley::validate()
{
    // RollGeskeWhaley is used for American calls on stocks with known
    // dividends. It computes the value of a call.
    const double S = 80.0;
    const double X = 82;
    const double t1 = 3.0 / 12;     // Time to expiration.
    const double T2 = 4.0 / 12;     // Time to dividend is paid
    const double D = 4.0;           // Dividend paid
    const double r = 0.06;
    const double v = 0.30;

    Q_ASSERT_DOUBLE( RollGeskeWhaley( S, r, v, t1, D, T2 ).optionPrice( OptionType::Call, X ), 4.3860 );
}
#endif


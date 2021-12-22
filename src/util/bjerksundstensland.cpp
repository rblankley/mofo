/**
 * @file bjerksundstensland.cpp
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

#include "bjerksundstensland.h"

#include <cmath>

/// Power of two (square) function.
#define pow2(n) ((n) * (n))

/// Continuous normal distribution function.
double cnd( double x );

///////////////////////////////////////////////////////////////////////////////////////////////////
BjerksundStensland::BjerksundStensland( double S, double r, double b, double sigma, double T ) :
    _Mybase( S, r, b, sigma, T )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland::optionPrice( OptionType type, double X ) const
{
    if ( OptionType::Call == type )
        return optionPriceCall( X );

    // use the Bjerksund and Stensland put-call transformation
    return _Myt( X, (r_ - b_), -b_, sigma_, T_ ).optionPrice( OptionType::Call, S_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland::optionPriceCall( double X ) const
{
    double result;

    // never optimal to exercise before maturity
    if ( r_ <= b_ )
        result = _Mybase::optionPrice( OptionType::Call, X );
    else
    {
        const double vv = pow2( sigma_ );

        const double Beta = (0.5 - b_ / vv) + sqrt( pow2( b_ / vv - 0.5 ) + 2.0 * r_ / vv );
        const double BInfinity = Beta / (Beta - 1.0) * X;
        const double B0 = fmax( X, r_ / (r_ - b_) * X );
        const double ht = -(b_ * T_ + 2.0 * sigma_ * st_) * B0 / (BInfinity - B0);
        const double I = B0 + (BInfinity - B0) * (1.0 - exp( ht ));

        if ( S_ >= I )
            result = S_ - X;
        else
        {
            const double alpha = (I - X) * pow( I, -Beta );

            result = alpha * pow( S_, Beta )
                   - alpha * phi( S_, T_, Beta, I, I, r_, b_, sigma_ )
                   + phi( S_, T_, 1.0, I, I, r_, b_, sigma_ )
                   - phi( S_, T_, 1.0, X, I, r_, b_, sigma_ )
                   - X * phi( S_, T_, 0.0,  I, I, r_, b_, sigma_ )
                   + X * phi( S_, T_, 0.0,  X, I, r_, b_, sigma_ );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland::phi( double S, double T, double gamma_val, double H, double I, double r, double b, double v )
{
    const double vst = v * sqrt(T);
    const double vv = pow2( v );

    const double lambda = (-r + gamma_val * b + 0.5 * gamma_val * (gamma_val - 1.0) * vv) * T;
    const double d = -(log( S / H ) + (b + (gamma_val - 0.5) * vv) * T) / vst;
    const double kappa = 2.0 * b / vv + (2.0 * gamma_val - 1.0);

    return exp( lambda )
        * pow( S, gamma_val )
        * (cnd( d ) - pow( I / S, kappa ) * cnd( d - 2.0 * log( I / S ) / vst ));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.003 < result && result < v+0.003 );}

void BjerksundStensland::validate()
{
    const double S = 42.0;
    const double X = 40.0;
    const double T = 0.75;
    const double r = 0.04;
    const double b = -0.04;
    const double v = 0.35;

    _Myt bs( S, r, b, v, T );

    Q_ASSERT_DOUBLE( bs.optionPrice( OptionType::Call, X ), 5.2704 );
}
#endif


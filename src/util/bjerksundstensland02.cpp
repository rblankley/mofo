/**
 * @file bjerksundstensland02.cpp
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

#include "bjerksundstensland02.h"

#include <cmath>

/// Power of two (square) function.
#define pow2(n) ((n) * (n))

/// Continuous normal distribution function.
double cnd( double x );

/// Cumulative bivariate normal distribution function.
double cbnd( double a, double b, double Rho );

///////////////////////////////////////////////////////////////////////////////////////////////////
BjerksundStensland2002::BjerksundStensland2002( double S, double r, double b, double sigma, double T ) :
    _Mybase( S, r, b, sigma, T )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland2002::optionPriceCall( double X ) const
{
    double result;

    // never optimal to exercise before maturity
    if ( r_ <= b_ )
        result = _Mybase::optionPriceCall( X );
    else
    {
        const double vv = pow2( sigma_ );

        const double beta = (0.5 - b_/vv) + std::sqrt( pow2( b_/vv - 0.5 ) + 2.0*r_/vv );
        const double Binf = (beta / (beta - 1.0)) * X;
        const double B0 = std::fmax( X, (r_ / (r_ - b_)) * X );

        const double t1 = 0.5 * (std::sqrt( 5.0 ) - 1.0)*T_;

        const double ht1 = -(b_*t1 + 2.0*sigma_*std::sqrt( t1 )) * pow2( X ) / ((Binf - B0) * B0);
        const double ht2 = -(b_*T_ + 2.0*sigma_*std::sqrt( T_ )) * pow2( X ) / ((Binf - B0) * B0);
        const double I1 = B0 + (Binf - B0) * (1.0 - std::exp( ht1 ));
        const double I2 = B0 + (Binf - B0) * (1.0 - std::exp( ht2 ));

        if ( I2 <= S_ )
            result = S_ - X;
        else
        {
            const double alpha1 = (I1 - X) * std::pow( I1, -beta );
            const double alpha2 = (I2 - X) * std::pow( I2, -beta );

            result = alpha2 * std::pow( S_, beta )
                   - alpha2 * phi( S_, t1, beta, I2, I2 )
                   +          phi( S_, t1, 1.0,  I2, I2 )
                   -          phi( S_, t1, 1.0,  I1, I2 )
                   -     X *  phi( S_, t1, 0.0,  I2, I2 )
                   +     X *  phi( S_, t1, 0.0,  I1, I2 )
                   + alpha1 * phi( S_, t1, beta, I1, I2 )
                   - alpha1 * ksi( S_, T_, beta, I1, I2, I1, t1 )
                   +          ksi( S_, T_, 1.0,  I1, I2, I1, t1 )
                   -          ksi( S_, T_, 1.0,  X,  I2, I1, t1 )
                   -      X * ksi( S_, T_, 0.0,  I1, I2, I1, t1 )
                   +      X * ksi( S_, T_, 0.0,  X,  I2, I1, t1 );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland2002::optionPricePut( double X ) const
{
    // use the Bjerksund and Stensland put-call transformation
    return _Myt( X, (r_ - b_), -b_, sigma_, T_ ).optionPriceCall( S_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland2002::ksi( double S, double T2, double gamma_val, double H, double I2, double I1, double t1 ) const
{
    const double vv( pow2( sigma_ ) );

    const double vst1( sigma_ * std::sqrt( t1 ) );
    const double b1( (b_ + (gamma_val - 0.5)*vv)*t1 );

    const double vst2( sigma_ * std::sqrt( T2 ) );
    const double b2( (b_ + (gamma_val - 0.5)*vv)*T2 );

    const double e1 = (std::log( S/I1 ) + b1) / vst1;
    const double e2 = (std::log( pow2( I2 )/(S*I1) ) + b1) / vst1;
    const double e3 = (std::log( S/I1 ) - b1) / vst1;
    const double e4 = (std::log( pow2( I2 )/(S*I1) ) - b1) / vst1;

    const double f1 = (std::log( S/H ) + b2) / vst2;
    const double f2 = (std::log( pow2( I2 )/(S*H) ) + b2) / vst2;
    const double f3 = (std::log( pow2( I1 )/(S*H) ) + b2) / vst2;
    const double f4 = (std::log( S*pow2( I1 )/( H*pow2( I2 ) ) ) + b2) / vst2;

    const double rho = std::sqrt( t1 / T2 );
    const double lambda = -r_ + gamma_val*b_ + 0.5*gamma_val*(gamma_val - 1.0)*vv;
    const double kappa = 2.0*b_/vv + (2.0*gamma_val - 1.0);

    return std::exp( lambda*T2 ) * std::pow( S, gamma_val ) *
        (                              cbnd( -e1, -f1,  rho )
          - std::pow( I2/S,  kappa ) * cbnd( -e2, -f2,  rho )
          - std::pow( I1/S,  kappa ) * cbnd( -e3, -f3, -rho )
          + std::pow( I1/I2, kappa ) * cbnd( -e4, -f4, -rho ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {Q_ASSERT( std::fabs( fn - v ) < 0.003 );}

void BjerksundStensland2002::validate()
{
    struct test_case {
        OptionType type;
        double X, S, q, r, T, v, result;
    };

    static const test_case tests[] = {
        // ATM option with very small volatility, reference value taken from R
        { OptionType::Call, 100.00, 100.00, 0.05, 0.05, 1.00,   0.0021,  0.08032314 },
        // ITM option with a very small volatility
        { OptionType::Call, 100.00, 110.00, 0.05, 0.05, 1.00,   0.0001, 10.0 },
        { OptionType::Put,  110.00, 100.00, 0.05, 0.05, 1.00,   0.0001, 10.0 },
        // ATM option with a very large volatility
        { OptionType::Put,  100.00, 110.00, 0.05, 0.05, 1.00,  10.0000, 94.89543 },
        // from "Option pricing formulas", E.G. Haug, Table 3-2
        { OptionType::Call, 100.00,  90.00, 0.10, 0.10, 0.10,   0.15,    0.0205 },
        { OptionType::Call, 100.00, 100.00, 0.10, 0.10, 0.10,   0.15,    1.8757 },
        { OptionType::Call, 100.00, 110.00, 0.10, 0.10, 0.10,   0.15,   10.0000 },
        { OptionType::Call, 100.00,  90.00, 0.10, 0.10, 0.10,   0.25,    0.3151 },
        { OptionType::Call, 100.00, 100.00, 0.10, 0.10, 0.10,   0.25,    3.1256 },
        { OptionType::Call, 100.00, 110.00, 0.10, 0.10, 0.10,   0.25,   10.3725 },
        { OptionType::Call, 100.00,  90.00, 0.10, 0.10, 0.10,   0.35,    0.9479 },
        { OptionType::Call, 100.00, 100.00, 0.10, 0.10, 0.10,   0.35,    4.3746 },
        { OptionType::Call, 100.00, 110.00, 0.10, 0.10, 0.10,   0.35,   11.1578 },
        { OptionType::Call, 100.00,  90.00, 0.10, 0.10, 0.50,   0.15,    0.8099 },
        { OptionType::Call, 100.00, 100.00, 0.10, 0.10, 0.50,   0.15,    4.0628 },
        { OptionType::Call, 100.00, 110.00, 0.10, 0.10, 0.50,   0.15,   10.7898 },
        { OptionType::Call, 100.00,  90.00, 0.10, 0.10, 0.50,   0.25,    2.7180 },
        { OptionType::Call, 100.00, 100.00, 0.10, 0.10, 0.50,   0.25,    6.7661 },
        { OptionType::Call, 100.00, 110.00, 0.10, 0.10, 0.50,   0.25,   12.9814 },
        { OptionType::Call, 100.00,  90.00, 0.10, 0.10, 0.50,   0.35,    4.9665 },
        { OptionType::Call, 100.00, 100.00, 0.10, 0.10, 0.50,   0.35,    9.4608 },
        { OptionType::Call, 100.00, 110.00, 0.10, 0.10, 0.50,   0.35,   15.5137 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.10,   0.15,   10.0000 },
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.10,   0.15,    1.8757 },
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.10,   0.15,    0.0408 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.10,   0.25,   10.2280 },
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.10,   0.25,    3.1256 },
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.10,   0.25,    0.4552 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.10,   0.35,   10.8663 },
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.10,   0.35,    4.3746 },
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.10,   0.35,    1.2383 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.50,   0.15,   10.5400 },
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.50,   0.15,    4.0628 },
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.50,   0.15,    1.0689 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.50,   0.25,   12.4097 },
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.50,   0.25,    6.7661 },
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.50,   0.25,    3.2932 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.50,   0.35,   14.6445 },
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.50,   0.35,    9.4608 },
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.50,   0.35,    5.8374 },
        { OptionType::Put,  100.00, 100.00, 0.00, 0.00, 0.50,   0.15,    4.2294 },
    };

    static const size_t ntests = sizeof(tests) / sizeof(tests[0]);

    // test each case
    for ( size_t i( ntests ); i--; )
    {
        const test_case *t( &tests[i] );

        const double b( t->r - t->q );

        const double val = _Myt( t->S, t->r, b, t->v, t->T ).optionPrice( t->type, t->X );
        Q_ASSERT_DOUBLE( val, t->result );
    }

}
#endif


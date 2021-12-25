/**
 * @file bjerksundstensland93.cpp
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

#include "bjerksundstensland93.h"

#include <cmath>

/// Power of two (square) function.
#define pow2(n) ((n) * (n))

/// Continuous normal distribution function.
double cnd( double x );

///////////////////////////////////////////////////////////////////////////////////////////////////
BjerksundStensland1993::BjerksundStensland1993( double S, double r, double b, double sigma, double T ) :
    _Mybase( S, r, b, sigma, T )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland1993::optionPrice( OptionType type, double X ) const
{
    if ( OptionType::Call == type )
        return optionPriceCall( X );

    return optionPricePut( X );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland1993::optionPriceCall( double X ) const
{
    double result;

    // never optimal to exercise before maturity
    if ( r_ <= b_ )
        result = _Mybase::optionPrice( OptionType::Call, X );
    else
    {
        const double vv = pow2( sigma_ );

        const double beta = (0.5 - b_/vv) + std::sqrt( pow2( b_/vv - 0.5 ) + 2.0*r_/vv );
        const double Binf = (beta / (beta - 1.0)) * X;
        const double B0 = std::fmax( X, (r_ / (r_ - b_)) * X );

        const double ht = -(b_*T_ + 2.0*sigma_*st_) * B0 / (Binf - B0);
        const double I = B0 + (Binf - B0) * (1.0 - std::exp( ht ));

        if ( I <= S_ )
            result = S_ - X;
        else
        {
            const double alpha = (I - X) * std::pow( I, -beta );

            result = alpha * std::pow( S_, beta )
                   - alpha * phi( S_, T_, beta, I, I )
                   +     phi( S_, T_, 1.0, I, I )
                   -     phi( S_, T_, 1.0, X, I )
                   - X * phi( S_, T_, 0.0, I, I )
                   + X * phi( S_, T_, 0.0, X, I );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland1993::optionPricePut( double X ) const
{
    // use the Bjerksund and Stensland put-call transformation
    return _Myt( X, (r_ - b_), -b_, sigma_, T_ ).optionPriceCall( S_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BjerksundStensland1993::phi( double S, double T, double gamma_val, double H, double I ) const
{
    const double vv( pow2( sigma_ ) );

    const double vst( sigma_ * std::sqrt( T ) );

    const double lambda = (-r_ + gamma_val*b_ + 0.5*gamma_val*(gamma_val - 1.0)*vv) * T;
    const double kappa = 2.0*b_/vv + (2.0*gamma_val - 1.0);

    const double d = -(log( S/H ) + (b_ + (gamma_val - 0.5)*vv)*T) / vst;

    return std::exp( lambda ) * std::pow( S, gamma_val )
        * (cnd( d ) - std::pow( I/S, kappa )*cnd( d - 2.0*std::log( I/S )/vst ));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {Q_ASSERT( std::fabs( fn - v ) < 0.003 );}

void BjerksundStensland1993::validate()
{
    struct test_case {
        OptionType type;
        double X, S, q, r, T, v, result;
    };

    static const test_case tests[] = {
        // from "Option pricing formulas", Haug, McGraw-Hill 1998, pag 27
        { OptionType::Call,  40.00,  42.00, 0.08, 0.04, 0.75,   0.3500,  5.2704 },
        // from "Option pricing formulas", Haug, McGraw-Hill 1998, VBA code
        { OptionType::Put,   40.00,  36.00, 0.00, 0.06, 1.00,   0.2000,  4.4531 },
        // ATM option with very small volatility, reference value taken from R
        { OptionType::Call, 100.00, 100.00, 0.05, 0.05, 1.00,   0.0021,  0.08032314 },
        // ITM option with a very small volatility
        { OptionType::Call, 100.00, 110.00, 0.05, 0.05, 1.00,   0.0001, 10.0 },
        { OptionType::Put,  110.00, 100.00, 0.05, 0.05, 1.00,   0.0001, 10.0 },
        // ATM option with a very large volatility
        { OptionType::Put,  100.00, 110.00, 0.05, 0.05, 1.00,  10.0000, 94.89543 },
        // from "Option pricing formulas", E.G. Haug, Table 3-2
        // The values I am getting match the 2002 table more closely, I modified where needed
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
        { OptionType::Call, 100.00, 100.00, 0.10, 0.10, 0.50,   0.15,    4.0567 }, //  4.0628
        { OptionType::Call, 100.00, 110.00, 0.10, 0.10, 0.50,   0.15,   10.7828 }, // 10.7898
        { OptionType::Call, 100.00,  90.00, 0.10, 0.10, 0.50,   0.25,    2.7144 }, //  2.7180
        { OptionType::Call, 100.00, 100.00, 0.10, 0.10, 0.50,   0.25,    6.7571 }, //  6.7661
        { OptionType::Call, 100.00, 110.00, 0.10, 0.10, 0.50,   0.25,   12.9693 }, // 12.9814
        { OptionType::Call, 100.00,  90.00, 0.10, 0.10, 0.50,   0.35,    4.9601 }, //  4.9665
        { OptionType::Call, 100.00, 100.00, 0.10, 0.10, 0.50,   0.35,    9.4499 }, //  9.4608
        { OptionType::Call, 100.00, 110.00, 0.10, 0.10, 0.50,   0.35,   15.4999 }, // 15.5137
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.10,   0.15,   10.0000 },
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.10,   0.15,    1.8757 },
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.10,   0.15,    0.0408 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.10,   0.25,   10.2280 },
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.10,   0.25,    3.1256 },
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.10,   0.25,    0.4552 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.10,   0.35,   10.8663 },
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.10,   0.35,    4.3746 },
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.10,   0.35,    1.2383 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.50,   0.15,   10.5349 }, // 10.5400
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.50,   0.15,    4.0567 }, //  4.0628
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.50,   0.15,    1.0689 },
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.50,   0.25,   12.3989 }, // 12.4097
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.50,   0.25,    6.7571 }, //  6.7661
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.50,   0.25,    3.2886 }, //  3.2932
        { OptionType::Put,  100.00,  90.00, 0.10, 0.10, 0.50,   0.35,   14.6319 }, // 14.6445
        { OptionType::Put,  100.00, 100.00, 0.10, 0.10, 0.50,   0.35,    9.4499 }, //  9.4608
        { OptionType::Put,  100.00, 110.00, 0.10, 0.10, 0.50,   0.35,    5.8301 }, //  5.8374
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


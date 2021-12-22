/**
 * @file trinomial.cpp
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
 *
 * @note
 * Also added optimizations based on code from Vinegar Hills
 * https://sites.google.com/view/vinegarhill-financelabs/binomial-lattice-framework/cox-ross-and-rubinstein/optimizing-cox-ross-and-rubinstein
 */

#include "trinomial.h"

#include <algorithm>
#include <cmath>

/// Power of two (square) function.
#define pow2(n) ((n) * (n))

// uncomment to debug calculations
//#define DEBUG_CALC

///////////////////////////////////////////////////////////////////////////////////////////////////
TrinomialTree::TrinomialTree( double S, double r, double b, double sigma, double T, size_t N, bool european ) :
    _Mybase( S, r, b, sigma, T, european ),
    N_( N )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double TrinomialTree::calcOptionPrice( bool isCall, double S, double K, double u, double d, double pu, double pd, double pm, double Df ) const
{
    const double z( isCall ? 1 : -1 );

    const size_t N2( 2 * N_ );

    // create pow tables
    std::vector<double> spowu;
    spowu.reserve( N2 + 1 );

    std::vector<double> powd;
    powd.reserve( N2 + 1 );

    for ( size_t i( 0 ); i <= N2; ++i )
    {
        const double exp( std::fmax( 0.0, (double)i - N_ ) );

        spowu.push_back( S * pow( u, exp ) );
        powd.push_back( pow( d, exp ) );
    }

    // init vector
    std::vector<double> val;
    val.reserve( N2 + 1 );

    for ( size_t i = 0; i <= N2; ++i )
    {
#ifdef DEBUG_CALC
        const double val0 = S * pow( u, std::fmax( 0.0, (double) i - N_ ) ) * pow( d, std::fmax( 0.0, (double) N_ - i ) ) - K;
        const double val1 = spowu[i] * powd[N2-i] - K;

        static const double ERROR = 0.000001;
        assert(( (val0 - ERROR) <= val1 ) && ( val1 <= (val0 + ERROR) ));
#endif
        val.push_back( fmax( 0.0, z * (spowu[i] * powd[N2-i] - K) ) );
    }

    // backward recursion through the tree
    for ( size_t j = N_; j--; )
    {
        const size_t j2( 2 * j );

        for ( size_t i = 0; i <= j2; ++i )
        {
            val[i] = Df * ((pu * val[i + 2]) + (pm * val[i + 1]) + (pd * val[i]));

            if ( isAmerican() )
            {
#ifdef DEBUG_CALC
                const double val0 = S * pow( u, std::fmax( 0.0, (double) i - j ) ) * pow( d, std::fmax( 0.0, (double) j - i ) ) - K;
                const double val1 = spowu[N_+i-j] * powd[N_+j-i] - K;

                static const double ERROR = 0.000001;
                assert(( (val0 - ERROR) <= val1 ) && ( val1 <= (val0 + ERROR) ));
#endif
                val[i] = fmax( val[i], z * (spowu[N_+i-j] * powd[N_+j-i] - K) );
            }
        }

        // track key values for partials calculation
        if ( j <= 1 )
        {
            f_[j][0] = val[0];
            f_[j][1] = val[1];
            f_[j][2] = val[2];
        }
    }

    // option price
    return val[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TrinomialTree::calcPartials( double u, double d, double& delta, double& gamma, double& theta ) const
{
    const double dt = T_ / N_;
    const double h = 0.5 * S_ * (u - d);

    delta = (f_[1][2] - f_[1][0]) / (S_ * (u - d));

    gamma =  (f_[1][2] - f_[1][1]) / (S_ * (u - 1.0));
    gamma -= (f_[1][1] - f_[1][0]) / (S_ * (1.0 - d));
    gamma /= h;

    theta = (f_[1][1] - f_[0][0]) / dt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TrinomialTree::copy( const _Myt& other )
{
    _Mybase::copy( other );

    N_ = other.N_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TrinomialTree::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    N_ = std::move( other.N_ );
}

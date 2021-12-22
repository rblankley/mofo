/**
 * @file kamradritchken.cpp
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
 */

#include "kamradritchken.h"

#include <algorithm>
#include <cmath>

/// Power of two (square) function.
#define pow2(n) ((n) * (n))

///////////////////////////////////////////////////////////////////////////////////////////////////
KamradRitchken::KamradRitchken( double S, double r, double b, double sigma, double T, size_t N, bool european  ) :
    _Mybase( S, r, b, sigma, T, N, european )
{
    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KamradRitchken::partials( OptionType type, double X, double& delta, double& gamma, double& theta, double& veg, double& rh ) const
{
    // calc partials
    calcPartials( u_, d_, delta, gamma, theta );

    // vega
    veg = vega( type, X );

    // rho
    rh = rho( type, X );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KamradRitchken::setSigma( double value )
{
    _Mybase::setSigma( value );

    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double KamradRitchken::optionPrice( OptionType type, double X ) const
{
    // calc!
    return calcOptionPrice( (OptionType::Call == type), S_, X, u_, d_, pu_, pd_, pm_, Df_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KamradRitchken::copy( const _Myt& other )
{
    _Mybase::copy( other );

    u_ = other.u_;
    d_ = other.d_;

    pu_ = other.pu_;
    pd_ = other.pd_;
    pm_ = other.pm_;

    Df_ = other.Df_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KamradRitchken::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    u_ = std::move( other.u_ );
    d_ = std::move( other.d_ );

    pu_ = std::move( other.pu_ );
    pd_ = std::move( other.pd_ );
    pm_ = std::move( other.pm_ );

    Df_ = std::move( other.Df_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void KamradRitchken::init()
{
    const double dt = T_ / N_;

    // lamda = sqrt( 3/2 )
    const double mu( b_ - 0.5 * pow2( sigma_ ) );
    const double musdt( (mu / sigma_) * sqrt( dt / 6.0 ) );

    // quantities for the tree
    u_ = exp( sigma_ * sqrt( (3.0 * dt) / 2.0 ) );
    d_ = 1.0 / u_;

    pu_ = (1.0 / 3.0) + musdt;
    pd_ = (1.0 / 3.0) - musdt;
    pm_ = (1.0 / 3.0);

    Df_ = exp( -r_ * dt );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.0001 <= result && result <= v+0.0001 );}

void KamradRitchken::validate()
{
    // Wiley, Table 4.1
    {
        const double S = 50.0;
        const double X = 50.0;
        const double r = 0.06;
        const double q = 0.03;
        const double sigma = 0.2;
        const double T = 1.0;

        _Myt kr( S, r, (r-q), sigma, T, 50 );

        const double cm0 = 4.5624;
        const double cm1 = kr.optionPrice( OptionType::Call, X );

        Q_ASSERT_DOUBLE( cm0, cm1 );
    }

    // Wiley, Table 4.1
    {
        const double S = 50.0;
        const double X = 50.0;
        const double r = 0.06;
        const double q = 0.03;
        const double sigma = 0.2;
        const double T = 1.0;

        _Myt kr( S, r, (r-q), sigma, T, 200 );

        const double cm0 = 4.5663;
        const double cm1 = kr.optionPrice( OptionType::Call, X );

        Q_ASSERT_DOUBLE( cm0, cm1 );
    }
}
#endif


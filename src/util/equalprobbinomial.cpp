/**
 * @file equalprobbinomial.cpp
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

#include "equalprobbinomial.h"

#include <cmath>

#define pow2(n) ((n) * (n))

///////////////////////////////////////////////////////////////////////////////////////////////////
EqualProbBinomialTree::EqualProbBinomialTree( double S, double r, double b, double sigma, double T, size_t N, bool european ) :
    _Mybase( S, r, b, sigma, T, N, european )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double EqualProbBinomialTree::optionPrice( OptionType type, double X ) const
{
    double r = r_;
    double q = r_ - b_;

    // quantities for the tree
    const double dt = T_ / N_;

    const double bv2dt = ((r - q) - 0.5 * pow2( sigma_ )) * dt;
    const double vsdt = sigma_ * sqrt( dt );

    const double u = exp( bv2dt + vsdt );
    const double d = exp( bv2dt - vsdt );

    const double Df = exp( -r * dt );

    // calc!
    return calcOptionPrice( (OptionType::Call == type), S_, X, u, d, 0.5, 0.5, Df );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void EqualProbBinomialTree::partials( OptionType type, double X, double& delta, double& gamma, double& theta, double& vega, double& rho ) const
{
    double diff;

    // calc partials
    const double dt = T_ / N_;

    const double bv2dt = (b_ - 0.5 * pow2( sigma_ )) * dt;
    const double vsdt = sigma_ * sqrt( dt );

    const double u = exp( bv2dt + vsdt );
    const double d = exp( bv2dt - vsdt );

    calcPartials( u, d, delta, gamma, theta );

    // vega
    diff = 0.02;

    _Myt vegaCalc( S_, r_, b_, sigma_+diff, T_, N_, european_ );
    vega = (vegaCalc.optionPrice( type, X ) - f_[0][0]) / diff;

    // rho
    const double q = r_ - b_;

    diff = 0.05;

    _Myt rhoCalc( S_, r_+diff, r_+diff-q, sigma_, T_, N_, european_ );
    rho = (rhoCalc.optionPrice( type, X ) - f_[0][0]) / diff;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.0001 <= result && result <= v+0.0001 );}

void EqualProbBinomialTree::validate()
{
    // from Hull book, figure 21.11
    const double S = 0.79;          // foreign currency value (measured in domestic)
    const double X = 0.795;         // strike
    const double r = 0.06;          // risk free rate of domestic
    const double rf = 0.10;         // risk free rate of foreign
    const double sigma = 0.04;
    const double T = 0.75;

    _Myt eqp( S, r, r-rf, sigma, T, 3 );

    const double cm0 = 0.0026;
    const double cm1 = eqp.optionPrice( OptionType::Call, X );

    Q_ASSERT_DOUBLE( cm0, cm1 );
}
#endif


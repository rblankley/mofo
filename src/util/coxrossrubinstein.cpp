/**
 * @file coxrossrubinstein.cpp
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

#include "coxrossrubinstein.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////
CoxRossRubinstein::CoxRossRubinstein( double S, double r, double b, double sigma, double T, size_t N, bool european ) :
    _Mybase( S, r, b, sigma, T, N, european )
{
    // init
    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CoxRossRubinstein::CoxRossRubinstein( double S, double r, double b, double sigma, double T, size_t N, const std::vector<double>& divTimes, const std::vector<double>& divYields, bool european ) :
    _Mybase( S, r, b, sigma, T, N, european ),
    divTimes_( divTimes ),
    div_( divYields )
{
    assert( divTimes.size() == divYields.size() );

    // init
    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double CoxRossRubinstein::optionPrice( OptionType type, double X ) const
{
    const double r = r_;
    const double q = r_ - b_;

    // quantities for the tree
    const double dt = T_ / N_;

    const double pu = (exp( (r - q) * dt ) - d_) / (u_ - d_);
    const double pd = 1.0 - pu;

    const double Df = exp( -r * dt );

    // calc!
    if ( divTimes_.size() )
        return calcOptionPrice( (OptionType::Call == type), S_, X, u_, d_, pu, pd, Df, divTimes_, div_ );

    return calcOptionPrice( (OptionType::Call == type), S_, X, u_, d_, pu, pd, Df );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CoxRossRubinstein::partials( OptionType type, double X, double& delta, double& gamma, double& theta, double& veg, double& rh ) const
{
    // calc partials
    calcPartials( u_, d_, delta, gamma, theta );

    // vega
    veg = vega( type, X );

    // rho
    rh = rho( type, X );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double CoxRossRubinstein::rho( OptionType type, double X ) const
{
    // rho
    const double diff = 0.01;
    const double q = r_ - b_;

    _Myt rhoCalc( S_, r_+diff, r_+diff-q, sigma_, T_, N_, divTimes_, div_, european_ );
    return (rhoCalc.optionPrice( type, X ) - f_[0][0]) / diff;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CoxRossRubinstein::setSigma( double value )
{
    _Mybase::setSigma( value );

    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double CoxRossRubinstein::vega( OptionType type, double X ) const
{
    // vega
    const double diff = 0.02;

    _Myt vegaCalc( S_, r_, b_, sigma_+diff, T_, N_, divTimes_, div_, european_ );
    return (vegaCalc.optionPrice( type, X ) - f_[0][0]) / diff;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CoxRossRubinstein::copy( const _Myt& other )
{
    _Mybase::copy( other );

    divTimes_ = other.divTimes_;
    div_ = other.div_;

    u_ = other.u_;
    d_ = other.d_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CoxRossRubinstein::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    divTimes_ = std::move( other.divTimes_ );
    div_ = std::move( other.div_ );

    u_ = std::move( other.u_ );
    d_ = std::move( other.d_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CoxRossRubinstein::init()
{
    const double dt = T_ / N_;

    u_ = exp( sigma_ * sqrt( dt ) );
    d_ = 1.0 / u_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.0001 <= result && result <= v+0.0001 );}

void CoxRossRubinstein::validate()
{
    {
        // from Hull book, figure 13.10
        const double S = 50;
        const double X = 52;
        const double r = 0.05;
        const double sigma = 0.3;
        const double T = 2;

        _Myt crr( S, r, r, sigma, T, 2 );

        const double cm0 = 7.4284;
        const double cm1 = crr.optionPrice( OptionType::Put, X );

        Q_ASSERT_DOUBLE( cm0, cm1 );
    }

    {
        const double S = 30.0;
        const double X = 30.0;
        const double r = 0.05;
        const double sigma = 0.3;
        const double T = 0.4167;

        BlackScholes bs( S, r, r, sigma, T );
        _Myt crr( S, r, r, sigma, T, 64 * 100, true );

        const double cm0 = bs.optionPrice( OptionType::Put, X );
        const double cm1 = crr.optionPrice( OptionType::Put, X );

        Q_ASSERT_DOUBLE( cm0, cm1 );
    }

    {
        const double S = 50.0;
        const double X = 40.0;
        const double r = 0.05;
        const double sigma = 0.3;
        const double T = 2.0;

        _Myt crr( S, r, r, sigma, T, 100 );

        const double cm0 = 2.47028;
        const double cm1 = crr.optionPrice( OptionType::Put, X );

        Q_ASSERT_DOUBLE( cm0, cm1 );
    }

    {
        // from Hull book, example 21.2
        const double S = 50;
        const double X = 50;
        const double r = 0.10;
        const double q = 0.0;
        const double sigma = 0.4;
        const double T = 5.0 / 12.0;

        _Myt crr( S, r, r-q, sigma, T, 50 );
        crr.optionPrice( OptionType::Put, X );

        // from Hull book, example 21.2
        double delta;
        double gamma;
        double theta;
        double vega;
        double rho;

        crr.partials( OptionType::Put, X, delta, gamma, theta, vega, rho );
        theta /= 365.0;
        vega /= 100.0;
        rho /= 100.0;

        Q_ASSERT_DOUBLE( delta, -0.4149 );
        Q_ASSERT_DOUBLE( gamma, 0.0338 );
        Q_ASSERT_DOUBLE( theta, -0.0117 );
        Q_ASSERT_DOUBLE( vega, 0.1229 );
        Q_ASSERT_DOUBLE( rho, -0.0715 );
    }
/*
    {
        // from Hull book, example 21.5
        const double S = 52;
        const double X = 50;
        const double r = 0.10;
        const double q = 0.0;
        const double sigma = 0.4;
        const double T = 5.0 / 12.0;

        std::vector<double> divTimes;
        divTimes.push_back( 3.5 / 12.0 );

        std::vector<double> divAmounts;
        divAmounts.push_back( 2.06 );

        _Myt crr( S, r, r-q, sigma, T, 100, divTimes, divAmounts );

        // from Hull:
        // 5 iterations = 4.44
        // 50 iterations = 4.202
        // 100 iterations = 4.212
        Q_ASSERT_DOUBLE( crr.optionPrice( OptionType::Put, X ), 4.2143 );
    }

    {
        // from Financial Numerical Recipies book, section 12.5
        const double S = 100;
        const double X = 100;
        const double r = 0.10;
        const double q = 0.02;
        const double sigma = 0.25;
        const double T = 1.0;

        _Myt crr_cont( S, r, r-q, sigma, T, 100 );

        Q_ASSERT_DOUBLE( crr_cont.optionPrice( OptionType::Call, X ), 13.5926 );

        std::vector<double> divTimes;
        divTimes.push_back( 0.25 );
        divTimes.push_back( 0.75 );

        std::vector<double> divAmounts;
        divAmounts.push_back( 2.5 );
        divAmounts.push_back( 2.5 );

        _Myt crr_disc( S, r, r, sigma, T, 100, divTimes, divAmounts );

        // from Numerical Recipies:
        // 100 iterations = 12.0233
        Q_ASSERT_DOUBLE( crr_disc.optionPrice( OptionType::Call, X ), 11.7861 );
    }
*/
    {
        // from Haug book, table 9-8
        const double S = 100;
        const double X = 102;
        const double r = 0.1;
        const double q = 0.0;
        const double sigma = 0.15;
        const double T = 6.0 / 12.0;
        const size_t N = 500;

        std::vector<double> divTimes;
        divTimes.push_back( 0.4 );

        std::vector<double> divYields;
        divYields.push_back( 0.05 );

        _Myt crr( S, r, r-q, sigma, T, N, divTimes, divYields );

        Q_ASSERT_DOUBLE( crr.optionPrice( OptionType::Call, X ), 4.8772 );
        Q_ASSERT_DOUBLE( crr.optionPrice( OptionType::Put, X ), 7.0000 );
    }

    {
        // from Haug book, table 9-8
        const double S = 100;
        const double X = 102;
        const double r = 0.1;
        const double q = 0.0;
        const double sigma = 0.3;
        const double T = 6.0 / 12.0;
        const size_t N = 500;

        std::vector<double> divTimes;
        divTimes.push_back( 0.25 );

        std::vector<double> divYields;
        divYields.push_back( 0.1 );

        _Myt crr( S, r, r-q, sigma, T, N, divTimes, divYields );

        Q_ASSERT_DOUBLE( crr.optionPrice( OptionType::Call, X ), 6.8194 );
        Q_ASSERT_DOUBLE( crr.optionPrice( OptionType::Put, X ), 12.9060 );
    }
}
#endif

/**
 * @file phelimboyle.cpp
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
 * This code based on code from Vinegar Hills
 * https://sites.google.com/view/vinegarhill-financelabs/monte-carlo
 */

#include "phelimboyle.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////
PhelimBoyle::PhelimBoyle( double S, double r, double b, double sigma, double T, size_t N, bool european  ) :
    _Mybase( S, r, b, sigma, T, european ),
    N_( N )
{
    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PhelimBoyle::setSigma( double value )
{
    _Mybase::setSigma( value );

    init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double PhelimBoyle::optionPrice( OptionType type, double X ) const
{
    double S = S_;

    double r = r_;
    double q = r_ - b_;

    // MacDonald Schroeder
    if ( OptionType::Call == type )
    {
        std::swap( S, X );
        std::swap( r, q );
    }

    const double dt = T_ / N_;

    const double epvsdt = exp( sigma_ * sqrt( 0.5 * dt ) );
    const double envsdt = exp( -sigma_ * sqrt( 0.5 * dt ) );
    const double ebdt = exp( 0.5 * (r - q) * dt );

    const double pu = pow( (ebdt - envsdt) / (epvsdt - envsdt), 2 );
    const double pd = pow( (epvsdt - ebdt) / (epvsdt - envsdt), 2 );
    const double pm = 1.0 - pu - pd;

    const double Df = exp( -r * dt );

    // create exercise table
    std::vector<double> extable;
    extable.reserve( 2*N_+1 );

    for ( size_t i( 0 ); i <= 2*N_; ++i )
        extable.push_back( X - S * pow( u_, (double)i - N_ ) );

    // init vector
    std::vector<double> val;
    extable.reserve( 2*N_+1 );

    for ( size_t i( 0 ); i <= 2*N_; ++i )
    {
        const double v = fmax( 0.0, extable[i] );

        val.push_back( v );

        if (( 0.0 == v ) && ( 3 <= val.size() ) && ( 0.0 == val[i-1] ))
            break;
    }

    // compute
    // backward recursion through the tree
    for ( size_t j( N_ ); j--; )
    {
        const size_t end( std::min( 2*j, val.size()-3 ) );

        size_t i = 0;

        do
        {
            val[i] = Df * ((pu * val[i + 2]) + (pm * val[i + 1]) + (pd * val[i]));

            if ( isAmerican() )
                val[i] = fmax( val[i], extable[N_+i-j] );

        } while ( i++ < end );
    }

    // return the option price
    return val[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PhelimBoyle::copy( const _Myt& other )
{
    _Mybase::copy( other );

    N_ = other.N_;

    u_ = other.u_;
    d_ = other.d_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PhelimBoyle::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    N_ = std::move( other.N_ );

    u_ = std::move( other.u_ );
    d_ = std::move( other.d_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PhelimBoyle::init()
{
    const double dt = T_ / N_;

    u_ = exp( sigma_ * sqrt( 2.0 * dt ) );
    d_ = 1.0 / u_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.0001 <= result && result <= v+0.0001 );}

void PhelimBoyle::validate()
{
    {
        const double S = 30.0;
        const double X = 30.0;
        const double r = 0.05;
        const double sigma = 0.3;
        const double T = 0.4167;

        BlackScholes bs( S, r, r, sigma, T );
        _Myt pb( S, r, r, sigma, T, 32 * 100, true );

        const double cm0 = bs.optionPrice( OptionType::Put, X );
        const double cm1 = pb.optionPrice( OptionType::Put, X );

        Q_ASSERT_DOUBLE( cm0, cm1 );
    }

    {
        const double S = 30.0;
        const double X = 29.0;
        const double r = 0.05;
        const double b = 0.025;
        const double sigma = 0.3;
        const double T = 1.0;

        _Myt pb( S, r, b, sigma, T, 100 );

        const double cm0 = 4.2918;
        const double cm1 = pb.optionPrice( OptionType::Call, X );

        Q_ASSERT_DOUBLE( cm0, cm1 );
    }
}
#endif


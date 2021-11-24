/**
 * @file binomial.cpp
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

#include "binomial.h"

#include <cmath>

#define pow2(n) ((n) * (n))

// uncomment to debug calculations
//#define DEBUG_CALC

///////////////////////////////////////////////////////////////////////////////////////////////////
BinomialTree::BinomialTree( double S, double r, double b, double sigma, double T, size_t N, bool european ) :
    _Mybase( S, r, b, sigma, T, european ),
    N_( N )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BinomialTree::calcOptionPrice( bool isCall, double S, double K, double u, double d, double pu, double pd, double Df ) const
{
    const double z( isCall ? 1 : -1 );

    // assert p in interval (0,1)
    //Q_ASSERT( dt < (pow2( sigma_ ) / pow2( b_ )) );

    //Q_ASSERT( 1.0 <= u );
    //Q_ASSERT( (0.0 < d) && (d <= 1.0) );

    // create pow tables
    std::vector<double> spowu;
    spowu.reserve( N_+1 );

    std::vector<double> powd;
    powd.reserve( N_+1 );

    for ( size_t i( 0 ); i <= N_; ++i )
    {
        spowu.push_back( S * pow( u, (double)i ) );
        powd.push_back( pow( d, (double)i ) );
    }

    // init vector
    std::vector<double> val;
    val.reserve( N_+1 );

    for ( size_t i = 0; i <= N_; ++i )
        val.push_back( fmax( 0.0, z * (spowu[i] * powd[N_ - i] - K) ) );

    // backward recursion through the tree
    for ( size_t j = N_; j--; )
    {
        for ( size_t i = 0; i <= j; ++i )
        {
            val[i] = Df * (pu * val[i + 1] + pd * val[i]);

            // check early exercise
            if ( isAmerican() )
            {
#ifdef DEBUG_CALC
                const double val0 = S * pow( u, i ) * pow( d, j - i ) - K;
                const double val1 = spowu[i] * powd[j - i] - K;

                static const double ERROR = 0.000001;
                assert(( (val0 - ERROR) <= val1 ) && ( val1 <= (val0 + ERROR) ));
#endif
                val[i] = fmax( val[i], z * (spowu[i] * powd[j - i] - K) );
            }
        }

        // track key values for partials calculation
        if ( j <= 2 )
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
double BinomialTree::calcOptionPrice( bool isCall, double S, double K, double u, double d, double pu, double pd, double Df, const std::vector<double>& divTimes, const std::vector<double>& div ) const
{
    const double z( isCall ? 1 : -1 );

    // assert p in interval (0,1)
    //Q_ASSERT( dt < (pow2( sigma_ ) / pow2( b_ )) );

    //Q_ASSERT( 1.0 <= u );
    //Q_ASSERT( (0.0 < d) && (d <= 1.0) );

    // sum dividends
    const size_t ndiv( divTimes.size() );

    std::vector<size_t> divSteps;
    divSteps.reserve( ndiv );

    double sumDiv( 1.0 );

    for ( size_t i( 0 ); i < ndiv; ++i )
    {
        divSteps.push_back( (divTimes[i] * N_) / T_ );
        sumDiv *= (1.0 - div[i]);
    }

    // init vector
    std::vector<double> St;
    St.reserve( N_+1 );

    std::vector<double> val;
    val.reserve( N_+1 );

    for ( size_t i = 0; i <= N_; ++i )
    {
        St.push_back( S * pow( u, i ) * pow( d, (N_ - i) ) * sumDiv );
        val.push_back( fmax( 0.0, z * (St[i] - K) ) );
    }

    // backward recursion through the tree
    for ( size_t j = N_; j--; )
    {
        for ( size_t m( ndiv ); m--; )
            if ( j == divSteps[m] )
            {
                for ( size_t i = 0; i <= j; ++i )
                    St[i] /= (1.0 - div[m]);
            }

        for ( size_t i = 0; i <= j; ++i )
        {
            St[i] = d * St[i + 1];
            val[i] = Df * (pu * val[i + 1] + pd * val[i]);

            // check early exercise
            if ( isAmerican() )
                val[i] = fmax( val[i], z * (St[i] - K) );
        }

        // track key values for partials calculation
        if ( j <= 2 )
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
void BinomialTree::calcPartials( double u, double d, double& delta, double& gamma, double& theta ) const
{
    const double dt = T_ / N_;
    const double h = 0.5 * S_ * (pow2( u ) - pow2( d ));

    delta = (f_[1][1] - f_[1][0]) / (S_ * (u - d));

    gamma =  (f_[2][2] - f_[2][1]) / (S_ * (pow2( u ) - 1.0));
    gamma -= (f_[2][1] - f_[2][0]) / (S_ * (1.0 - pow2( d )));
    gamma /= h;

    theta = (f_[2][1] - f_[0][0]) / (2.0 * dt);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BinomialTree::copy( const _Myt& other )
{
    _Mybase::copy( other );

    N_ = other.N_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BinomialTree::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    N_ = std::move( other.N_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.0001 <= result && result <= v+0.0001 );}

void BinomialTree::validate()
{
    {
        // from Hull book, example 21.1
        const double S = 50;
        const double X = 50;
        const double r = 0.10;
        const double q = 0.0;
        const double sigma = 0.4;
        const double T = 5.0 / 12.0;

        _Myt bt( S, r, r-q, sigma, T, 5 );

        const double u = 1.1224;
        const double d = 0.8909;

        const double pu = 0.5073;
        const double pd = 0.4927;

        const double Df = 0.9917;

        Q_ASSERT_DOUBLE( bt.calcOptionPrice( false, S, X, u, d, pu, pd, Df ), 4.4919 );

        // from Hull book, example 21.2
        double delta;
        double gamma;
        double theta;

        bt.calcPartials( u, d, delta, gamma, theta );

        Q_ASSERT_DOUBLE( delta, -0.4146 );
        Q_ASSERT_DOUBLE( gamma, 0.0341 );
        Q_ASSERT_DOUBLE( theta, -4.3035 );
    }
}

#endif


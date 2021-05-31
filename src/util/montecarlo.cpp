/**
 * @file montecarlo.cpp
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

#include "montecarlo.h"

#include <cmath>
#include <list>
#include <numeric>
#include <random>
#include <thread>

static const double pi = 3.14159265358979323846;
static const double pi2 = pi * 2.0;

#define pow2(n) ((n) * (n))

///////////////////////////////////////////////////////////////////////////////////////////////////
MonteCarlo::MonteCarlo( double S, double r, double b, double sigma, double T, size_t N ) :
    _Mybase( S, r, b, sigma, T ),
    N_( N )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double MonteCarlo::optionPrice( OptionType type, double X ) const
{
    const size_t numThreads( std::thread::hardware_concurrency() );

    size_t payoffSize( N_ );

    while ( 0 != (payoffSize % numThreads) )
        ++payoffSize;

    const size_t s( payoffSize / numThreads );

    // allocate buffer for payoff
    std::vector<double> payoff( payoffSize );

    // start some worker threads
    std::vector<std::thread> threads;

    for ( size_t i( numThreads ); i--; )
    {
        std::vector<double>::iterator begin = payoff.begin() + s*i;
        std::vector<double>::iterator end = payoff.begin() + s*(i+1);

        threads.emplace_back( std::thread( &_Myt::calcOptionPrice, this, type, X, std::move( begin ), std::move( end ) ) );
    }

    // wait for completion
    for ( std::thread& t : threads )
        if ( t.joinable() )
            t.join();

    // Simulated prices as discounted average of terminal prices
    return ert_ * std::accumulate( payoff.begin(), payoff.end(), 0.0 ) / payoffSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MonteCarlo::copy( const _Myt& other )
{
    _Mybase::copy( other );

    N_ = other.N_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MonteCarlo::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    N_ = std::move( other.N_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MonteCarlo::calcOptionPrice( OptionType type, double X, std::vector<double>::iterator begin, std::vector<double>::iterator end ) const
{
    std::random_device device;
    std::mt19937 rng( device() );

    std::uniform_real_distribution<double> dist( 0.0, 1.0 );

    do
    {
        // Independent uniform random variables
        // Floor u1 to avoid errors with log function
        const double u1 = fmax( dist( rng ), 1.0e-10 );
        const double u2 = dist( rng );

        // Z ~ N(0,1) by Box-Muller transformation
        const double Z = sqrt( -2.0 * log( u1 ) ) * sin( pi2 * u2 );

        // Simulated terminal price S(T)
        const double ST = S_ * exp( (b_ - pow2( sigma_ ) / 2.0) * T_ + vst_ * Z );

        *begin = fmax( (OptionType::Call == type) ? (ST - X) : (X - ST), 0.0 );

    } while ( ++begin != end );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.02 <= result && result <= v+0.02 );}

void MonteCarlo::validate()
{
    {
        double S = 100.0;       // Spot Price
        double K = 100.0;       // Strike Price
        double T = 1;           // Maturity in Years
        double r = 0.05;        // Interest Rate
        double q = 0;           // Dividend yeild
        double v = 0.2;         // Volatility
        int Nsims = 1e7;        // Number of simulations

        _Myt mc( S, r, r-q, v, T, Nsims );

        const double p0 = 5.5735;
        const double p1 = mc.optionPrice( OptionType::Put, K );

        Q_ASSERT_DOUBLE( p0, p1 );

        const double c0 = 10.4506;
        const double c1 = mc.optionPrice( OptionType::Call, K );

        Q_ASSERT_DOUBLE( c0, c1 );
    }
}
#endif


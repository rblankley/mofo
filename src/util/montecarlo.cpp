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
#include <thread>

static const double pi = 3.14159265358979323846;

/// Power of two (square) function.
#define pow2(n) ((n) * (n))

///////////////////////////////////////////////////////////////////////////////////////////////////
MonteCarlo::MonteCarlo( double S, double r, double b, double sigma, double T, size_t N ) :
    _Mybase( S, r, b, sigma, T ),
    N_( N ),
    rng_( std::random_device{}() )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
MonteCarlo::MonteCarlo( double S, double r, double b, double sigma, double T, size_t N, const rng_engine_type& rng ) :
    _Mybase( S, r, b, sigma, T ),
    N_( N ),
    rng_( rng )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double MonteCarlo::optionPrice( OptionType type, double X ) const
{
    const double z( (OptionType::Call == type) ? 1.0 : -1.0 );

    const double drift( (b_ - pow2( sigma_ ) / 2.0) * T_ );

    rng_engine_type rng( rng_ );
    std::uniform_real_distribution<double> dist( 0.0, 1.0 );

    size_t n( N_ );

    double sum( 0.0 );

    double deltaSum( 0.0 );
    double gammaSum( 0.0 );

    do
    {
        // Independent uniform random variables
        // Floor u1 to avoid errors with log function
        const double u1 = std::fmax( dist( rng ), 1.0e-10 );
        const double u2 = dist( rng );

        // Z ~ N(0,1) by Box-Muller transformation
        // Haug, chapter 8.3, for two values instead of one
        const double T = tan( pi * u2 );
        const double L = sqrt( -2.0 * log( u1 ) );

        const double Z0 = L * (1.0 - pow2( T )) / (1.0 + pow2( T ));
        const double Z1 = L * 2.0 * T / (1.0 + pow2( T ));

        // unroll loop for processing of Z0 and Z1

        {
            // Simulated terminal price S(T)
            const double ST = S_ * exp( drift + vst_ * Z0 );

            sum += std::fmax( 0.0, z * (ST - X) );

            if ( 0.0 < z )
            {
                if ( X < ST )
                    deltaSum += ST;
            }
            else if ( ST < X )
                deltaSum += ST;

            if ( std::fabs( ST - X ) < 2.0 )
                gammaSum += 1.0;
        }

        if ( 0 == --n )
            break;

        {
            // Simulated terminal price S(T)
            const double ST = S_ * exp( drift + vst_ * Z1 );

            sum += std::fmax( 0.0, z * (ST - X) );

            if ( 0.0 < z )
            {
                if ( X < ST )
                    deltaSum += ST;
            }
            else if ( ST < X )
                deltaSum += ST;

            if ( std::fabs( ST - X ) < 2.0 )
                gammaSum += 1.0;
        }

    } while ( --n );

    price_ = ert_ * sum / N_;

    delta_ = (z * ert_ * deltaSum) / (N_ * S_);
    gamma_ = (ert_ * pow2( X / S_ ) * gammaSum) / (4.0 * N_);
    theta_ = (r_ * price_) - (b_ * S_ * delta_) - (0.5 * pow2( sigma_ ) * pow2( S_ ) * gamma_);
    vega_ = gamma_ * sigma_ * pow2( S_ ) * T_;

    return price_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MonteCarlo::partials( OptionType type, double X, double& delta, double& gamma, double& theta, double& veg, double& rh ) const
{
    delta = delta_;
    gamma = gamma_;
    theta = theta_;

    // vega
    veg = vega( type, X );

    // rho
    rh = rho( type, X );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double MonteCarlo::rho( OptionType type, double X ) const
{
    // rho
    const double diff( 0.01 );

    _Myt calc( S_, r_+diff, b_+diff, sigma_, T_, N_, rng_ );
    return (calc.optionPrice( type, X ) - price_) / diff;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double MonteCarlo::vega( OptionType type, double X ) const
{
    Q_UNUSED( type )
    Q_UNUSED( X )

    // vega
    return vega_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MonteCarlo::copy( const _Myt& other )
{
    _Mybase::copy( other );

    N_ = other.N_;

    rng_ = other.rng_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MonteCarlo::move( const _Myt&& other )
{
    _Mybase::move( std::move( other ) );

    N_ = std::move( other.N_ );

    rng_ = std::move( other.rng_ );
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


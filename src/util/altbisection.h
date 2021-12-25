/**
 * @file altbisection.h
 * Alternative Bisection Method to compute implied volatility.
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

#ifndef ALTBISECTION_H
#define ALTBISECTION_H

#include "optiontype.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Alternative Bisection Method to compute implied volatility.
class AlternativeBisection
{
    using _Myt = AlternativeBisection;

public:

    // ========================================================================
    // Static Methods
    // ========================================================================

    /// Calculate implied volatility.
    /**
     * @tparam T  option pricing class
     * @param[in,out] pricing  option pricing
     * @param[in] type  option type
     * @param[in] X  strike price
     * @param[in] price  option price
     * @param[out] okay  @c true if calculation okay, @c false otherwise
     * @return  implied volatility of @a pricing
     */
    template <class T>
    static double calcImplVol( T *pricing, OptionType type, double X, double price, bool *okay = nullptr );

private:

    static constexpr size_t MAX_LOOPS = 64;

    static constexpr double VOLATILITY_MIN = 0.0;
    static constexpr double VOLATILITY_MAX = 100.0;
    static constexpr double EPSILON = 0.001;

    static constexpr double ERR = 0.0000001;

    /// Retrieve step size for VI.
    static constexpr double stepSize( double vi )
    {
        if ( vi < 1.0 )
            return 0.1;
        else if ( vi < 10.0 )
            return 1.0;

        return 10.0;
    }

    /// Calculate slope for current VI.
    template <class T>
    static double slope( T *pricing, OptionType type, double X, double ci );

    /// Newton's method for root finding.
    template <class T>
    static double newtonsMethod( T *pricing, OptionType type, double X, double price, double min, double max, double vi, bool& okay );

    /// Bisections method for intercept finding.
    template <class T>
    static double bisections( T *pricing, OptionType type, double X, double price, double min, double max, double z, bool& okay );

    // not implemented
    AlternativeBisection() = delete;

    // not implemented
    AlternativeBisection( _Myt& ) = delete;

    // not implemented
    AlternativeBisection( _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
inline double AlternativeBisection::calcImplVol( T *pricing, OptionType type, double X, double price, bool *okay )
{
    bool valid( false );
    double sigma;

    // Compute the Manaster and Koehler seed value (vi)
    const double seed = pricing->calcImplVolSeedValue( X );

    // Try using seed value first, it usually works... if not then we will exhaust range (time consuming)
    sigma = newtonsMethod( pricing, type, X, price, VOLATILITY_MIN, VOLATILITY_MAX, seed, valid );

    if (( valid ) && ( std::isnormal( sigma ) ))
    {
        if ( okay )
            *okay = true;

        return sigma;
    }

    // Yuck... Exhaustive Search in boundry...
    bool init( false );

    double vi0( 0.0 );
    double ci0( 0.0 );
    double m0( 0.0 );

    // split curve into sections, test each one for valid VI
    static constexpr double VOLATILITY_START = VOLATILITY_MIN;
    static constexpr double VOLATILITY_STOP = VOLATILITY_MAX + stepSize( VOLATILITY_MAX );

    for ( double vi( VOLATILITY_START ); vi < VOLATILITY_STOP; vi += stepSize( vi ) )
    {
        // calc price with new vi
        pricing->setSigma( std::fmax( vi, ERR ) );
        const double ci( pricing->optionPrice( type, X ) );

        if (( std::isinf( ci ) ) || ( std::isnan( ci ) ))
        {
            init = false;
            continue;
        }

        // calc slope
        const double m( slope( pricing, type, X, ci ) );

        // skip section we looked at already from above
        if (( init ) && (( seed < vi0 ) || ( vi < seed )))
        {
            // price bounded by lower and upper
            if ((( ci0 <= price ) && ( price <= ci )) ||
                (( ci <= price ) && ( price <= ci0 )))
            {
                const double z( ci - ci0 );

                // check!!
                sigma = bisections( pricing, type, X, price, vi0, vi, z, valid );

                if (( valid ) && ( std::isnormal( sigma ) ))
                {
                    if ( okay )
                        *okay = true;

                    return sigma;
                }
            }
            // price above or below both values but sloping towards
            else if ((( price <= ci0 ) && ( price <= ci ) && ( m0 <= 0.0 ) && ( 0.0 <= m )) ||
                     (( ci0 <= price ) && ( ci <= price ) && ( 0.0 <= m0 ) && ( m <= 0.0 )))
            {
                const double mid( (vi0 + vi) / 2.0 );

                // check!!
                sigma = newtonsMethod( pricing, type, X, price, vi0, vi, mid, valid );

                if (( valid ) && ( std::isnormal( sigma ) ))
                {
                    if ( okay )
                        *okay = true;

                    return sigma;
                }
            }
        }

        init = true;
        vi0 = vi;
        ci0 = ci;
        m0 = m;
    }

    if ( okay )
        *okay = false;

    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
inline double AlternativeBisection::slope( T *pricing, OptionType type, double X, double ci0 )
{
    static const double DELTA = 0.0000000001;

    // adjust sigma and calculate new value
    pricing->setSigma( pricing->sigma() + DELTA );
    const double ci1 = pricing->optionPrice( type, X );

    // return slope
    return (ci1 - ci0) / DELTA;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
inline double AlternativeBisection::newtonsMethod( T *pricing, OptionType type, double X, double price, double min, double max, double vi, bool& okay )
{
    const double lowerBound( min + ERR );
    const double upperBound( max - ERR );

    size_t maxloops( MAX_LOOPS );

    for ( ;; )
    {
        pricing->setSigma( vi );
        const double ci( pricing->optionPrice( type, X ) );

        // bad price
        if (( std::isinf( ci ) ) || ( std::isnan( ci ) ))
            break;

        const double delta( ci - price );

        // found solution!!
        if ( std::fabs( delta ) <= EPSILON )
        {
            okay = true;
            return vi;
        }

        // Newton's Method
        const double m( slope( pricing, type, X, ci ) );

        if ( !std::isnormal( m ) )
            break;

        // find next volatility
        const double vinext = vi - delta/m;

        if ( vinext < lowerBound )
            vi = (vi + min) / 2.0;
        else if ( upperBound < vinext )
            vi = (vi + max) / 2.0;
        else
            vi = vinext;

        // error out when:
        // 1) too many loops
        // 2) vi too small or too large
        if (( !maxloops-- ) || ( vi < lowerBound ) || ( upperBound < vi ))
            break;
    }

    okay = false;
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
inline double AlternativeBisection::bisections( T *pricing, OptionType type, double X, double price, double low, double high, double z, bool& okay )
{
    const double lowerBound( low + ERR );
    const double upperBound( high - ERR );

    size_t maxloops( MAX_LOOPS );

    // set new volatility
    pricing->setSigma( (low + high) / 2.0 );

    for ( ;; )
    {
        const double ci( pricing->optionPrice( type, X ) );

        // bad price
        if (( std::isinf( ci ) ) || ( std::isnan( ci ) ))
            break;

        // found solution!!
        if ( std::fabs( ci - price ) <= EPSILON )
        {
            okay = true;
            return pricing->sigma();
        }

        if ( price < ci )
        {
            if ( 0.0 < z )
                high = (high + low) / 2.0;
            else
                low = (high + low) / 2.0;
        }
        else if ( z <= 0.0 )
            high = (high + low) / 2.0;
        else
            low = (high + low) / 2.0;

        // set new volatility
        pricing->setSigma( (high + low) / 2.0 );

        // error out when:
        // 1) too many loops
        // 2) vi too small or too large
        if (( !maxloops-- ) || ( pricing->sigma() < lowerBound ) || ( upperBound < pricing->sigma() ))
            break;
    }

    okay = false;
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BISECTION_H

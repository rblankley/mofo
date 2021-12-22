/**
 * @file newtonraphson.h
 * Newton-Raphson Implied Volatility method.
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

#ifndef NEWTONRAPHSON_H
#define NEWTONRAPHSON_H

#include "optiontype.h"

#include <QtGlobal>

#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Newton-Raphson Implied Volatility methods.
class NewtonRaphson
{
    using _Myt = NewtonRaphson;

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

#if defined( QT_DEBUG )
    /// Validate methods.
    static void validate();
#endif

private:

    static const size_t MAX_LOOPS = 512;

    // not implemented
    NewtonRaphson() = delete;

    // not implemented
    NewtonRaphson( _Myt& ) = delete;

    // not implemented
    NewtonRaphson( _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( _Myt&& ) = delete;

};

template <class T>
inline double NewtonRaphson::calcImplVol( T *pricing, OptionType type, double X, double price, bool *okay )
{
    static const double VOLATILITY_MIN = 0.0000001;
    static const double VOLATILITY_MAX = 1000.0 - VOLATILITY_MIN;
    static const double EPSILON = 0.001;

    size_t maxloops( MAX_LOOPS );

    // Compute the Manaster and Koehler seed value (vi)
    double vi = pricing->calcImplVolSeedValue( X );

    pricing->setSigma( vi );

    double ci = pricing->optionPrice( type, X );
    double vegai = pricing->vega( type, X );

    while ( EPSILON < std::fabs( price - ci ) )
    {
        vi -= (ci - price) / vegai;

        if (( std::isinf( vi ) ) || ( std::isnan( vi ) ) || ( vi < VOLATILITY_MIN ) || ( vi > VOLATILITY_MAX ))
        {
            if ( okay )
                *okay = false;

            return 0.0;
        }

        pricing->setSigma( vi );

        ci = pricing->optionPrice( type, X );
        vegai = pricing->vega( type, X );

        if ( !maxloops-- )
        {
            if ( okay )
                *okay = false;

            return 0.0;
        }
    }

    if ( okay )
        *okay = true;

    return vi;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // NEWTONRAPHSON_H

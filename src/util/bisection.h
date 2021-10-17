/**
 * @file bisection.h
 * Bisection Method to compute implied volatility.
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

#ifndef BISECTION_H
#define BISECTION_H

#include "optiontype.h"

#include <cmath>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Bisection Method to compute implied volatility.
class Bisection
{
    using _Myt = Bisection;

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
     * @return  implied volatility of @p pricing
     */
    template <class T>
    static double calcImplVol( T& pricing, OptionType type, double X, double price, bool *okay = nullptr );

private:

    // not implemented
    Bisection() = delete;

    // not implemented
    Bisection( _Myt& ) = delete;

    // not implemented
    Bisection( _Myt&& ) = delete;

    // not implemented
    _Myt& operator = ( _Myt& ) = delete;

    // not implemented
    _Myt& operator = ( _Myt&& ) = delete;

};

template <class T>
inline double Bisection::calcImplVol( T& pricing, OptionType type, double X, double price, bool *okay )
{
    static const double VOLATILITY_MIN = 0.0000001;
    static const double VOLATILITY_MAX = 100.99999;
    static const double EPSILON = 0.00000001;

    T vLow( pricing );
    vLow.setSigma( VOLATILITY_MIN );

    T vHigh( pricing );
    vHigh.setSigma( VOLATILITY_MAX );

    for ( ;; )
    {
        const double cLow( vLow.optionPrice( type, X ) );
        const double cHigh( vHigh.optionPrice( type, X ) );

        // If price < cLow, then it is impossible to compute a proper
        // implied volatility. This because that volatility would be
        // greater than VOLATILITY_MIN. Now what?
        // Another condition is when price > cHigh. When that happens,
        // IV is way above VOLATILITY_MAX.
        if (( price < cLow ) || ( std::isinf( cLow ) ) || ( std::isnan( cLow ) ) ||
            ( price > cHigh ) || ( std::isinf( cHigh ) ) || ( std::isnan( cHigh ) ))
        {
            if ( okay )
                *okay = false;

            return 0.0;
        }

        const double vi = vLow.sigma() + (price - cLow) * (vHigh.sigma() - vLow.sigma()) / (cHigh - cLow);

        // set new volatility
        pricing.setSigma( vi );

        const double val( pricing.optionPrice( type, X ) );

        if ( std::fabs( price - val ) <= EPSILON )
            break;
        else if ( val < price )
            vLow.setSigma( vi );
        else
            vHigh.setSigma( vi );
    }

    if ( okay )
        *okay = true;

    return pricing.sigma();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BISECTION_H

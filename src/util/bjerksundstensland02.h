/**
 * @file bjerksundstensland02.h
 * Bjerksund & Stensland 2002 American Option Approximation methods.
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

#ifndef BJERKSUNDSTENSLAND02_H
#define BJERKSUNDSTENSLAND02_H

#include "bjerksundstensland93.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Bjerksund & Stensland American Option Approximation methods.
class BjerksundStensland2002 : public BjerksundStensland1993
{
    using _Myt = BjerksundStensland2002;
    using _Mybase = BjerksundStensland1993;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] S  underlying price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     */
    BjerksundStensland2002( double S, double r, double b, double sigma, double T );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    BjerksundStensland2002( const _Myt& other ) : _Mybase( other ) {}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    BjerksundStensland2002( const _Myt&& other ) : _Mybase( other ) {}

    // ========================================================================
    // Operators
    // ========================================================================

    /// Assignment operator.
    /**
     * @param[in] rhs  object to copy
     * @return  reference to this
     */
    _Myt& operator = ( const _Myt& rhs ) {copy( rhs ); return *this;}

    /// Move operator.
    /**
     * @param[in] rhs  object to move
     * @return  reference to this
     */
    _Myt& operator = ( const _Myt&& rhs ) {move( std::move( rhs ) ); return *this;}

    // ========================================================================
    // Static Methods
    // ========================================================================

#if defined( QT_DEBUG )
    /// Validate methods.
    static void validate();
#endif

protected:

    // ========================================================================
    // Properties
    // ========================================================================

    /// Compute option price for call option.
    /**
     * @param[in] X  strike price
     * @return  option price
     */
    virtual double optionPriceCall( double X ) const override;

    /// Compute option price for put option.
    /**
     * @param[in] X  strike price
     * @return  option price
     */
    virtual double optionPricePut( double X ) const override;

    /// Compute ksi.
    /**
     * @param[in] S  underlying price
     * @param[in] T2  time to expiration (years)
     * @param[in] gamma  gamma
     * @param[in] H  h(T) value
     * @param[in] I2  trigger (boundary) price
     * @param[in] I1  trigger (boundary) price
     * @param[in] t1  time to expiration (years)
     */
    virtual double ksi( double S, double T2, double gamma, double H, double I2, double I1, double t1 ) const;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BJERKSUNDSTENSLAND02_H

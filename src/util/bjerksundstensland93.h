/**
 * @file bjerksundstensland93.h
 * Bjerksund & Stensland 1993 American Option Approximation methods.
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

#ifndef BJERKSUNDSTENSLAND93_H
#define BJERKSUNDSTENSLAND93_H

#include "blackscholes.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Bjerksund & Stensland 1993 American Option Approximation methods.
class BjerksundStensland1993 : public BlackScholes
{
    using _Myt = BjerksundStensland1993;
    using _Mybase = BlackScholes;

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
    BjerksundStensland1993( double S, double r, double b, double sigma, double T );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    BjerksundStensland1993( const _Myt& other ) : _Mybase() {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    BjerksundStensland1993( const _Myt&& other ) : _Mybase() {move( std::move( other ) );}

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
    // Properties
    // ========================================================================

    /// Check for european style option.
    /**
     * @return  @c true if european, @c false otherwise
     */
    virtual bool isEuropean() const override {return false;}

    /// Compute option price.
    /**
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  option price
     */
    virtual double optionPrice( OptionType type, double X ) const override;

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
    virtual double optionPriceCall( double X ) const;

    /// Compute option price for put option.
    /**
     * @param[in] X  strike price
     * @return  option price
     */
    virtual double optionPricePut( double X ) const;

    /// Compute phi.
    /**
     * @param[in] S  underlying price
     * @param[in] T  time to expiration (years)
     * @param[in] gamma  gamma
     * @param[in] H  h(T) value
     * @param[in] I  trigger (boundary) price
     */
    virtual double phi( double S, double T, double gamma, double H, double I ) const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Copy object.
    /**
     * @param[in] other  object to copy
     * @return  reference to this
     */
    void copy( const _Myt& other ) {_Mybase::copy( other );}

    /// Move object.
    /**
     * @param[in] other  object to move
     * @return  reference to this
     */
    void move( const _Myt&& other ) {_Mybase::move( std::move( other ) );}

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BJERKSUNDSTENSLAND93_H

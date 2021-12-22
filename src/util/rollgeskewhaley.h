/**
 * @file rollgeskewhaley.h
 * Roll-Geske-Whaley American Option Approximation method with known dividend payout.
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

#ifndef ROLLGESKEWHALEY_H
#define ROLLGESKEWHALEY_H

#include "blackscholes.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Roll-Geske-Whaley American Option Approximation method with known dividend payout.
class RollGeskeWhaley : public BlackScholes
{
    using _Myt = RollGeskeWhaley;
    using _Mybase = BlackScholes;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] S  underlying price
     * @param[in] r  risk-free interest rate
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] d  dividend payout
     * @param[in] DT  time to dividend payout (years)
     */
    RollGeskeWhaley( double S, double r, double sigma, double T, double d, double DT );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    RollGeskeWhaley( const _Myt& other ) : _Mybase() {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    RollGeskeWhaley( const _Myt&& other ) : _Mybase() {move( std::move( other ) );}

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
     * @warning
     * Only call type options are supported!
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

    double d_;                                      ///< Dividend payout.
    double DT_;                                     ///< Time to dividend payout (years).

    // ========================================================================
    // Methods
    // ========================================================================

    /// Copy object.
    /**
     * @param[in] other  object to copy
     * @return  reference to this
     */
    void copy( const _Myt& other );

    /// Move object.
    /**
     * @param[in] other  object to move
     * @return  reference to this
     */
    void move( const _Myt&& other );

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ROLLGESKEWHALEY_H

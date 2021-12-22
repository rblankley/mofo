/**
 * @file baroneadesiwhaley.h
 * Barone-Adesi-Whaley American Option Approximation methods.
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

#ifndef BARONEADESIWHALEY_H
#define BARONEADESIWHALEY_H

#include "blackscholes.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Barone-Adesi-Whaley American Option Approximation methods.
class BaroneAdesiWhaley : public BlackScholes
{
    using _Myt = BaroneAdesiWhaley;
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
    BaroneAdesiWhaley( double S, double r, double b, double sigma, double T );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    BaroneAdesiWhaley( const _Myt& other ) : _Mybase() {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    BaroneAdesiWhaley( const _Myt&& other ) : _Mybase() {move( std::move( other ) );}

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

    /// Set new volatility.
    /**
     * @param[in] value  volatility of underlying
     */
    virtual void setSigma( double value ) override;

    // ========================================================================
    // Static Methods
    // ========================================================================

#if defined( QT_DEBUG )
    /// Validate methods.
    static void validate();
#endif

protected:

    double p2v_;                                    ///< sigma^2

    // ========================================================================
    // Properties
    // ========================================================================

    /// Calculate option price for call option.
    /**
     * @param[in] X  strike price
     * @return  option price
     */
    virtual double calcOptionPriceCall( double X ) const;

    /// Calculate option price for put option.
    /**
     * @param[in] X  strike price
     * @return  option price
     */
    virtual double calcOptionPricePut( double X ) const;

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

private:

    /// Initialize.
    void init();

    /// Compute seed value for call option (Kc).
    double calcSeedCall( double X ) const;

    /// Compute seed value for put option (Kp).
    double calcSeedPut( double X ) const;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BARONEADESIWHALEY_H

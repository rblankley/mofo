/**
 * @file blackscholes.h
 * Black-Scholes Option Pricing methods.
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

#ifndef BLACKSCHOLES_H
#define BLACKSCHOLES_H

#include "abstractoptionpricing.h"

#include <QtGlobal>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Generalized Black-Scholes (Extended) Options Model.
class BlackScholes : public AbstractOptionPricing
{
    using _Myt = BlackScholes;
    using _Mybase = AbstractOptionPricing;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @note
     * when b = r           gives the Black-Scholes (1973) stock option model
     * when b = r - q       gives the Merton (1973) stock option model
     * when b = 0           gives the Black (1976) futures option model
     * when b = r - rf      gives the Garman-Kohlhangen (1983) currency option model (where rf is risk-free rate of foreign currency)
     * @param[in] S  underlying price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     */
    BlackScholes( double S, double r, double b, double sigma, double T );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    BlackScholes( const _Myt& other ) : _Mybase() {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    BlackScholes( const _Myt&& other ) : _Mybase() {move( std::move( other ) );}

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
    virtual bool isEuropean() const override {return true;}

    /// Compute option price.
    /**
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  option price
     */
    virtual double optionPrice( OptionType type, double X ) const override;

    /// Compute partials.
    /**
     * @param[in] type  option type
     * @param[in] X  strike price
     * @param[out] delta  partial with respect to underlying price
     * @param[out] gamma  second partial with respect to underlying price
     * @param[out] theta  partial with respect to time
     * @param[out] vega  partial with respect to sigma
     * @param[out] rho  partial with respect to rate
     */
    virtual void partials( OptionType type, double X, double& delta, double& gamma, double& theta, double& vega, double& rho ) const;

    /// Set new volatility.
    /**
     * @param[in] value  volatility of underlying
     */
    virtual void setSigma( double value ) override;

    /// Compute vega greek.
    /**
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  partial with respect to sigma
     */
    virtual double vega( OptionType type, double X ) const;

    // ========================================================================
    // Static Methods
    // ========================================================================

#if defined( QT_DEBUG )
    /// Validate methods.
    static void validate();
#endif

protected:

    double st_;
    double vst_;

    double ebrt_;
    double sbrt_;

    double ert_;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    BlackScholes() {}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Copy object.
    /**
     * @param[in] rhs  object to copy
     * @return  reference to this
     */
    void copy( const _Myt& other );

    /// Move object.
    /**
     * @param[in] rhs  object to move
     * @return  reference to this
     */
    void move( const _Myt&& other );

private:

    /// Initialize.
    void init();

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BLACKSCHOLES_H

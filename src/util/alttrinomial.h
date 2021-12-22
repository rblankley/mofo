/**
 * @file alttrinomial.h
 * Alternative Trinomial Tree Option Pricing methods.
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

#ifndef ALTTRINOMIAL_H
#define ALTTRINOMIAL_H

#include "trinomial.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Alternative Trinomial Tree Option Pricing methods.
class AlternativeTrinomialTree : public TrinomialTree
{
    using _Myt = AlternativeTrinomialTree;
    using _Mybase = TrinomialTree;

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
     * @param[in] N  trinomial tree depth
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     */
    AlternativeTrinomialTree( double S, double r, double b, double sigma, double T, size_t N, bool european = false );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    AlternativeTrinomialTree( const _Myt& other ) : _Mybase() {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    AlternativeTrinomialTree( const _Myt&& other ) : _Mybase() {move( std::move( other ) );}

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

    /// Compute option price.
    /**
     * Computed via trinomial tree.
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
    virtual void partials( OptionType type, double X, double& delta, double& gamma, double& theta, double& vega, double& rho ) const override;

    /// Compute rho greek.
    /**
     * @note
     * Assumes you calculated the option price prior to calling this.
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  partial with respect to interest rate
     * @sa  optionPrice()
     */
    virtual double rho( OptionType type, double X ) const {return calcRho<_Myt>( type, X );}

    /// Set new volatility.
    /**
     * @param[in] value  volatility of underlying
     */
    virtual void setSigma( double value ) override;

    /// Compute vega greek.
    /**
     * @note
     * Assumes you calculated the option price prior to calling this.
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  partial with respect to sigma
     * @sa  optionPrice()
     */
    virtual double vega( OptionType type, double X ) const override {return calcVega<_Myt>( type, X );}

    // ========================================================================
    // Static Methods
    // ========================================================================

#if defined( QT_DEBUG )
    /// Validate methods.
    static void validate();
#endif

protected:

    double u_;                                      ///< Up movement amount.
    double d_;                                      ///< Down movement amount.

    double pu_;                                     ///< Probability of up movement.
    double pd_;                                     ///< Probability of down movement.
    double pm_;                                     ///< Probability of no movement.

    double Df_;                                     ///< Discount factor.

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

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ALTTRINOMIAL_H

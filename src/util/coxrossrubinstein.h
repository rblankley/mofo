/**
 * @file coxrossrubinstein.h
 * Cox-Ross-Rubinstein Binomial Tree Option Pricing methods.
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

#ifndef COXROSSRUBINSTEIN_H
#define COXROSSRUBINSTEIN_H

#include "binomial.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Cox-Ross-Rubinstein Binomial Tree Option Pricing methods.
class CoxRossRubinstein : public BinomialTree
{
    using _Myt = CoxRossRubinstein;
    using _Mybase = BinomialTree;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] S  underlying (spot) price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] N  binomial tree depth
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     */
    CoxRossRubinstein( double S, double r, double b, double sigma, double T, size_t N, bool european = false );

    /// Constructor.
    /**
     * @warning
     * Passed in @c vector classes @p divTimes and @p divAmounts are assumed to have equal sizes.
     * @param[in] S  underlying (spot) price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] N  binomial tree depth
     * @param[in] divTimes  dividend times
     * @param[in] divAmounts  dividend amounts
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     */
    CoxRossRubinstein( double S, double r, double b, double sigma, double T, size_t N, const std::vector<double>& divTimes, const std::vector<double>& divAmounts, bool european = false );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    CoxRossRubinstein( const _Myt& other ) : _Mybase() {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    CoxRossRubinstein( const _Myt&& other ) : _Mybase() {move( std::move( other ) );}

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
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  option price
     */
    virtual double optionPrice( OptionType type, double X ) const override;

    /// Compute partials.
    /**
     * @note
     * Assumes you called @fn optionPrice() prior to calling this.
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
     * Assumes you called @fn optionPrice() prior to calling this.
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  partial with respect to sigma
     */
    virtual double rho( OptionType type, double X ) const;

    /// Set new volatility.
    /**
     * @param[in] value  volatility of underlying
     */
    virtual void setSigma( double value ) override;

    /// Compute vega greek.
    /**
     * @note
     * Assumes you called @fn optionPrice() prior to calling this.
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  partial with respect to sigma
     */
    virtual double vega( OptionType type, double X ) const override;

    // ========================================================================
    // Static Methods
    // ========================================================================

#if defined( QT_DEBUG )
    /// Validate methods.
    static void validate();
#endif

protected:

    std::vector<double> div_;

    double u_;
    double d_;

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
     * @param[in] N  binomial tree depth
     * @param[in] div  dividend table
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     */
    CoxRossRubinstein( double S, double r, double b, double sigma, double T, size_t N, const std::vector<double>& div, bool european );

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

#endif // COXROSSRUBINSTEIN_H

/**
 * @file binomialcalc.h
 * Binomial tree based option profit calculator.
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

#ifndef BINOMIALCALC_H
#define BINOMIALCALC_H

#include "expectedvaluecalc.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Binomial tree based option profit calculator.
class BinomialCalculator : public ExpectedValueCalculator
{
    using _Myt = BinomialCalculator;
    using _Mybase = ExpectedValueCalculator;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] underlying  underlying price (i.e. mark)
     * @param[in] chains  chains to evaluate
     * @param[in] results  results
     */
    BinomialCalculator( double underlying, const table_model_type *chains, item_model_type *results );

    /// Destructor.
    ~BinomialCalculator();

protected:

    // ========================================================================
    // Methods
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
    virtual double calcImplVol( AbstractOptionPricing *pricing, OptionType type, double X, double price, bool *okay = nullptr ) const override;

    /// Option pricing method factory method.
    /**
     * @param[in] S  underlying price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     * @return  pointer to pricing method
     */
    virtual AbstractOptionPricing *createPricingMethod( double S, double r, double b, double sigma, double T, bool european = false ) const override;

private:

    enum {BINOM_DEPTH = 256};

    // not implemented
    BinomialCalculator( const _Myt& ) = delete;

    // not implemented
    BinomialCalculator( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BINOMIALCALC_H

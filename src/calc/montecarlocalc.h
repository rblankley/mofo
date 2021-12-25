/**
 * @file montecarlocalc.h
 * Monte Carlo simulaions based option profit calculator.
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

#ifndef MONTECARLOCALC_H
#define MONTECARLOCALC_H

#include "abstractevcalc.h"

#include "util/altbisection.h"
#include "util/montecarlo.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Monte Carlo simulaions based option profit calculator.
class MonteCarloCalculator : public AbstractExpectedValueCalculator<MonteCarlo, AlternativeBisection>
{
    using _Myt = MonteCarloCalculator;
    using _Mybase = AbstractExpectedValueCalculator<MonteCarlo, AlternativeBisection>;

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
    MonteCarloCalculator( double underlying, const table_model_type *chains, item_model_type *results );

    /// Destructor.
    ~MonteCarloCalculator();

protected:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Factory method for creation of Option Pricing Methods.
    /**
     * @param[in] S  underlying (spot) price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     * @return  pointer to pricing method
     */
    virtual AbstractOptionPricing *createPricingMethod( double S, double r, double b, double sigma, double T, bool european = false ) const override;

private:

    static constexpr int NUM_SIMULATIONS = 4*1024;

    pricing_method_type::rng_engine_type rng_;

    // not implemented
    MonteCarloCalculator( const _Myt& ) = delete;

    // not implemented
    MonteCarloCalculator( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MONTECARLOCALC_H

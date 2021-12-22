/**
 * @file montecarlo.h
 * Monte Carlo Simulation for Option Pricing.
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

#ifndef MONTECARLO_H
#define MONTECARLO_H

#include "blackscholes.h"

#include <random>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Monte Carlo Simulation for Option Pricing
class MonteCarlo : public BlackScholes
{
    using _Myt = MonteCarlo;
    using _Mybase = BlackScholes;

public:

    /// Random number generator engine type.
    using rng_engine_t = std::mt19937_64;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @note
     * @param[in] S  underlying price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] N  number of simulations
     */
    MonteCarlo( double S, double r, double b, double sigma, double T, size_t N );

    /// Constructor.
    /**
     * @note
     * @param[in] S  underlying price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] N  number of simulations
     * @param[in] rnd  random number generator
     */
    MonteCarlo( double S, double r, double b, double sigma, double T, size_t N, const rng_engine_t& rng );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    MonteCarlo( const _Myt& other ) : _Mybase() {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    MonteCarlo( const _Myt&& other ) : _Mybase() {move( std::move( other ) );}

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
     * Assumes you calculated the option price prior to calling this.
     * @param[in] type  option type
     * @param[in] X  strike price
     * @param[out] delta  partial with respect to underlying price
     * @param[out] gamma  second partial with respect to underlying price
     * @param[out] theta  partial with respect to time
     * @param[out] vega  partial with respect to sigma
     * @param[out] rho  partial with respect to rate
     * @sa  optionPrice()
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
    virtual double rho( OptionType type, double X ) const;

    /// Compute vega greek.
    /**
     * @note
     * Assumes you calculated the option price prior to calling this.
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  partial with respect to sigma
     * @sa  optionPrice()
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

    size_t N_;                                      ///< Number of simulations.

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

    mutable double price_;

    mutable double delta_;
    mutable double gamma_;
    mutable double theta_;
    mutable double vega_;

    rng_engine_t rng_;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // MONTECARLO_H

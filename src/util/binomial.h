/**
 * @file binomial.h
 * Binomial Tree Option Pricing methods.
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

#ifndef BINOMIAL_H
#define BINOMIAL_H

#include "dualmodeoptionpricing.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Binomial Tree Option Pricing methods.
class BinomialTree : public DualModeOptionPricing
{
    using _Myt = BinomialTree;
    using _Mybase = DualModeOptionPricing;

public:

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

    size_t N_;                                      ///< Tree depth.

    std::vector<double> divTimes_;                  ///< List of dividend payout times.
    std::vector<double> div_;                       ///< List of dividend yields.

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    BinomialTree() {}

    /// Constructor.
    /**
     * @param[in] S  underlying price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] N  binomial tree depth
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     */
    BinomialTree( double S, double r, double b, double sigma, double T, size_t N, bool european = false );

    /// Constructor.
    /**
     * @warning
     * Passed in @c vector classes @a divTimes and @a divYields are assumed to have equal sizes.
     * @param[in] S  underlying (spot) price
     * @param[in] r  risk-free interest rate
     * @param[in] b  cost-of-carry rate of holding underlying
     * @param[in] sigma  volatility of underlying
     * @param[in] T  time to expiration (years)
     * @param[in] N  binomial tree depth
     * @param[in] divTimes  dividend times
     * @param[in] divYields  dividend yields
     * @param[in] european  @c true for european style option (exercise at expiry only), @c false for american style (exercise any time)
     */
    BinomialTree( double S, double r, double b, double sigma, double T, size_t N, const std::vector<double>& divTimes, const std::vector<double>& divYields, bool european = false );

    /// Constructor.
    /**
     * @param[in] other  object to copy
     */
    BinomialTree( const _Myt& other ) : _Mybase() {copy( other );}

    /// Constructor.
    /**
     * @param[in] other  object to move
     */
    BinomialTree( const _Myt&& other ) : _Mybase() {move( std::move( other ) );}

    // ========================================================================
    // Properties
    // ========================================================================

    /// Calculate option price using binomial pricing.
    /**
     * @param[in] isCall  @c true if option is call, @c false for put
     * @param[in] S  underlying (spot) price
     * @param[in] K  strike price
     * @param[in] u  upward amount
     * @param[in] d  downward amount
     * @param[in] pu  probability up
     * @param[in] pd  probability down
     * @param[in] Df  discount factor
     * @return  option price
     */
    virtual double calcOptionPrice( bool isCall, double S, double K, double u, double d, double pu, double pd, double Df ) const;

    /// Calculate partials.
    /**
     * @param[in] u  upward amount
     * @param[in] d  downward amount
     * @param[out] delta  partial with respect to underlying price
     * @param[out] gamma  second partial with respect to underlying price
     * @param[out] theta  partial with respect to time
     */
    virtual void calcPartials( double u, double d, double& delta, double& gamma, double& theta ) const;

    /// Calculate rho greek.
    /**
     * @note
     * Assumes you calculated the option price prior to calling this.
     * @tparam T  option pricing type
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  partial with respect to interest rate
     * @sa  calcOptionPrice()
     */
    template <class T>
    double calcRho( OptionType type, double X ) const
    {
        // rho
        const double diff( 0.01 );

        T calc( S_, r_+diff, b_+diff, sigma_, T_, N_, divTimes_, div_, european_ );
        return (calc.optionPrice( type, X ) - f_[0][0]) / diff;
    }

    /// Calculate vega greek.
    /**
     * @note
     * Assumes you calculated the option price prior to calling this.
     * @tparam T  option pricing type
     * @param[in] type  option type
     * @param[in] X  strike price
     * @return  partial with respect to sigma
     * @sa  calcOptionPrice()
     */
    template <class T>
    double calcVega( OptionType type, double X ) const
    {
        // vega
        const double diff( 0.02 );

        T calc( S_, r_, b_, sigma_+diff, T_, N_, divTimes_, div_, european_ );
        return (calc.optionPrice( type, X ) - f_[0][0]) / diff;
    }

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

    mutable double f_[3][3];

    /// Calculate option price.
    double calcOptionPriceImpl( bool isCall, double S, double K, double u, double d, double pu, double pd, double Df ) const;

    /// Calculate option price.
    double calcOptionPriceImpl( bool isCall, double S, double K, double u, double d, double pu, double pd, double Df, const std::vector<double>& divTimes, const std::vector<double>& divYields ) const;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // BINOMIAL_H

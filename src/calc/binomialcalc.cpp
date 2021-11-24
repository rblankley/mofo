/**
 * @file binomialcalc.cpp
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

#include "common.h"
#include "binomialcalc.h"

#include "util/coxrossrubinstein.h"
#include "util/newtonraphson.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
BinomialCalculator::BinomialCalculator( double underlying, const table_model_type *chains, item_model_type *results ) :
    _Mybase( underlying, chains, results )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
BinomialCalculator::~BinomialCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double BinomialCalculator::calcImplVol( AbstractOptionPricing *pricing, OptionType type, double X, double price, bool *okay ) const
{
    return NewtonRaphson::calcImplVol( pricing, type, X, price, okay );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractOptionPricing *BinomialCalculator::createPricingMethod( double S, double r, double b, double sigma, double T, bool european ) const
{
    return new CoxRossRubinstein( S, r, b, sigma, T, BINOM_DEPTH, european );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractOptionPricing *BinomialCalculator::createPricingMethod( double S, double r, double b, double sigma, double T, const std::vector<double>& divTimes, const std::vector<double>& divYields, bool european ) const
{
    return new CoxRossRubinstein( S, r, b, sigma, T, BINOM_DEPTH, divTimes, divYields, european );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void BinomialCalculator::destroyPricingMethod( AbstractOptionPricing *doomed ) const
{
    if ( doomed )
        delete doomed;
}

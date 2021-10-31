/**
 * @file trinomialcalc.cpp
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
#include "trinomialcalc.h"

#include "util/newtonraphson.h"
#include "util/phelimboyle.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
TrinomialCalculator::TrinomialCalculator( double underlying, const table_model_type *chains, item_model_type *results ) :
    _Mybase( underlying, chains, results )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TrinomialCalculator::~TrinomialCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double TrinomialCalculator::calcImplVol( AbstractOptionPricing *pricing, OptionType type, double X, double price, bool *okay ) const
{
    return NewtonRaphson::calcImplVol( pricing, type, X, price, okay );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AbstractOptionPricing *TrinomialCalculator::createPricingMethod( double S, double r, double b, double sigma, double T, bool european ) const
{
    return new PhelimBoyle( S, r, b, sigma, T, TRINOM_DEPTH, european );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TrinomialCalculator::destroyPricingMethod( AbstractOptionPricing *doomed ) const
{
    if ( doomed )
        delete doomed;
}

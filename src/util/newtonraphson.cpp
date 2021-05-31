/**
 * @file newtonraphson.cpp
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
 *
 * @note
 * This code adapted from libmetaoptions - A collection of option-related functions.
 * Copyright (C) 2000-2004 B. Augestad, bjorn.augestad@gmail.com
 */

#include "blackscholes.h"
#include "newtonraphson.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.0001 <= result && result <= v+0.0001 );}

void NewtonRaphson::validate()
{
    bool okay;

    double iv;
    double cm;

    {
        const double S = 100.0;
        const double X = 100.0;
        const double r = 0.08;
        const double sigma = 0.20;
        const double T = 0.5;

        BlackScholes bs( S, r, r, sigma, T );

        cm = bs.optionPrice( OptionType::Call, X );
        iv = _Myt::calcImplVol( bs, OptionType::Call, X, cm, &okay );

        Q_ASSERT( okay );
        Q_ASSERT_DOUBLE( sigma, iv );

        cm = bs.optionPrice( OptionType::Put, X );
        iv = _Myt::calcImplVol( bs, OptionType::Put, X, cm, &okay );

        Q_ASSERT( okay );
        Q_ASSERT_DOUBLE( sigma, iv );
    }

    {
        const double S = 75.0;
        const double X = 70.0;
        const double r = 0.10;
        const double sigma = 0.35;
        const double T = 0.5;

        BlackScholes bs( S, r, r, sigma, T );

        cm = bs.optionPrice( OptionType::Call, X );
        iv = _Myt::calcImplVol( bs, OptionType::Call, X, cm, &okay );

        Q_ASSERT( okay );
        Q_ASSERT_DOUBLE( sigma, iv );

        cm = bs.optionPrice( OptionType::Put, X );
        iv = _Myt::calcImplVol( bs, OptionType::Put, X, cm, &okay );

        Q_ASSERT( okay );
        Q_ASSERT_DOUBLE( sigma, iv );
    }

}
#endif

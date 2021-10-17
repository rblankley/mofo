/**
 * @file cnd.cpp
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

#include <cmath>

static const double one_div_sqrt2pi = 0.39894228040143270286;

#define pow2(n) ((n) * (n))
#define normdist(x) ( one_div_sqrt2pi * exp(-(((x) * (x))/ 2.0)))

///////////////////////////////////////////////////////////////////////////////////////////////////
double cnd( double x )
{
    static const double
        a1 = +0.31938153,
        a2 = -0.356563782,
        a3 = +1.781477937,
        a4 = -1.821255978,
        a5 = +1.330274429;

    const double L = std::fabs(x);
    const double K = 1.0 / (1.0 + (0.2316419 * L));

    const double a12345k
        = (a1 * K)
        + (a2 * K * K)
        + (a3 * K * K * K)
        + (a4 * K * K * K * K)
        + (a5 * K * K * K * K * K);

    double result = 1.0 - one_div_sqrt2pi * exp(-pow2(L) / 2.0) * a12345k;

    if ( x < 0.0 )
        result = 1.0 - result;

    return result;
}

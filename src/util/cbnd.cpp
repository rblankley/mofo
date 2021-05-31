/**
 * @file cbnd.cpp
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
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include <QVector>

static const double pi = 3.14159265358979323846;

#define pow2(n) ((n) * (n))

double cnd( double x );

static const double y[] = {
    0.10024215,
    0.48281397,
    1.0609498,
    1.7797294,
    2.6697604
};

/* We keep multiplying y with 2.0, so here is a precomputed version */
static const double y2[] = {
    0.2004843000,
    0.9656279400,
    2.1218996000,
    3.5594588000,
    5.3395208000
};

static const double XX[5][5] = {
    {
        0.06170561535782249917847508414,
        0.09745745062408049663726927747,
        0.05251757861786850167806761647,
        0.00825867481095899844123486844,
        0.00020489864250404100768677973,
    }, {
        0.09745745062408049663726927747,
        0.15392366848734490014649622935,
        0.08294592470016330654214442575,
        0.01304369769172619882013908210,
        0.00032361559347527383201023610,
    }, {
        0.05251757861786850167806761647,
        0.08294592470016330654214442575,
        0.04469765106287610506585750159,
        0.00702894868074539942714995533,
        0.00017438900015825459450038992,
    }, {
        0.00825867481095899844123486844,
        0.01304369769172619882013908210,
        0.00702894868074539942714995533,
        0.00110534040115559985262283504,
        0.00002742361854484439765474585,
    }, {
        0.00020489864250404100768677973,
        0.00032361559347527383201023610,
        0.00017438900015825459450038992,
        0.00002742361854484439765474585,
        0.00000068038303250915557643890,
    },
};

///////////////////////////////////////////////////////////////////////////////////////////////////
double sign( double d )
{
    if ( d < 0.0 )
        return -1.0;

    return 1.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// cumulative bivariate normal distribution function
double cbnd( double a, double b, double Rho )
{
    double result;

    if ( std::isinf( b ) )
        b = 10;

    const double t = sqrt(2.0 * (1.0 - pow2(Rho)));
    const double a1 = a / t;
    const double b1 = b / t;

    if (( a <= 0.0 ) && ( b <= 0.0 ) && ( Rho <= 0.0 ))
    {
        const double rho20 = Rho * 2.0;

        double sum = 0.0;

        for ( int i( 5 ); i--; )
        {
            sum += XX[i][0] * exp(a1 * (y2[i] - a1) + (b1 * (y2[0] - b1) ) + (rho20 * (y[i] - a1) * (y[0] - b1)));
            sum += XX[i][1] * exp(a1 * (y2[i] - a1) + (b1 * (y2[1] - b1) ) + (rho20 * (y[i] - a1) * (y[1] - b1)));
            sum += XX[i][2] * exp(a1 * (y2[i] - a1) + (b1 * (y2[2] - b1) ) + (rho20 * (y[i] - a1) * (y[2] - b1)));
            sum += XX[i][3] * exp(a1 * (y2[i] - a1) + (b1 * (y2[3] - b1) ) + (rho20 * (y[i] - a1) * (y[3] - b1)));
            sum += XX[i][4] * exp(a1 * (y2[i] - a1) + (b1 * (y2[4] - b1) ) + (rho20 * (y[i] - a1) * (y[4] - b1)));
        }

        result = sqrt(1.0 - pow2(Rho)) / pi * sum;
    }
    else if (( a <= 0.0 ) && ( b >= 0.0 ) && ( Rho >= 0.0 ))
    {
        result = cnd(a) - cbnd(a, -b, -Rho);
    }
    else if (( a >= 0.0 ) && ( b <= 0.0 ) && ( Rho >= 0.0 ))
    {
        result = cnd(b) - cbnd(-a, b, -Rho);
    }
    else if (( a >= 0.0 ) && ( b >= 0.0 ) && ( Rho <= 0.0 ))
    {
        result = cnd(a) + cnd(b) - 1.0 + cbnd(-a, -b, Rho);
    }
    else if ( (a * b * Rho) > 0.0 )
    {
        const double sp2a = sqrt(pow2(a) - (Rho * 2.0 * a * b) + pow2(b));

        const double rho1 = (Rho * a - b) * sign(a) / sp2a;
        const double rho2 = (Rho * b - a) * sign(b) / sp2a;

        const double Delta = (1.0 - sign(a) * sign(b)) / 4.0;

        result = cbnd(a, 0.0, rho1) + cbnd(b, 0.0, rho2) - Delta;
    }
    else
    {
        /* Hmm, illegal input? Abort to be on the safe side */
        assert(0);
        abort();
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#if defined( QT_DEBUG )

#define Q_ASSERT_DOUBLE( fn, v ) {const double result = fn; Q_ASSERT( v-0.000001 <= result && result <= v+0.000001 );}

void cbnd_validate()
{
    Q_ASSERT_DOUBLE( cbnd( 0.0,  0.0,  0.0), 0.250000 );
    Q_ASSERT_DOUBLE( cbnd( 0.0,  0.0, -0.5), 0.166667 );
    Q_ASSERT_DOUBLE( cbnd( 0.0,  0.0,  0.5), 0.333333 );
    Q_ASSERT_DOUBLE( cbnd( 0.0, -0.5,  0.0), 0.154269 );
    Q_ASSERT_DOUBLE( cbnd( 0.0, -0.5, -0.5), 0.081660 );
    Q_ASSERT_DOUBLE( cbnd( 0.0, -0.5,  0.5), 0.226878 );
    Q_ASSERT_DOUBLE( cbnd( 0.0,  0.5,  0.0), 0.345731 );
    Q_ASSERT_DOUBLE( cbnd( 0.0,  0.5, -0.5), 0.273122 );
    Q_ASSERT_DOUBLE( cbnd( 0.0,  0.5,  0.5), 0.418340 );

    Q_ASSERT_DOUBLE( cbnd(-0.5,  0.0,  0.0), 0.154269 );
    Q_ASSERT_DOUBLE( cbnd(-0.5,  0.0, -0.5), 0.081660 );
    Q_ASSERT_DOUBLE( cbnd(-0.5,  0.0,  0.5), 0.226878 );
    Q_ASSERT_DOUBLE( cbnd(-0.5, -0.5,  0.0), 0.095195 );
    Q_ASSERT_DOUBLE( cbnd(-0.5, -0.5, -0.5), 0.036298 );
    Q_ASSERT_DOUBLE( cbnd(-0.5, -0.5,  0.5), 0.163319 );
    Q_ASSERT_DOUBLE( cbnd(-0.5,  0.5,  0.0), 0.213342 );
    Q_ASSERT_DOUBLE( cbnd(-0.5,  0.5, -0.5), 0.145218 );
    Q_ASSERT_DOUBLE( cbnd(-0.5,  0.5,  0.5), 0.272239 );

    Q_ASSERT_DOUBLE( cbnd( 0.5,  0.0,  0.0), 0.345731 );
    Q_ASSERT_DOUBLE( cbnd( 0.5,  0.0, -0.5), 0.273122 );
    Q_ASSERT_DOUBLE( cbnd( 0.5,  0.0,  0.5), 0.418340 );
    Q_ASSERT_DOUBLE( cbnd( 0.5, -0.5,  0.0), 0.213342 );
    Q_ASSERT_DOUBLE( cbnd( 0.5, -0.5, -0.5), 0.145218 );
    Q_ASSERT_DOUBLE( cbnd( 0.5, -0.5,  0.5), 0.272239 );
    Q_ASSERT_DOUBLE( cbnd( 0.5,  0.5,  0.0), 0.478120 );
    Q_ASSERT_DOUBLE( cbnd( 0.5,  0.5, -0.5), 0.419223 );
    Q_ASSERT_DOUBLE( cbnd( 0.5,  0.5,  0.5), 0.546244 );
}
#endif

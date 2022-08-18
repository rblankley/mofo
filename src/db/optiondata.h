/**
 * @file optiondata.h
 * Option Data.
 *
 * @copyright Copyright (C) 2022 Randy Blankley. All rights reserved.
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

#ifndef OPTIONDATA_H
#define OPTIONDATA_H

#include <QDateTime>
#include <QMap>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Option Chain Curves.
struct OptionChainCurves
{
    QMap<double, double> volatility;
    QMap<double, double> callVolatility;
    QMap<double, double> putVolatility;

    QMap<double, double> itmProbability;
    QMap<double, double> otmProbability;

    QMap<double, int> callOpenInterest;
    QMap<double, int> putOpenInterest;
};

/// Option Chain Open Interest
struct OptionChainOpenInterest
{
    QMap<double, int> callOpenInterest;
    QMap<double, int> putOpenInterest;

    QMap<double, int> callTotalVolume;
    QMap<double, int> putTotalVolume;
};

/// Future Volatility Information (i.e. Estimated Movement).
struct FutureVolatilities
{
    QDateTime stamp;                                ///< Stamp of when option chain data was retrieved.
    int dte;                                        ///< Trading days to expiration.

    double hist;                                    ///< Historical volatility for same period (DTE).
    double impl;                                    ///< Implied volatility.

    double strike;                                  ///< Strike price for implied volatility.
    bool analyzed;                                  ///< @c true if implied volatility was computed from analysis, @c false otherwise.
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // OPTIONDATA_H

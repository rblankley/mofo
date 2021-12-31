/**
 * @file candledata.h
 * Candle Data.
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

#ifndef CANDLEDATA_H
#define CANDLEDATA_H

#include <QDateTime>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Candle Data.
struct CandleData
{
    QDateTime stamp;
    double openPrice;
    double highPrice;
    double lowPrice;
    double closePrice;
    unsigned long long totalVolume;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // CANDLEDATA_H

/**
 * @file stringsxml.h
 * String values for U.S. Department of the Treasury documents.
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

#ifndef STRINGSXML_H
#define STRINGSXML_H

#include <QString>

///////////////////////////////////////////////////////////////////////////////////////////////////

inline static const QString XML_FEED                                = "feed";
inline static const QString XML_TITLE                               = "title";
inline static const QString XML_UPDATED                             = "updated";

// Treasury Bill Rate Data
inline static const QString XML_ENTRY                               = "entry";
inline static const QString XML_CONTENT                             = "content";

inline static const QString XML_PROPERTIES                          = "m:properties";

inline static const QString XML_DATA_ID                             = "d:DailyTreasuryBillRateDataId";
inline static const QString XML_INDEX_DATE                          = "d:INDEX_DATE";
inline static const QString XML_ROUND_B1_CLOSE_4WK_2                = "d:ROUND_B1_CLOSE_4WK_2";
inline static const QString XML_ROUND_B1_YIELD_4WK_2                = "d:ROUND_B1_YIELD_4WK_2";
inline static const QString XML_ROUND_B1_CLOSE_8WK_2                = "d:ROUND_B1_CLOSE_8WK_2";
inline static const QString XML_ROUND_B1_YIELD_8WK_2                = "d:ROUND_B1_YIELD_8WK_2";
inline static const QString XML_ROUND_B1_CLOSE_13WK_2               = "d:ROUND_B1_CLOSE_13WK_2";
inline static const QString XML_ROUND_B1_YIELD_13WK_2               = "d:ROUND_B1_YIELD_13WK_2";
inline static const QString XML_ROUND_B1_CLOSE_26WK_2               = "d:ROUND_B1_CLOSE_26WK_2";
inline static const QString XML_ROUND_B1_YIELD_26WK_2               = "d:ROUND_B1_YIELD_26WK_2";
inline static const QString XML_ROUND_B1_CLOSE_52WK_2               = "d:ROUND_B1_CLOSE_52WK_2";
inline static const QString XML_ROUND_B1_YIELD_52WK_2               = "d:ROUND_B1_YIELD_52WK_2";
inline static const QString XML_BOND_MKT_UNAVAIL_REASON             = "d:BOND_MKT_UNAVAIL_REASON";
inline static const QString XML_MATURITY_DATE_4WK                   = "d:MATURITY_DATE_4WK";
inline static const QString XML_MATURITY_DATE_8WK                   = "d:MATURITY_DATE_8WK";
inline static const QString XML_MATURITY_DATE_13WK                  = "d:MATURITY_DATE_13WK";
inline static const QString XML_MATURITY_DATE_26WK                  = "d:MATURITY_DATE_26WK";
inline static const QString XML_MATURITY_DATE_52WK                  = "d:MATURITY_DATE_52WK";
inline static const QString XML_CUSIP_4WK                           = "d:CUSIP_4WK";
inline static const QString XML_CUSIP_8WK                           = "d:CUSIP_8WK";
inline static const QString XML_CUSIP_13WK                          = "d:CUSIP_13WK";
inline static const QString XML_CUSIP_26WK                          = "d:CUSIP_26WK";
inline static const QString XML_CUSIP_52WK                          = "d:CUSIP_52WK";
inline static const QString XML_QUOTE_DATE                          = "d:QUOTE_DATE";
inline static const QString XML_CF_NEW_DATE                         = "d:CF_NEW_DATE";
inline static const QString XML_CS_4WK_CLOSE_AVG                    = "d:CS_4WK_CLOSE_AVG";
inline static const QString XML_CS_4WK_YIELD_AVG                    = "d:CS_4WK_YIELD_AVG";
inline static const QString XML_CS_8WK_CLOSE_AVG                    = "d:CS_8WK_CLOSE_AVG";
inline static const QString XML_CS_8WK_YIELD_AVG                    = "d:CS_8WK_YIELD_AVG";
inline static const QString XML_CS_13WK_CLOSE_AVG                   = "d:CS_13WK_CLOSE_AVG";
inline static const QString XML_CS_13WK_YIELD_AVG                   = "d:CS_13WK_YIELD_AVG";
inline static const QString XML_CS_26WK_CLOSE_AVG                   = "d:CS_26WK_CLOSE_AVG";
inline static const QString XML_CS_26WK_YIELD_AVG                   = "d:CS_26WK_YIELD_AVG";
inline static const QString XML_CS_52WK_CLOSE_AVG                   = "d:CS_52WK_CLOSE_AVG";
inline static const QString XML_CS_52WK_YIELD_AVG                   = "d:CS_52WK_YIELD_AVG";
inline static const QString XML_CF_WEEK                             = "d:CF_WEEK";

// Treasury Yield Curve Rate Data
inline static const QString XML_ID                                  = "d:Id";
inline static const QString XML_NEW_DATE                            = "d:NEW_DATE";

inline static const QString XML_BC_1MONTH                           = "d:BC_1MONTH";
inline static const QString XML_BC_2MONTH                           = "d:BC_2MONTH";
inline static const QString XML_BC_3MONTH                           = "d:BC_3MONTH";
inline static const QString XML_BC_6MONTH                           = "d:BC_6MONTH";
inline static const QString XML_BC_1YEAR                            = "d:BC_1YEAR";
inline static const QString XML_BC_2YEAR                            = "d:BC_2YEAR";
inline static const QString XML_BC_3YEAR                            = "d:BC_3YEAR";
inline static const QString XML_BC_5YEAR                            = "d:BC_5YEAR";
inline static const QString XML_BC_7YEAR                            = "d:BC_7YEAR";
inline static const QString XML_BC_10YEAR                           = "d:BC_10YEAR";
inline static const QString XML_BC_20YEAR                           = "d:BC_20YEAR";
inline static const QString XML_BC_30YEAR                           = "d:BC_30YEAR";
inline static const QString XML_BC_30YEARDISPLAY                    = "d:BC_30YEARDISPLAY";

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // STRINGSXML_H

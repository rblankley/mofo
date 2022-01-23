/**
 * @file fundamentalstablemodel.cpp
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

#include "appdb.h"
#include "common.h"
#include "fundamentalstablemodel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
FundamentalsTableModel::FundamentalsTableModel( const QString& symbol, const QDateTime& stamp, QObject *parent ) :
    _Mybase( _NUM_COLUMNS, parent, AppDatabase::instance()->openDatabaseConnection( symbol ) ),
    symbol_( symbol )
{
    // setup filter
    QString filter;

    if ( stamp.isValid() )
        filter += "DATETIME('" + stamp.toString( Qt::ISODateWithMs ) + "')=DATETIME(stamp)";
    else
        filter += "stamp=(SELECT MAX(stamp) FROM fundamentals)";

    filter += " AND '" + symbol + "'=symbol";

    // setup view
    setTable( "fundamentals" );
    setFilter( filter );

    // text columns
    columnIsText_[STAMP] = true;

    columnIsText_[SYMBOL] = true;

    columnIsText_[DIV_DATE] = true;
    columnIsText_[DIV_FREQUENCY] = true;
    columnIsText_[DIV_PAY_DATE] = true;

    // number of decimal places
    numDecimalPlaces_[HIGH52] = 2;
    numDecimalPlaces_[LOW52] = 2;

    numDecimalPlaces_[DIV_AMOUNT] = 2;
    numDecimalPlaces_[DIV_YIELD] = 2;

    numDecimalPlaces_[PE_RATIO] = 5;
    numDecimalPlaces_[PEG_RATIO] = 5;
    numDecimalPlaces_[PB_RATIO] = 5;
    numDecimalPlaces_[PR_RATIO] = 5;
    numDecimalPlaces_[PCF_RATIO] = 5;

    numDecimalPlaces_[GROSS_MARGIN_TTM] = 5;
    numDecimalPlaces_[GROSS_MARGIN_MRQ] = 5;
    numDecimalPlaces_[NET_PROFIT_MARGIN_TTM] = 5;
    numDecimalPlaces_[NET_PROFIT_MARGIN_MRQ] = 5;
    numDecimalPlaces_[OPERATING_MARGIN_TTM] = 5;
    numDecimalPlaces_[OPERATING_MARGIN_MRQ] = 5;

    numDecimalPlaces_[RETURN_ON_EQUITY] = 5;
    numDecimalPlaces_[RETURN_ON_ASSETS] = 5;
    numDecimalPlaces_[RETURN_ON_INVESTMENT] = 5;

    numDecimalPlaces_[QUICK_RATIO] = 5;
    numDecimalPlaces_[CURRENT_RATIO] = 5;

    numDecimalPlaces_[INTEREST_COVERAGE] = 5;
    numDecimalPlaces_[TOTAL_DEBT_TO_CAPITAL] = 5;
    numDecimalPlaces_[LT_DEBT_TO_EQUITY] = 5;
    numDecimalPlaces_[TOTAL_DEBT_TO_EQUITY] = 5;

    numDecimalPlaces_[EPS_TTM] = 5;
    numDecimalPlaces_[EPS_CHANGE_PERCENT_TTM] = 5;
    numDecimalPlaces_[EPS_CHANGE_YEAR] = 5;
    numDecimalPlaces_[EPS_CHANGE] = 5;

    numDecimalPlaces_[REV_CHANGE_YEAR] = 5;
    numDecimalPlaces_[REV_CHANGE_TTM] = 5;
    numDecimalPlaces_[REV_CHANGE_IN] = 5;

    numDecimalPlaces_[MARKET_CAP_FLOAT] = 4;
    numDecimalPlaces_[MARKET_CAP] = 2;

    numDecimalPlaces_[BOOK_VALUE_PER_SHARE] = 5;
    numDecimalPlaces_[SHORT_INT_TO_FLOAT] = 5;
    numDecimalPlaces_[SHORT_INT_DAY_TO_COVER] = 5;
    numDecimalPlaces_[DIV_GROWTH_RATE_3YEAR] = 5;
    numDecimalPlaces_[DIV_PAY_AMOUNT] = 2;

    numDecimalPlaces_[BETA] = 5;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
FundamentalsTableModel::~FundamentalsTableModel()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString FundamentalsTableModel::columnDescription( int col ) const
{
    switch ( (ColumnIndex) col )
    {
    case STAMP:
        return tr( "Stamp" );
    case SYMBOL:
        return tr( "Symbol" );
    case HIGH52:
        return tr( "52 Week High" );
    case LOW52:
        return tr( "52 Week Low" );
    case DIV_AMOUNT:
        return tr( "Dividend Amount" );
    case DIV_YIELD:
        return tr( "Dividend Yield" );
    case DIV_DATE:
        return tr( "Dividend Date" );
    case DIV_FREQUENCY:
        return tr( "Dividend Frequency" );
    case PE_RATIO:
        return tr( "P/E Ratio" );
    case PEG_RATIO:
        return tr( "PEG Ratio" );
    case PB_RATIO:
        return tr( "P/B Ratio" );
    case PR_RATIO:
        return tr( "P/R Ratio" );
    case PCF_RATIO:
        return tr( "P/CF Ratio" );
    case GROSS_MARGIN_TTM:
        return tr( "Gross Margin - TTM" );
    case GROSS_MARGIN_MRQ:
        return tr( "Gross Margin - MRQ" );
    case NET_PROFIT_MARGIN_TTM:
        return tr( "Net Profit Margin - TTM" );
    case NET_PROFIT_MARGIN_MRQ:
        return tr( "Net Profit Margin - MRQ" );
    case OPERATING_MARGIN_TTM:
        return tr( "Operating Margin - TTM" );
    case OPERATING_MARGIN_MRQ:
        return tr( "Operating Margin - MRQ" );
    case RETURN_ON_EQUITY:
        return tr( "Return on Equity (ROE)" );
    case RETURN_ON_ASSETS:
        return tr( "Return on Assets (ROA)" );
    case RETURN_ON_INVESTMENT:
        return tr( "Return on Investment (ROI)" );
    case QUICK_RATIO:
        return tr( "Quick Ratio" );
    case CURRENT_RATIO:
        return tr( "Current Ratio" );
    case INTEREST_COVERAGE:
        return tr( "Interest Coverage" );
    case TOTAL_DEBT_TO_CAPITAL:
        return tr( "Total Debt to Capital (D/C Ratio)" );
    case LT_DEBT_TO_EQUITY:
        return tr( "Long Term Debt to Equity" );
    case TOTAL_DEBT_TO_EQUITY:
        return tr( "Debt to Equity (D/E Ratio)" );
    case EPS_TTM:
        return tr( "Earnings per Share (EPS) - TTM" );
    case EPS_CHANGE_PERCENT_TTM:
        return tr( "Earnings per Share Change Percent - TTM" );
    case EPS_CHANGE_YEAR:
        return tr( "Earnings per Share Change Year" );
    case EPS_CHANGE:
        return tr( "Earnings per Share Change" );
    case REV_CHANGE_YEAR:
        return tr( "Revenue Change Year" );
    case REV_CHANGE_TTM:
        return tr( "Revenue Change - TTM" );
    case REV_CHANGE_IN:
        return tr( "Revenue Change In" );
    case SHARES_OUTSTANDING:
        return tr( "Shares Outstanding" );
    case MARKET_CAP_FLOAT:
        return tr( "Free-Float Market Cap" );
    case MARKET_CAP:
        return tr( "Market Cap" );
    case BOOK_VALUE_PER_SHARE:
        return tr( "Book Value per Share" );
    case SHORT_INT_TO_FLOAT:
        return tr( "Short Interest to Float" );
    case SHORT_INT_DAY_TO_COVER:
        return tr( "Short Interest Day to Cover" );
    case DIV_GROWTH_RATE_3YEAR:
        return tr( "3 Year Dividend Growth Rate" );
    case DIV_PAY_AMOUNT:
        return tr( "Dividend Pay Amount" );
    case DIV_PAY_DATE:
        return tr( "Dividend Pay Date" );
    case BETA:
        return tr( "Beta" );
    case VOL_1DAY_AVG:
        return tr( "Average Volume - 1 Day" );
    case VOL_10DAY_AVG:
        return tr( "Average Volume - 10 Day" );
    case VOL_3MONTH_AVG:
        return tr( "Average Volume - 3 Month" );
    default:
        break;
    }

    return QString();
}

/**
 * @file quotetablemodel.cpp
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
#include "quotetablemodel.h"
#include "symboldbs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
QuoteTableModel::QuoteTableModel( const QString& symbol, const QDateTime& stamp, QObject *parent ) :
    _Mybase( _NUM_COLUMNS, parent, SymbolDatabases::instance()->openDatabaseConnection( symbol ) ),
    symbol_( symbol )
{
    // setup filter
    QString filter;

    if ( stamp.isValid() )
        filter += "DATETIME('" + stamp.toString( Qt::ISODateWithMs ) + "')=DATETIME(stamp)";
    else
        filter += "stamp=(SELECT MAX(stamp) FROM quotes)";

    filter += " AND '" + symbol + "'=symbol";

    // setup view
    setTable( "quotes" );
    setFilter( filter );

    // text columns
    columnIsText_[STAMP] = true;

    columnIsText_[SYMBOL] = true;
    columnIsText_[DESCRIPTION] = true;

    columnIsText_[ASSET_MAIN_TYPE] = true;
    columnIsText_[ASSET_SUB_TYPE] = true;
    columnIsText_[ASSET_TYPE] = true;
    columnIsText_[CUSIP] = true;

    columnIsText_[BID_ASK_SIZE] = true;
    columnIsText_[BID_ID] = true;
    columnIsText_[BID_TICK] = true;
    columnIsText_[ASK_ID] = true;
    columnIsText_[LAST_ID] = true;

    columnIsText_[QUOTE_TIME] = true;
    columnIsText_[TRADE_TIME] = true;

    columnIsText_[EXCHANGE] = true;
    columnIsText_[EXCHANGE_NAME] = true;

    columnIsText_[DIV_DATE] = true;
    columnIsText_[DIV_FREQUENCY] = true;
    columnIsText_[SECURITY_STATUS] = true;

    columnIsText_[REG_MARKET_TRADE_TIME] = true;

    columnIsText_[TICK] = true;
    columnIsText_[PRODUCT] = true;
    columnIsText_[TRADING_HOURS] = true;

    // number of decimal places
    numDecimalPlaces_[BID_PRICE] = 2;
    numDecimalPlaces_[ASK_PRICE] = 2;
    numDecimalPlaces_[LAST_PRICE] = 2;

    numDecimalPlaces_[OPEN_PRICE] = 2;
    numDecimalPlaces_[HIGH_PRICE] = 2;
    numDecimalPlaces_[LOW_PRICE] = 2;
    numDecimalPlaces_[CLOSE_PRICE] = 2;

    numDecimalPlaces_[CHANGE] = 2;
    numDecimalPlaces_[PERCENT_CHANGE] = 2;

    numDecimalPlaces_[MARK] = 2;
    numDecimalPlaces_[MARK_CHANGE] = 2;
    numDecimalPlaces_[MARK_PERCENT_CHANGE] = 2;

    numDecimalPlaces_[FIFTY_TWO_WEEK_HIGH] = 2;
    numDecimalPlaces_[FIFTY_TWO_WEEK_LOW] = 2;
    numDecimalPlaces_[PERCENT_BELOW_FIFTY_TWO_WEEK_HIGH] = 2;
    numDecimalPlaces_[PERCENT_ABOVE_FIFTY_TWO_WEEK_LOW] = 2;
    numDecimalPlaces_[FIFTY_TWO_WEEK_PRICE_RANGE] = 2;

    numDecimalPlaces_[VOLATILITY] = 5;

    numDecimalPlaces_[NAV] = 5;
    numDecimalPlaces_[PE_RATIO] = 5;
    numDecimalPlaces_[IMPLIED_YIELD] = 2;

    numDecimalPlaces_[DIV_AMOUNT] = 2;
    numDecimalPlaces_[DIV_YIELD] = 2;

    numDecimalPlaces_[REG_MARKET_LAST_PRICE] = 2;
    numDecimalPlaces_[REG_MARKET_CHANGE] = 2;
    numDecimalPlaces_[REG_MARKET_PERCENT_CHANGE] = 2;

    numDecimalPlaces_[TICK_AMOUNT] = 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QuoteTableModel::~QuoteTableModel()
{
    // remove reference
    SymbolDatabases::instance()->removeRef( symbol_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString QuoteTableModel::columnDescription( int col ) const
{
    switch ( (ColumnIndex) col )
    {
    case STAMP:
        return tr( "Stamp" );
    case SYMBOL:
        return tr( "Symbol" );
    case DESCRIPTION:
        return tr( "Description" );
    case ASSET_MAIN_TYPE:
        return tr( "Asset Main Type" );
    case ASSET_SUB_TYPE:
        return tr( "Asset Sub Type" );
    case ASSET_TYPE:
        return tr( "Asset Type" );
    case CUSIP:
        return tr( "CUSIP" );
    case BID_ASK_SIZE:
        return tr( "Bid/Ask Size" );
    case BID_PRICE:
        return tr( "Bid Price" );
    case BID_SIZE:
        return tr( "Bid Size" );
    case BID_ID:
        return tr( "Bid Id" );
    case BID_TICK:
        return tr( "Bid Tick" );
    case ASK_PRICE:
        return tr( "Ask Price" );
    case ASK_SIZE:
        return tr( "Ask Size" );
    case ASK_ID:
        return tr( "Ask Id" );
    case LAST_PRICE:
        return tr( "Last Price" );
    case LAST_SIZE:
        return tr( "Last Size" );
    case LAST_ID:
        return tr( "Last Id" );
    case OPEN_PRICE:
        return tr( "Open Price" );
    case HIGH_PRICE:
        return tr( "High Price" );
    case LOW_PRICE:
        return tr( "Low Price" );
    case CLOSE_PRICE:
        return tr( "Close Price" );
    case CHANGE:
        return tr( "Change" );
    case PERCENT_CHANGE:
        return tr( "Percent Change" );
    case TOTAL_VOLUME:
        return tr( "Total Volume" );
    case QUOTE_TIME:
        return tr( "Quote Time" );
    case TRADE_TIME:
        return tr( "Trade Time" );
    case MARK:
        return tr( "Mark" );
    case MARK_CHANGE:
        return tr( "Mark Change" );
    case MARK_PERCENT_CHANGE:
        return tr( "Mark Percent Change" );
    case FIFTY_TWO_WEEK_HIGH:
        return tr( "52 Week High" );
    case FIFTY_TWO_WEEK_LOW:
        return tr( "52 Week Low" );
    case PERCENT_BELOW_FIFTY_TWO_WEEK_HIGH:
        return tr( "Percent Below 52 Week High" );
    case PERCENT_ABOVE_FIFTY_TWO_WEEK_LOW:
        return tr( "Percent Above 52 Week Low" );
    case FIFTY_TWO_WEEK_PRICE_RANGE:
        return tr( "52 Week Price Range" );
    case EXCHANGE:
        return tr( "Exchange" );
    case EXCHANGE_NAME:
        return tr( "Exchange Name" );
    case IS_MARGINABLE:
        return tr( "Is Marginable" );
    case IS_SHORTABLE:
        return tr( "Is Shortable" );
    case IS_DELAYED:
        return tr( "Is Delayed" );
    case VOLATILITY:
        return tr( "Volatility" );
    case DIGITS:
        return tr( "Digits" );
    case NAV:
        return tr( "Net Asset Value" );
    case PE_RATIO:
        return tr( "P/E Ratio" );
    case IMPLIED_YIELD:
        return tr( "Implied Yield" );
    case DIV_AMOUNT:
        return tr( "Dividend Amount" );
    case DIV_YIELD:
        return tr( "Dividend Yield" );
    case DIV_DATE:
        return tr( "Dividend Date" );
    case DIV_FREQUENCY:
        return tr( "Dividend Frequency" );
    case SECURITY_STATUS:
        return tr( "Security Status" );
    case REG_MARKET_LAST_PRICE:
        return tr( "Regular Market Last Price" );
    case REG_MARKET_LAST_SIZE:
        return tr( "Regular Market Last Size" );
    case REG_MARKET_CHANGE:
        return tr( "Regular Market Change" );
    case REG_MARKET_PERCENT_CHANGE:
        return tr( "Regular Market Percent Change" );
    case REG_MARKET_TRADE_TIME:
        return tr( "Regular Market Trade Time" );
    case TICK:
        return tr( "Tick" );
    case TICK_AMOUNT:
        return tr( "Tick Amount" );
    case PRODUCT:
        return tr( "Product" );
    case TRADING_HOURS:
        return tr( "Trading Hours" );
    case IS_TRADABLE:
        return tr( "Is Tradable" );
    case MARKET_MAKER:
        return tr( "Market Maker" );
    default:
        break;
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double QuoteTableModel::mark() const
{
    return data0( MARK ).toDouble();
}

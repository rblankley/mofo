/**
 * @file optionchaintablemodel.cpp
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
#include "optionchaintablemodel.h"

#include <QPalette>

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionChainTableModel::OptionChainTableModel( const QString& symbol, const QDate& expiryDate, const QDateTime& stamp, QObject *parent ) :
    _Mybase( _NUM_COLUMNS, parent, AppDatabase::instance()->openDatabaseConnection( symbol ) ),
    symbol_( symbol ),
    expiryDate_( expiryDate )
{
    // setup filter
    QString filter;

    if ( stamp.isValid() )
        filter += "DATETIME('" + stamp.toString( Qt::ISODateWithMs ) + "')=DATETIME(stamp)";
    else
        filter += "stamp=(SELECT MAX(stamp) FROM optionChainView)";

    filter += " AND '" + symbol + "'=underlying";
    filter += " AND DATE('" + expiryDate.toString( Qt::ISODate ) + "')=DATE(expirationDate)";

    // setup view
    setTable( "optionChainView" );
    setFilter( filter );

    // text columns
    columnIsText_[STAMP] = true;
    columnIsText_[UNDERLYING] = true;
    columnIsText_[EXPIRY_DATE] = true;

    columnIsText_[CALL_SYMBOL] = columnIsText_[PUT_SYMBOL] = true;
    columnIsText_[CALL_DESC] = columnIsText_[PUT_DESC] = true;

    columnIsText_[CALL_BID_ASK_SIZE] = columnIsText_[PUT_BID_ASK_SIZE] = true;

    columnIsText_[CALL_QUOTE_TIME] = columnIsText_[PUT_QUOTE_TIME] = true;
    columnIsText_[CALL_TRADE_TIME] = columnIsText_[PUT_TRADE_TIME] = true;

    columnIsText_[CALL_EXCHANGE_NAME] = columnIsText_[PUT_EXCHANGE_NAME] = true;

    columnIsText_[CALL_EXPIRY_DATE] = columnIsText_[PUT_EXPIRY_DATE] = true;
    columnIsText_[CALL_EXPIRY_TYPE] = columnIsText_[PUT_EXPIRY_TYPE] = true;

    columnIsText_[CALL_LAST_TRADING_DAY] = columnIsText_[PUT_LAST_TRADING_DAY] = true;

    columnIsText_[CALL_SETTLEMENT_TYPE] = columnIsText_[PUT_SETTLEMENT_TYPE] = true;
    columnIsText_[CALL_DELIVERABLE_NOTE] = columnIsText_[PUT_DELIVERABLE_NOTE] = true;

    // number of decimal places
    numDecimalPlaces_[STRIKE_PRICE] = 2;

    numDecimalPlaces_[CALL_BID_PRICE] = numDecimalPlaces_[PUT_BID_PRICE] = 2;
    numDecimalPlaces_[CALL_ASK_PRICE] = numDecimalPlaces_[PUT_ASK_PRICE] = 2;
    numDecimalPlaces_[CALL_LAST_PRICE] = numDecimalPlaces_[PUT_LAST_PRICE] = 2;

    numDecimalPlaces_[CALL_BREAK_EVEN_PRICE] = numDecimalPlaces_[PUT_BREAK_EVEN_PRICE] = 2;
    numDecimalPlaces_[CALL_INTRINSIC_VALUE] = numDecimalPlaces_[PUT_INTRINSIC_VALUE] = 2;

    numDecimalPlaces_[CALL_OPEN_PRICE] = numDecimalPlaces_[PUT_OPEN_PRICE] = 2;
    numDecimalPlaces_[CALL_HIGH_PRICE] = numDecimalPlaces_[PUT_HIGH_PRICE] = 2;
    numDecimalPlaces_[CALL_LOW_PRICE] = numDecimalPlaces_[PUT_LOW_PRICE] = 2;
    numDecimalPlaces_[CALL_CLOSE_PRICE] = numDecimalPlaces_[PUT_CLOSE_PRICE] = 2;

    numDecimalPlaces_[CALL_CHANGE] = numDecimalPlaces_[PUT_CHANGE] = 2;
    numDecimalPlaces_[CALL_PERCENT_CHANGE] = numDecimalPlaces_[PUT_PERCENT_CHANGE] = 2;

    numDecimalPlaces_[CALL_MARK] = numDecimalPlaces_[PUT_MARK] = 2;
    numDecimalPlaces_[CALL_MARK_CHANGE] = numDecimalPlaces_[PUT_MARK_CHANGE] = 2;
    numDecimalPlaces_[CALL_MARK_PERCENT_CHANGE] = numDecimalPlaces_[PUT_MARK_PERCENT_CHANGE] = 2;

    numDecimalPlaces_[CALL_VOLATILITY] = numDecimalPlaces_[PUT_VOLATILITY] = 4;
    numDecimalPlaces_[CALL_DELTA] = numDecimalPlaces_[PUT_DELTA] = 4;
    numDecimalPlaces_[CALL_GAMMA] = numDecimalPlaces_[PUT_GAMMA] = 4;
    numDecimalPlaces_[CALL_THETA] = numDecimalPlaces_[PUT_THETA] = 4;
    numDecimalPlaces_[CALL_VEGA] = numDecimalPlaces_[PUT_VEGA] = 4;
    numDecimalPlaces_[CALL_RHO] = numDecimalPlaces_[PUT_RHO] = 4;

    numDecimalPlaces_[CALL_TIME_VALUE] = numDecimalPlaces_[PUT_TIME_VALUE] = 2;
    numDecimalPlaces_[CALL_THEO_OPTION_VALUE] = numDecimalPlaces_[PUT_THEO_OPTION_VALUE] = 2;
    numDecimalPlaces_[CALL_THEO_VOLATILITY] = numDecimalPlaces_[PUT_THEO_VOLATILITY] = 4;

    // create mapping from bid/ask to bid/ask size
    bidAskSize_[CALL_BID_PRICE] = CALL_BID_SIZE;
    bidAskSize_[CALL_ASK_PRICE] = CALL_ASK_SIZE;
    bidAskSize_[PUT_BID_PRICE] = PUT_BID_SIZE;
    bidAskSize_[PUT_ASK_PRICE] = PUT_ASK_SIZE;

    // color of money!!!!
    inTheMoneyColor_ = Qt::green;
    inTheMoneyColor_.setAlpha( 32 );

    const QPalette p;

    strikeColor_ = p.color( QPalette::Button );

    textColor_ = p.color( QPalette::Active, QPalette::Text );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionChainTableModel::~OptionChainTableModel()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString OptionChainTableModel::columnDescription( int col ) const
{
    switch ( (ColumnIndex) col )
    {
    case STAMP:
        return tr( "Stamp" );
    case UNDERLYING:
        return tr( "Underlying Symbol" );
    case EXPIRY_DATE:
        return tr( "Expiration Date" );

    case CALL_SYMBOL:
        return tr( "Call Symbol" );
    case CALL_DESC:
        return tr( "Call Description" );
    case CALL_BID_ASK_SIZE:
        return tr( "Call Bid/Ask Size" );
    case CALL_BID_PRICE:
        return tr( "Call Bid Price" );
    case CALL_BID_SIZE:
        return tr( "Call Bid Size" );
    case CALL_ASK_PRICE:
        return tr( "Call Ask Price" );
    case CALL_ASK_SIZE:
        return tr( "Call Ask Size" );
    case CALL_LAST_PRICE:
        return tr( "Call Last Price" );
    case CALL_LAST_SIZE:
        return tr( "Call Last Size" );
    case CALL_BREAK_EVEN_PRICE:
        return tr( "Call Break Even Price" );
    case CALL_INTRINSIC_VALUE:
        return tr( "Call Intrinsic Value" );
    case CALL_OPEN_PRICE:
        return tr( "Call Open Price" );
    case CALL_HIGH_PRICE:
        return tr( "Call High Price" );
    case CALL_LOW_PRICE:
        return tr( "Call Low Price" );
    case CALL_CLOSE_PRICE:
        return tr( "Call Close Price" );
    case CALL_CHANGE:
        return tr( "Call Change" );
    case CALL_PERCENT_CHANGE:
        return tr( "Call Percent Change" );
    case CALL_TOTAL_VOLUME:
        return tr( "Call Volume" );
    case CALL_QUOTE_TIME:
        return tr( "Call Quote Time" );
    case CALL_TRADE_TIME:
        return tr( "Call Trade Time" );
    case CALL_MARK:
        return tr( "Call Mark" );
    case CALL_MARK_CHANGE:
        return tr( "Call Mark Change" );
    case CALL_MARK_PERCENT_CHANGE:
        return tr( "Call Mark Percent Change" );
    case CALL_EXCHANGE_NAME:
        return tr( "Call Exchange" );
    case CALL_VOLATILITY:
        return tr( "Call Volatility" );
    case CALL_DELTA:
        return tr( "Call Delta" );
    case CALL_GAMMA:
        return tr( "Call Gamma" );
    case CALL_THETA:
        return tr( "Call Theta" );
    case CALL_VEGA:
        return tr( "Call Vega" );
    case CALL_RHO:
        return tr( "Call Rho" );
    case CALL_TIME_VALUE:
        return tr( "Call Time Value" );
    case CALL_OPEN_INTEREST:
        return tr( "Call Open Interest" );
    case CALL_IS_IN_THE_MONEY:
        return tr( "Call In The Money" );
    case CALL_THEO_OPTION_VALUE:
        return tr( "Call Theoretical Value" );
    case CALL_THEO_VOLATILITY:
        return tr( "Call Theoretical Volatility" );
    case CALL_IS_MINI:
        return tr( "Call Is Mini" );
    case CALL_IS_NON_STANDARD:
        return tr( "Call Is Non-Standard" );
    case CALL_IS_INDEX:
        return tr( "Call Is Index" );
    case CALL_IS_WEEKLY:
        return tr( "Call Is Weekly" );
    case CALL_IS_QUARTERLY:
        return tr( "Call Is Quarterly" );
    case CALL_EXPIRY_DATE:
        return tr( "Call Expiration Date" );
    case CALL_EXPIRY_TYPE:
        return tr( "Call Expiration Type" );
    case CALL_DAYS_TO_EXPIRY:
        return tr( "Call Days to Expiration" );
    case CALL_LAST_TRADING_DAY:
        return tr( "Call Last Trading Day" );
    case CALL_MULTIPLIER:
        return tr( "Call Multiplier" );
    case CALL_SETTLEMENT_TYPE:
        return tr( "Call Settlement Type" );
    case CALL_DELIVERABLE_NOTE:
        return tr( "Call Deliverable Note" );

    case STRIKE_PRICE:
        return tr( "Strike Price" );

    case PUT_SYMBOL:
        return tr( "Put Symbol" );
    case PUT_DESC:
        return tr( "Put Description" );
    case PUT_BID_ASK_SIZE:
        return tr( "Put Bid/Ask Size" );
    case PUT_BID_PRICE:
        return tr( "Put Bid Price" );
    case PUT_BID_SIZE:
        return tr( "Put Bid Size" );
    case PUT_ASK_PRICE:
        return tr( "Put Ask Price" );
    case PUT_ASK_SIZE:
        return tr( "Put Ask Size" );
    case PUT_LAST_PRICE:
        return tr( "Put Last Price" );
    case PUT_LAST_SIZE:
        return tr( "Put Last Size" );
    case PUT_BREAK_EVEN_PRICE:
        return tr( "Put Break Even Price" );
    case PUT_INTRINSIC_VALUE:
        return tr( "Put Intrinsic Value" );
    case PUT_OPEN_PRICE:
        return tr( "Put Open Price" );
    case PUT_HIGH_PRICE:
        return tr( "Put High Price" );
    case PUT_LOW_PRICE:
        return tr( "Put Low Price" );
    case PUT_CLOSE_PRICE:
        return tr( "Put Close Price" );
    case PUT_CHANGE:
        return tr( "Put Change" );
    case PUT_PERCENT_CHANGE:
        return tr( "Put Percent Change" );
    case PUT_TOTAL_VOLUME:
        return tr( "Put Volume" );
    case PUT_QUOTE_TIME:
        return tr( "Put Quote Time" );
    case PUT_TRADE_TIME:
        return tr( "Put Trade Time" );
    case PUT_MARK:
        return tr( "Put Mark" );
    case PUT_MARK_CHANGE:
        return tr( "Put Mark Change" );
    case PUT_MARK_PERCENT_CHANGE:
        return tr( "Put Mark Percent Change" );
    case PUT_EXCHANGE_NAME:
        return tr( "Put Exchange" );
    case PUT_VOLATILITY:
        return tr( "Put Volatility" );
    case PUT_DELTA:
        return tr( "Put Delta" );
    case PUT_GAMMA:
        return tr( "Put Gamma" );
    case PUT_THETA:
        return tr( "Put Theta" );
    case PUT_VEGA:
        return tr( "Put Vega" );
    case PUT_RHO:
        return tr( "Put Rho" );
    case PUT_TIME_VALUE:
        return tr( "Put Time Value" );
    case PUT_OPEN_INTEREST:
        return tr( "Put Open Interest" );
    case PUT_IS_IN_THE_MONEY:
        return tr( "Put In The Money" );
    case PUT_THEO_OPTION_VALUE:
        return tr( "Put Theoretical Value" );
    case PUT_THEO_VOLATILITY:
        return tr( "Put Theoretical Volatility" );
    case PUT_IS_MINI:
        return tr( "Put Is Mini" );
    case PUT_IS_NON_STANDARD:
        return tr( "Put Is Non-Standard" );
    case PUT_IS_INDEX:
        return tr( "Put Is Index" );
    case PUT_IS_WEEKLY:
        return tr( "Put Is Weekly" );
    case PUT_IS_QUARTERLY:
        return tr( "Put Is Quarterly" );
    case PUT_EXPIRY_DATE:
        return tr( "Put Expiration Date" );
    case PUT_EXPIRY_TYPE:
        return tr( "Put Expiration Type" );
    case PUT_DAYS_TO_EXPIRY:
        return tr( "Put Days to Expiration" );
    case PUT_LAST_TRADING_DAY:
        return tr( "Put Last Trading Day" );
    case PUT_MULTIPLIER:
        return tr( "Put Multiplier" );
    case PUT_SETTLEMENT_TYPE:
        return tr( "Put Settlement Type" );
    case PUT_DELIVERABLE_NOTE:
        return tr( "Put Deliverable Note" );

    default:
        break;
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant OptionChainTableModel::data( const QModelIndex& index, int role ) const
{
    if ( Qt::DisplayRole == role )
    {
        switch ( index.column() )
        {
        case CALL_BID_PRICE:
        case CALL_ASK_PRICE:
        case PUT_BID_PRICE:
        case PUT_ASK_PRICE:

            // no bid/ask size
            if ( 0 == _Mybase::data( index.row(), bidAskSize_[index.column()], role ).toInt() )
                return QVariant();

            break;

        default:
            break;
        }

        return formatValue( _Mybase::data( index, role ), numDecimalPlaces_[index.column()] );
    }
    else if ( Qt::BackgroundRole == role )
    {
        if ( STRIKE_PRICE == index.column() )
            return QVariant( strikeColor_ );
        else if ( isColumnCallOption( (ColumnIndex) index.column() ) )
        {
            if ( tableData( index.row(), CALL_IS_IN_THE_MONEY ).toBool() )
                return QVariant( inTheMoneyColor_ );
        }
        else if ( isColumnPutOption( (ColumnIndex) index.column() ) )
        {
            if ( tableData( index.row(), PUT_IS_IN_THE_MONEY ).toBool() )
                return QVariant( inTheMoneyColor_ );
        }
    }
    else if ( Qt::ForegroundRole == role )
    {
        return QVariant( textColor_ );
    }
    else if ( Qt::TextAlignmentRole == role )
    {
        if ( STRIKE_PRICE == index.column() )
            return QVariant( Qt::AlignCenter );
        else if ( columnIsText_[index.column()] )
            return QVariant( Qt::AlignLeft | Qt::AlignVCenter );

        return QVariant( Qt::AlignRight | Qt::AlignVCenter );
    }

    return _Mybase::data( index, role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionChainTableModel::ColumnIndex OptionChainTableModel::mappedColumn( ColumnIndex col ) const
{
    static const int diff( _PUT_COLUMNS_BEGIN - _CALL_COLUMNS_BEGIN );

    if ( isColumnCallOption( col ) )
        return (ColumnIndex) (col + diff);
    else if ( isColumnPutOption( col ) )
        return (ColumnIndex) (col - diff);

    return col;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Qt::ItemFlags OptionChainTableModel::flags( const QModelIndex& index ) const
{
    // disable item
    Qt::ItemFlags f( _Mybase::flags( index ) );
    f &= ~Qt::ItemIsEnabled;

    return f;
}


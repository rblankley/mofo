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

    columnIsText_[CALL_SYMBOL] = columnIsText_[PUT_SYMBOL] = true;
    columnIsText_[CALL_DESC] = columnIsText_[PUT_DESC] = true;

    columnIsText_[CALL_EXCHANGE_NAME] = columnIsText_[PUT_EXCHANGE_NAME] = true;

    columnIsText_[CALL_SETTLEMENT_TYPE] = columnIsText_[PUT_SETTLEMENT_TYPE] = true;
    columnIsText_[CALL_DELIVERABLE_NOTE] = columnIsText_[PUT_DELIVERABLE_NOTE] = true;

    // number of decimal places
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
    numDecimalPlaces_[CALL_PERCENT_CHANGE] = numDecimalPlaces_[PUT_PERCENT_CHANGE] = 1;

    numDecimalPlaces_[CALL_MARK] = numDecimalPlaces_[PUT_MARK] = 2;
    numDecimalPlaces_[CALL_MARK_CHANGE] = numDecimalPlaces_[PUT_MARK_CHANGE] = 2;
    numDecimalPlaces_[CALL_MARK_PERCENT_CHANGE] = numDecimalPlaces_[PUT_MARK_PERCENT_CHANGE] = 1;

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
    // close database connection
    database().close();
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


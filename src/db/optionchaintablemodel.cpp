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

    // generate column is currency
    columnIsCurrency_[CALL_BID_PRICE] = true;
    columnIsCurrency_[CALL_ASK_PRICE] = true;
    columnIsCurrency_[CALL_LAST_PRICE] = true;

    columnIsCurrency_[CALL_BREAK_EVEN_PRICE] = true;
    columnIsCurrency_[CALL_INTRINSIC_VALUE] = true;

    columnIsCurrency_[CALL_OPEN_PRICE] = true;
    columnIsCurrency_[CALL_HIGH_PRICE] = true;
    columnIsCurrency_[CALL_LOW_PRICE] = true;
    columnIsCurrency_[CALL_CLOSE_PRICE] = true;

    columnIsCurrency_[CALL_CHANGE] = true;

    columnIsCurrency_[CALL_MARK] = true;
    columnIsCurrency_[CALL_MARK_CHANGE] = true;

    columnIsCurrency_[CALL_TIME_VALUE] = true;
    columnIsCurrency_[CALL_THEO_OPTION_VALUE] = true;

    columnIsCurrency_[STRIKE_PRICE] = true;

    columnIsCurrency_[PUT_BID_PRICE] = true;
    columnIsCurrency_[PUT_ASK_PRICE] = true;
    columnIsCurrency_[PUT_LAST_PRICE] = true;

    columnIsCurrency_[PUT_BREAK_EVEN_PRICE] = true;
    columnIsCurrency_[PUT_INTRINSIC_VALUE] = true;

    columnIsCurrency_[PUT_OPEN_PRICE] = true;
    columnIsCurrency_[PUT_HIGH_PRICE] = true;
    columnIsCurrency_[PUT_LOW_PRICE] = true;
    columnIsCurrency_[PUT_CLOSE_PRICE] = true;

    columnIsCurrency_[PUT_CHANGE] = true;
    columnIsCurrency_[PUT_PERCENT_CHANGE] = true;

    columnIsCurrency_[PUT_MARK] = true;
    columnIsCurrency_[PUT_MARK_CHANGE] = true;

    columnIsCurrency_[PUT_TIME_VALUE] = true;
    columnIsCurrency_[PUT_THEO_OPTION_VALUE] = true;

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
QVariant OptionChainTableModel::data( const QModelIndex& index, int role ) const
{
    if ( Qt::BackgroundRole == role )
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


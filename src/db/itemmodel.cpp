/**
 * @file itemmodel.cpp
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
#include "itemmodel.h"

#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QTime>

///////////////////////////////////////////////////////////////////////////////////////////////////
ItemModel::ItemModel( int rows, int columns, QObject *parent ) :
    _Mybase( rows, columns, parent ),
#if QT_VERSION < QT_VERSION_CHECK( 5, 14, 0 )
    m_( QMutex::Recursive ),
#endif
    columnIsText_( columns, false ),
    numDecimalPlaces_( columns, 0 )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ItemModel::~ItemModel()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant ItemModel::data( int row, int col, int role ) const
{
    QMutexLocker guard( &m_ );
    return _Mybase::data( createIndex( row, col ), role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant ItemModel::data0( int col, int role ) const
{
    QMutexLocker guard( &m_ );
    return _Mybase::data( createIndex( 0, col ), role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ItemModel::removeAllRows()
{
    QMutexLocker guard( &m_ );

    if ( !rowCount() )
        return;

    removeRows( 0, rowCount() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString ItemModel::formatValue( const QVariant& v, int numDecimalPlaces )
{
#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
    if ( QMetaType::QString == v.typeId() )
#else
    if ( QVariant::String == v.type() )
#endif
    {
        return v.toString();
    }

#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
    else if ( QMetaType::QDate == v.typeId() )
#else
    else if ( QVariant::Date == v.type() )
#endif
    {
        const QDate result( v.toDate() );

        return result.toString();
    }

#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
    else if ( QMetaType::QDateTime == v.typeId() )
#else
    else if ( QVariant::DateTime == v.type() )
#endif
    {
        const QDateTime result( v.toDateTime() );

        return result.toString();
    }

#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
    else if ( QMetaType::QTime == v.typeId() )
#else
    else if ( QVariant::Time == v.type() )
#endif
    {
        const QTime result( v.toTime() );

        return result.toString();
    }

    const double doubleValue( v.toDouble() );

    if ( numDecimalPlaces )
        return QString::number( doubleValue, 'f', numDecimalPlaces );

    // check for integer
    const qlonglong intValue( v.toLongLong() );

    if ( doubleValue == (double) intValue )
    {
        const QLocale l( QLocale::system() );
        return l.toString( intValue );
    }

    return QString::number( doubleValue );
}

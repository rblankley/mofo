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
#include <QStandardItem>

QMutex ItemModel::poolItemMutex_;
QList<ItemModel::item_type*> ItemModel::poolItems_;

///////////////////////////////////////////////////////////////////////////////////////////////////
ItemModel::ItemModel( int rows, int columns, QObject *parent ) :
    _Mybase( parent ),
    lock_( QReadWriteLock::Recursive ),
    numColumns_( columns ),
    columnIsText_( columns, false ),
    numDecimalPlaces_( columns, 0 ),
    sortRole_( Qt::DisplayRole )
{
    horzHeader_ = allocRowItems();

    // insert rows
    insertRows( 0, rows );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ItemModel::~ItemModel()
{
    removeAllRows();

    freeRowItems( horzHeader_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int ItemModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED( parent )

    return numColumns_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant ItemModel::dataImpl( int row, int col, int role ) const
{
    if (( 0 <= row ) && ( row < rows_.count() ))
    {
        const item_type *item( rows_[row] );

        if (( 0 <= col ) && ( col < columnCount() ))
            return item[col].data( role );
    }

    return QVariant();
}

QVariant ItemModel::data( int row, int col, int role ) const
{
    QReadLocker guard( &lock_ );

    return dataImpl( row, col, role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant ItemModel::data( const QModelIndex& index, int role ) const
{
    return data( index.row(), index.column(), role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant ItemModel::data0( int col, int role ) const
{
    return data( 0, col, role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Qt::ItemFlags ItemModel::flags( const QModelIndex& index ) const
{
    Q_UNUSED( index )

    return Qt::NoItemFlags;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant ItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    QReadLocker guard( &lock_ );

    if ( Qt::Horizontal == orientation )
        if (( 0 <= section ) && ( section < columnCount() ))
            return horzHeader_[section].data( role );

    return QVariant();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ItemModel::setData( int row, int col, const QVariant& value, int role )
{
    QWriteLocker guard( &lock_ );

    if (( 0 <= row ) && ( row < rows_.count() ))
    {
        item_type *items( rows_[row] );

        if (( 0 <= col ) && ( col < columnCount() ))
        {
            // set data
            items[col].setData( value, role );

            // emit
            const QModelIndex idx( createIndex( row, col ) );

            emit dataChanged( idx, idx );

            // success!
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ItemModel::setData( const QModelIndex& index, const QVariant &value, int role )
{
    return setData( index.row(), index.column(), value, role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ItemModel::setHeaderData( int section, Qt::Orientation orientation, const QVariant& value, int role )
{
    QWriteLocker guard( &lock_ );

    if ( Qt::Horizontal == orientation )
        if (( 0 <= section ) && ( section < columnCount() ))
        {
            // set data
            horzHeader_[section].setData( value, role );

            // emit
            emit headerDataChanged( orientation, section, section );

            // success!
            return true;
        }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int ItemModel::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED( parent )

    int result;

    // yuck!
    // when inserting rows the base class will invoke this method with a write lock already in
    // place... because of this we cannot claim a read lock (which doesn't make a lot of sense to
    // me but whatever)... so for workaround lets check for existing write lock first
    if ( lock_.tryLockForWrite() )
    {
        result = rows_.size();
        lock_.unlock();
    }
    else
    {
        QReadLocker guard( &lock_ );
        result = rows_.size();
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ItemModel::appendRow( item_type *items )
{
    if ( !items )
        return;

    QWriteLocker guard( &lock_ );

    const int row( rows_.count() );

    // append to list
    beginInsertRows( QModelIndex(), row, row );
    rows_.append( items );
    endInsertRows();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ItemModel::insertRows( int row, int count, const QModelIndex& parent )
{
    if ( count <= 0 )
        return true;

    QWriteLocker guard( &lock_ );

    // verify rows
    row = qMax( row, 0 );
    row = qMin( row, rows_.count() );

    beginInsertRows( parent, row, (row + count - 1) );

    // allocate some data
    while ( count-- )
        rows_.insert( row, allocRowItems() );

    endInsertRows();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ItemModel::removeAllRows()
{
    QWriteLocker guard( &lock_ );

    const int rows( rows_.count() );

    if ( !rows )
        return;

    LOG_DEBUG << "removing " << rows << " rows...";
    removeRows( 0, rows );

    LOG_DEBUG << "removal complete";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ItemModel::removeRows( int row, int count, const QModelIndex& parent )
{
    if ( count <= 0 )
        return true;

    QWriteLocker guard( &lock_ );

    if (( row < 0 ) || ( rows_.count() <= row ))
        return false;

    // verify count of rows to remove
    count = qMin( count, (rows_.count() - row) );

    beginRemoveRows( parent, row, (row + count - 1) );

    // free ALL row data
    if (( 0 == row ) && ( rows_.count() == count ))
    {
        freeRowItems( rows_ );

        rows_.clear();
    }

    // free row data
    else
    {
        while ( count-- )
        {
            const int i( row + count );

            freeRowItems( rows_[i] );

            rows_.removeAt( i );
        }
    }

    endRemoveRows();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int ItemModel::removeRowsIf( int column, const QVariant& value, RemovalRule rule )
{
    QList<int> doomed;

    QWriteLocker guard( &lock_ );

    // make list of rows to remove
    for ( int row( rows_.count() ); row--; )
    {
        const QVariant v( dataImpl( row, column, Qt::UserRole ) );

        bool addToList( false );

#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
        const QPartialOrdering result( QVariant::compare( v, value ) );

        if ( QPartialOrdering::Equivalent == result )
            addToList = (( RemovalRule::LessThanEqual == rule ) || ( RemovalRule::Equal == rule ) || ( RemovalRule::GreaterThanEqual == rule ));
        else if ( QPartialOrdering::Less == result )
            addToList = (( RemovalRule::LessThan == rule ) || ( RemovalRule::NotEqual == rule ));
        else if ( QPartialOrdering::Greater == result )
            addToList = (( RemovalRule::GreaterThan == rule ) || ( RemovalRule::NotEqual == rule ));
#else
        if ( RemovalRule::LessThan == rule )
            addToList = (v < value);
        else if ( RemovalRule::LessThanEqual == rule )
            addToList = (v <= value);
        else if ( RemovalRule::Equal == rule )
            addToList = (v == value);
        else if ( RemovalRule::GreaterThanEqual == rule )
            addToList = (value <= v);
        else if ( RemovalRule::GreaterThan == rule )
            addToList = (value < v);
        else if ( RemovalRule::NotEqual == rule )
            addToList = (v != value);
#endif

        if ( addToList )
            doomed.append( row );
    }

    // remove some rows
    LOG_DEBUG << "removing " << doomed.size() << " rows...";

    foreach ( int row, doomed )
        removeRows( row, 1 );

    LOG_DEBUG << "removal complete";

    return doomed.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Functor class for sorting by column.
class SortByColumnOrder
{
public:

    /// Constructor.
    SortByColumnOrder( int column, int role, Qt::SortOrder order ) : column_( column ), role_( role ), order_( order ) {}

    /// Sorting functor.
    bool operator()( const MapItem *a, const MapItem *b ) const {
#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
        const QPartialOrdering result( QVariant::compare( a[column_].data( role_ ), b[column_].data( role_ ) ) );
        if ( Qt::AscendingOrder == order_ )
            return (QPartialOrdering::Less == result);
        else if ( Qt::DescendingOrder == order_ )
            return (QPartialOrdering::Greater == result);
#else
        if ( Qt::AscendingOrder == order_ )
            return a[column_].data( role_ ) < b[column_].data( role_ );
        else if ( Qt::DescendingOrder == order_ )
            return a[column_].data( role_ ) > b[column_].data( role_ );
#endif
        return false;
    }

private:

    int column_;
    int role_;

    Qt::SortOrder order_;

};

void ItemModel::sort( int column, Qt::SortOrder order )
{
    if (( column < 0 ) || ( columnCount() <= column ))
        return;

    QWriteLocker guard( &lock_ );

    if ( !rows_.count() )
        return;

    LOG_DEBUG << "sorting by column " << column << " order " << order << "...";
    emit layoutAboutToBeChanged();

    // sort!!
    std::sort( rows_.begin(), rows_.end(), SortByColumnOrder( column, sortRole(), order ) );

    LOG_DEBUG << "sorting complete";
    emit layoutChanged();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ItemModel::item_type *ItemModel::allocRowItems() const
{
    {
        QMutexLocker guard( &poolItemMutex_ );

        if ( poolItems_.length() )
        {
            // pull item from front of list
            item_type *item( poolItems_.front() );
            poolItems_.pop_front();

            guard.unlock();

            // clear out existing data
            for ( int i( columnCount() ); i--; )
                item[i].clearData();

            return item;
        }
    }

    return new item_type[columnCount()];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ItemModel::freeRowItems( item_type *doomed ) const
{
    if ( !doomed )
        return;

    QMutexLocker guard( &poolItemMutex_ );

    poolItems_.push_back( doomed );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ItemModel::freeRowItems( const QList<item_type*>& doomed ) const
{
    QMutexLocker guard( &poolItemMutex_ );

    poolItems_.append( doomed );
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
    {
        // check for negative zero
        if (( 2 == numDecimalPlaces ) && ( doubleValue < 0.0 ))
        {
            if ( -0.005 <= doubleValue )
                return "0.00";
        }

        return QString::number( doubleValue, 'f', numDecimalPlaces );
    }

    // check for integer
    const qlonglong intValue( v.toLongLong() );

    if ( doubleValue == (double) intValue )
    {
        const QLocale l( QLocale::system() );
        return l.toString( intValue );
    }

    return QString::number( doubleValue );
}

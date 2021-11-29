/**
 * @file itemmodel.h
 * Item model base class.
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

#ifndef ITEMMODEL_H
#define ITEMMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QReadWriteLock>
#include <QVariant>
#include <QVector>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Mapped item.
/**
 * Map of variant data by role.
 */
class MapItem
{
    using _Myt = MapItem;

public:

    // ========================================================================
    // CTOR
    // ========================================================================

    /// Constructor.
    MapItem() {}

    // ========================================================================
    // Properties
    // ========================================================================

    /// Clear data.
    virtual void clearData() {data_.clear();}

    /// Retrieve data for role.
    /**
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data( int role ) const
    {
        RoleDataMap::const_iterator i( data_.find( role ) );

        if ( i != data_.constEnd() )
            return (*i);

        return QVariant();
    }

    /// Set data for role.
    /**
     * @param[in] value  data value
     * @param[in] role  role
     */
    virtual void setData( const QVariant& value, int role )
    {
        data_[role] = value;
    }

protected:

    /// Map of data by role.
    using RoleDataMap = QMap<int, QVariant>;

    RoleDataMap data_;                              ///< Map of role data.

private:

    // not implemented
    MapItem( const _Myt& ) = delete;

    // not implemented
    MapItem( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

/// Row based list item model.
class ItemModel : public QAbstractListModel
{
    Q_OBJECT

    using _Myt = ItemModel;
    using _Mybase = QAbstractListModel;

public:

    /// Item type.
    using item_type = MapItem;

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] rows  number of rows
     * @param[in] columns  number of columns
     * @param[in] parent  parent object
     */
    ItemModel( int rows, int columns, QObject *parent = nullptr );

    /// Destructor.
    virtual ~ItemModel();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve number of columns.
    /**
     * @param[in] parent  parent index
     * @return  number of columns
     */
    virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const override;

    /// Retrieve model data.
    /**
     * @param[in] row  row
     * @param[in] col  column
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data( int row, int col, int role = Qt::DisplayRole ) const;

    /// Retrieve model data.
    /**
     * @param[in] index  model index
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;

    /// Retrieve model data for single row.
    /**
     * @param[in] col  column
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data0( int col, int role = Qt::DisplayRole ) const;

    /// Retrieve flags.
    /**
     * @param[in] index  index
     * @return  flags
     */
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const override;

    /// Retrieve header data.
    /**
     * @param[in] section  header section
     * @param[in] orientation  header orientation
     * @param[in] role  data role
     * @return  data
     */
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    /// Retrieve number of rows.
    /**
     * @param[in] parent  parent index
     * @return  number of rows
     */
    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const override;

    /// Set model data.
    /**
     * @param[in] row  row
     * @param[in] col  column
     * @param[in] value  data
     * @param[in] role  data role
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool setData( int row, int col, const QVariant& value, int role = Qt::EditRole );

    /// Set model data.
    /**
     * @param[in] index  index
     * @param[in] value  data
     * @param[in] role  data role
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;

    /// Set header data.
    /**
     * @param[in] section  header section
     * @param[in] orientation  header orientation
     * @param[in] value  data
     * @param[in] role  data role
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool setHeaderData( int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole ) override;

    /// Set role for sorting.
    /**
     * @param[in] role  data role used for sorting
     */
    virtual void setSortRole( int role ) {sortRole_ = role;}

    /// Retrieve role for sorting.
    /**
     * @return  data role used for sorting
     */
    virtual int sortRole() const {return sortRole_;}

    // ========================================================================
    // Methods
    // ========================================================================

    /// Append row to model.
    /**
     * Model will assume ownership over @p items.
     * @param[in] items  pointer to array of items
     */
    virtual void appendRow( item_type *items );

    /// Insert model rows.
    /**
     * @param[in] row  insert location
     * @param[in] count  number of rows to insert
     * @param[in] parent  parent index
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool insertRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;

    /// Remove model rows.
    /**
     * @param[in] row  remove location
     * @param[in] count  number of rows to remove
     * @param[in] parent  parent index
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;

    /// Remove all rows from model.
    virtual void removeAllRows();

    /// Sort model by @p column in given @p order.
    /**
     * @param[in] column  column to sort by
     * @param[in] order  order of sort
     */
    virtual void sort( int column, Qt::SortOrder order ) override;

protected:

    mutable QReadWriteLock lock_;                   ///< Lock.

    int numColumns_;                                ///< Number of columns.

    QVector<bool> columnIsText_;                    ///< Column contains text data.
    QVector<int> numDecimalPlaces_;                 ///< Number of decimal places for this column that contains numeric data.

    QList<item_type*> rows_;                        ///< Row item data.

    item_type *horzHeader_;                         ///< Horizontal header data.

    int sortRole_;                                  ///< Sorting role.

    // ========================================================================
    // Methods
    // ========================================================================

    /// Allocate array of items for populating a row.
    /**
     * Allocates number of items equal to column count.
     * @return  pointer to array
     */
    virtual item_type *allocRowItems() const;

    /// Free array of items.
    /**
     * @param[in] doomed  items to free
     */
    virtual void freeRowItems( item_type *doomed ) const;

    /// Free array of items.
    /**
     * @param[in] doomed  items to free
     */
    virtual void freeRowItems( const QList<item_type*>& doomed ) const;

    // ========================================================================
    // Static Methods
    // ========================================================================

    /// Format value into string.
    /**
     * @param[in] value  value to format
     * @param[in] numDecimalPlaces  number of decimal places (for numeric values)
     * @return  formatted value
     */
    static QString formatValue( const QVariant& value, int numDecimalPlaces = 0 );

private:

    static QMutex poolItemMutex_;
    static QList<item_type*> poolItems_;

    // not implemented
    ItemModel( const _Myt& ) = delete;

    // not implemented
    ItemModel( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // ITEMMODEL_H

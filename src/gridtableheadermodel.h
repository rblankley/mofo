/**
 * @file gridtableheadermodel.h
 * Grid table header model.
 *
 * Based on code from https://github.com/eyllanesc/stackoverflow/tree/master/questions/46469720
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

#ifndef GRIDTABLEHEADERMODEL_H
#define GRIDTABLEHEADERMODEL_H

#include <QAbstractTableModel>

class TableHeaderItem;

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Grid table header model.
class GridTableHeaderModel : public QAbstractTableModel
{
    Q_OBJECT

    using _Myt = GridTableHeaderModel;
    using _Mybase = QAbstractTableModel;

public:

    /// Header roles.
    enum HeaderRole
    {
        ColumnSpanRole = Qt::UserRole + 1,          ///< Column span value.
        RowSpanRole,                                ///< Row span value.
    };

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] rows  rows
     * @param[in] columns  columns
     * @param[in,out] parent  parent
     */
    GridTableHeaderModel( int rows, int columns, QObject *parent = nullptr );

    /// Destructor.
    virtual ~GridTableHeaderModel();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve data.
    /**
     * @param[in] index  index
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data( const QModelIndex& index, int role ) const override;

    /// Retrieve column count.
    /**
     * @param[in] parent  parent index
     * @return  number of columns
     */
    virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const override;

    /// Retrieve item flags.
    /**
     * @param[in] index  index
     * @return  item flags
     */
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const override;

    /// Retrieve item index.
    /**
     * @param[in] row  row
     * @param[in] column  column
     * @param[in] parent  parent
     * @return  index
     */
    virtual QModelIndex index( int row, int column, const QModelIndex& parent = QModelIndex() ) const override;

    /// Retrieve row count.
    /**
     * @param[in] parent  parent
     * @return  number of rows
     */
    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const override;

    /// Set data.
    /**
     * @param[in] index  index
     * @param[in] value  data to set
     * @param[in] role  role
     * @return  @c true if set, @c false otherwise
     */
    virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;

private:

    using item_type = TableHeaderItem;

    item_type *rootItem_;

    int rows_;
    int columns_;

    // not implemented
    GridTableHeaderModel() = delete;

    // not implemented
    GridTableHeaderModel( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // GRIDTABLEHEADERMODEL_H

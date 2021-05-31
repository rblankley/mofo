/**
 * @file tableheaderitem.h
 * Table header item.
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

#ifndef TABLEHEADERITEM_H
#define TABLEHEADERITEM_H

#include <QHash>
#include <QPair>
#include <QVariant>

///////////////////////////////////////////////////////////////////////////////////////////////////

// Table header item.
class TableHeaderItem
{
    using _Myt = TableHeaderItem;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] row  row
     * @param[in] column  column
     * @param[in,out] parent  parent
     */
    TableHeaderItem( int row, int column, _Myt *parent = nullptr );

    /// Constructor.
    /**
     * @param[in,out] parent  parent
     */
    TableHeaderItem( _Myt *parent = nullptr );

    /// Destructor.
    virtual ~TableHeaderItem();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve child item.
    /**
     * @param[in] row  row
     * @param[in] col  column
     * @return  pointer to item
     */
    virtual _Myt *child( int row, int col );

    /// Retrieve column.
    /**
     * @return  column
     */
    virtual int column() const {return column_;}

    /// Retrieve item data.
    /**
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data( int role ) const;

    /// Retrieve parent item.
    /**
     * @return  pointer to item
     */
    virtual _Myt *parent() {return parentItem_;}

    /// Retrieve row.
    /**
     * @return  row
     */
    virtual int row() const {return row_;}

    /// Set item data.
    /**
     * @param[in] data  data
     * @param[in] role  role
     */
    virtual void setData( const QVariant &data, int role );

    // ========================================================================
    // Methods
    // ========================================================================

    /// Insert child item.
    /**
     * @param[in] row  row
     * @param[in] col  column
     * @return  pointer to item
     */
    virtual _Myt *insertChild( int row, int col );

    /// Clear all items.
    virtual void clear();

private:

    using RowColumnPair = QPair<int, int>;
    using ChildItemsHashTable = QHash<RowColumnPair, _Myt*>;

    using ItemDataHashTable = QHash<int, QVariant>;

    _Myt *parentItem_;

    int row_;
    int column_;

    ChildItemsHashTable childItems_;
    ItemDataHashTable itemData_;

    // not implemented
    TableHeaderItem( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TABLEHEADERITEM_H

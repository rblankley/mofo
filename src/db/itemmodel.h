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

#include <QMutex>
#include <QStandardItemModel>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Item model base class.
class ItemModel : public QStandardItemModel
{
    Q_OBJECT

    using _Myt = ItemModel;
    using _Mybase = QStandardItemModel;

public:

    /// Map of column values.
    using ColumnValueMap = QMap<int, QVariant>;

    // ========================================================================
    // Properties
    // ========================================================================

    /// Retrieve table data.
    /**
     * @param[in] row  row
     * @param[in] col  column
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data( int row, int col, int role = Qt::DisplayRole ) const;

    /// Retrieve table data.
    /**
     * @param[in] index  model index
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override {return _Mybase::data( index, role );}

    /// Retrieve table data for single row.
    /**
     * @param[in] col  column
     * @param[in] role  role
     * @return  data
     */
    virtual QVariant data0( int col, int role = Qt::DisplayRole ) const;

    // ========================================================================
    // Methods
    // ========================================================================

    /// Remove all rows from model.
    virtual void removeAllRows();

protected:

    mutable QMutex m_;                              ///< Mutex.

    QVector<bool> columnIsText_;                    ///< Column contains text data.
    QVector<int> numDecimalPlaces_;                 ///< Number of decimal places for this column that contains numeric data.

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

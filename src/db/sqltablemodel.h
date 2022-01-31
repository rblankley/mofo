/**
 * @file sqltablemodel.h
 * Table model base class.
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

#ifndef SQLTABLEMODEL_H
#define SQLTABLEMODEL_H

#include <QDateTime>
#include <QSqlTableModel>
#include <QString>

///////////////////////////////////////////////////////////////////////////////////////////////////

/// Table model base class.
class SqlTableModel : public QSqlTableModel
{
    Q_OBJECT
    Q_PROPERTY( bool ready READ ready RESET resetReady );

    using _Myt = SqlTableModel;
    using _Mybase = QSqlTableModel;

public:

    // ========================================================================
    // CTOR / DTOR
    // ========================================================================

    /// Constructor.
    /**
     * @param[in] columns  number of columns
     * @param[in] parent  parent object
     * @param[in] db  database
     */
    SqlTableModel( int columns, QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase() );

    /// Destructor.
    virtual ~SqlTableModel();

    // ========================================================================
    // Properties
    // ========================================================================

    /// Check if column is numeric (non-string).
    /**
     * @param[in] col  column
     * @return  @c true if numeric, @c false otherwise
     */
    virtual bool columnIsNumeric( int col ) const {return !columnIsText( col );}

    /// Check if column is string.
    /**
     * @param[in] col  column
     * @return  @c true if string, @c false otherwise
     */
    virtual bool columnIsText( int col ) const {return columnIsText_[col];}

    /// Retrieve column description.
    /**
     * @param[in] col  column
     * @return  description
     */
    virtual QString columnDescription( int col ) const = 0;

    /// Retrieve number of decimal places for a column.
    /**
     * @param[in] col  column
     * @return  number decimal places
     */
    virtual int columnNumDecimalPlaces( int col ) const {return numDecimalPlaces_[col];}

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

    /// Check if data is ready.
    /**
     * @return  @c true if data is ready, @c false otherwise
     */
    virtual bool ready() const {return ready_;}

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Refresh table data.
    /**
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool refreshTableData();

    /// Reset data ready flag.
    virtual void resetReady() {ready_ = false;}

protected:

    bool ready_;                                    ///< Data is ready.

    QVector<bool> columnIsText_;                    ///< Column contains text data.
    QVector<int> numDecimalPlaces_;                 ///< Number of decimal places for this column that contains numeric data.

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

    static constexpr int SELECT_TIMEOUT = 25;       // 25ms

    // not implemented
    SqlTableModel( const _Myt& ) = delete;

    // not implemented
    SqlTableModel( const _Myt&& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt& ) = delete;

    // not implemented
    _Myt &operator = ( const _Myt&& ) = delete;

};

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // SQLTABLEMODEL_H

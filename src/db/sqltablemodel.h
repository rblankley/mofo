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

    using _Myt = SqlTableModel;
    using _Mybase = QSqlTableModel;

public:

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

public slots:

    // ========================================================================
    // Methods
    // ========================================================================

    /// Refresh table data.
    /**
     * @return  @c true upon success, @c false otherwise
     */
    virtual bool refreshTableData();

protected:

    QVector<bool> columnIsCurrency_;

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

private:

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

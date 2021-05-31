/**
 * @file sqltablemodel.cpp
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
#include "sqltablemodel.h"

#include <QSqlError>

///////////////////////////////////////////////////////////////////////////////////////////////////
SqlTableModel::SqlTableModel( int columns, QObject *parent, QSqlDatabase db ) :
    _Mybase( parent, db ),
    columnIsCurrency_( columns, false )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SqlTableModel::~SqlTableModel()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant SqlTableModel::data( int row, int col, int role ) const
{
    return index( row, col ).data( role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant SqlTableModel::data0( int col, int role ) const
{
    return index( 0, col ).data( role );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SqlTableModel::refreshTableData()
{
    if ( !select() )
    {
        const QSqlError e( lastError() );

        LOG_WARN << "error during replace " << e.type() << " " << qPrintable( e.text() );
        return false;
    }

    return true;
}

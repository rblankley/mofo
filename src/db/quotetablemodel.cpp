/**
 * @file quotetablemodel.cpp
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
#include "quotetablemodel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
QuoteTableModel::QuoteTableModel( const QString& symbol, const QDateTime& stamp, QObject *parent ) :
    _Mybase( _NUM_COLUMNS, parent, AppDatabase::instance()->openDatabaseConnection( symbol ) ),
    symbol_( symbol )
{
    // setup filter
    QString filter;

    if ( stamp.isValid() )
        filter += "DATETIME('" + stamp.toString( Qt::ISODateWithMs ) + "')=DATETIME(stamp)";
    else
        filter += "stamp=(SELECT MAX(stamp) FROM quotes)";

    filter += " AND '" + symbol + "'=symbol";

    // setup view
    setTable( "quotes" );
    setFilter( filter );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QuoteTableModel::~QuoteTableModel()
{
}

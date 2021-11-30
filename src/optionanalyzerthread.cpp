/**
 * @file optionanalyzerthread.cpp
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
#include "optionanalyzerthread.h"
#include "optionprofitcalc.h"
#include "optionprofitcalcfilter.h"

#include "db/appdb.h"
#include "db/optionchaintablemodel.h"
#include "db/optiontradingitemmodel.h"
#include "db/quotetablemodel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionAnalyzerThread::OptionAnalyzerThread( const QString& symbol, const QDate& expiryDate, model_type *model, QObject *parent ) :
    _Mybase( parent ),
    analysis_( model ),
    symbol_( symbol ),
    expiryDate_( expiryDate )
{
    assert( symbol.length() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionAnalyzerThread::~OptionAnalyzerThread()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzerThread::OptionAnalyzerThread::run()
{
    const QDateTime now( AppDatabase::instance()->currentDateTime() );

    QStringList cnames;

    // create filter for analysis
    OptionProfitCalculatorFilter calcFilter;

    const QString f( filter() );

    // load filter
    if ( f.length() )
        calcFilter.restoreState( AppDatabase::instance()->filter( f ) );

    {
        // open connections to databases
        QSqlDatabase connApp( AppDatabase::instance()->openDatabaseConnection() );
        cnames.append( connApp.connectionName() );

        QSqlDatabase connSymbol( AppDatabase::instance()->openDatabaseConnection( symbol_ ) );
        cnames.append( connSymbol.connectionName() );

        // retrieve quote
        QuoteTableModel quote( symbol_ );
        quote.refreshTableData();

        LOG_DEBUG << "processing " << qPrintable( symbol_ ) << " " << qPrintable( expiryDate_.toString() ) << "...";

        // check filter for days to expiry
        if (( calcFilter.minDaysToExpiry() ) && ( now.date().daysTo( expiryDate_ ) < calcFilter.minDaysToExpiry() ))
            LOG_TRACE << "DTE below filter threshold";
        else if (( calcFilter.maxDaysToExpiry() ) && ( calcFilter.maxDaysToExpiry() < now.date().daysTo( expiryDate_ ) ))
            LOG_TRACE << "DTE above filter threshold";
        else
        {
            // retrieve chain data
            OptionChainTableModel chains( symbol_, expiryDate_ );
            chains.refreshTableData();

            // create a calculator
            OptionProfitCalculator *calc( OptionProfitCalculator::create( quote.tableData( QuoteTableModel::MARK ).toDouble(), &chains, analysis_ ) );

            if ( !calc )
                LOG_WARN << "no calculator";
            else
            {
                // setup calculator
                calc->setFilter( calcFilter );
                calc->setOptionTradeCost( AppDatabase::instance()->optionTradeCost() );

                // analyze
                calc->analyze( OptionTradingItemModel::SINGLE );
                calc->analyze( OptionTradingItemModel::VERT_BEAR_CALL );
                calc->analyze( OptionTradingItemModel::VERT_BULL_PUT );

                OptionProfitCalculator::destroy( calc );
            }
        }
    }

    // remove databases
    foreach ( const QString& cname, cnames )
        QSqlDatabase::removeDatabase( cname );

    LOG_DEBUG << "processing complete";
}

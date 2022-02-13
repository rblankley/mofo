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
#include "db/fundamentalstablemodel.h"
#include "db/optionchaintablemodel.h"
#include "db/optiontradingitemmodel.h"
#include "db/quotetablemodel.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionAnalyzerThread::OptionAnalyzerThread( const QString& symbol, const QList<QDate>& expiryDates, model_type *model, QObject *parent ) :
    _Mybase( parent ),
    analysis_( model ),
    symbol_( symbol ),
    expiryDates_( expiryDates )
{
    assert( symbol.length() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionAnalyzerThread::~OptionAnalyzerThread()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionAnalyzerThread::run()
{
    // create filter for analysis
    OptionProfitCalculatorFilter calcFilter;

    const QString f( filter() );

    // load filter
    if ( f.length() )
        calcFilter.restoreState( AppDatabase::instance()->filter( f ) );

    // retrieve quote and fundamentals
    QuoteTableModel quote( symbol_ );
    FundamentalsTableModel fundamentals( symbol_ );

    if ( !quote.refreshData() )
        LOG_WARN << "error refreshing quote table data";
    else if ( !fundamentals.refreshData() )
        LOG_WARN << "error refreshing fundamentals table data";

    // check filter
    else if ( !calcFilter.check( &quote, &fundamentals ) )
        LOG_TRACE << "filtered out from underlying";

    else
    {
        foreach ( const QDate& d, expiryDates_ )
        {
            LOG_DEBUG << "processing " << qPrintable( symbol_ ) << " " << qPrintable( d.toString() ) << "...";

            // retrieve chain data
            OptionChainTableModel chains( symbol_, d );

            if ( !chains.refreshData() )
                LOG_WARN << "error refreshing chain table data";
            else
            {
                // create a calculator
                OptionProfitCalculator *calc( OptionProfitCalculator::create( quote.tableData( QuoteTableModel::MARK ).toDouble(), &chains, analysis_ ) );

                // no calculator
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
    }

    // remove app database connection
    AppDatabase::instance()->removeConnection();

    LOG_DEBUG << "processing complete";
}

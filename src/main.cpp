/**
 * @file main.cpp
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
#include "mainwindow.h"
#include "networkaccess.h"
#include "tddaemon.h"

#include "./db/appdb.h"
#include "./tda/dbadaptertd.h"
#include "./tda/tdapi.h"
#include "./usdot/usdotapi.h"
#include "./usdot/dbadapterusdot.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QPalette>

#if defined( Q_OS_WIN )
#include <Windows.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
void setStyle( QApplication& a, const QString& theme, const QColor& highlight )
{
    static const QColor blue( 42, 130, 218 );

    // use default color when passed in color is not valid
    QColor c( highlight );

    if ( !c.isValid() )
        c = blue;

    // use fusion style
    a.setStyle( "Fusion" );

    // use palette to switch to chosen theme
    if ( "DARK" == theme )
    {
        static const QColor darkGray( 53, 53, 53 );
        static const QColor gray( 128, 128, 128 );
        static const QColor black( 25, 25, 25 );

        QPalette palette;
        palette.setColor( QPalette::Window, darkGray );
        palette.setColor( QPalette::WindowText, Qt::white );
        palette.setColor( QPalette::Base, black );
        palette.setColor( QPalette::AlternateBase, darkGray );
        palette.setColor( QPalette::ToolTipBase, c );
        palette.setColor( QPalette::ToolTipText, Qt::white );
        palette.setColor( QPalette::Text, Qt::white );
        palette.setColor( QPalette::Button, darkGray );
        palette.setColor( QPalette::ButtonText, Qt::white );
        palette.setColor( QPalette::Link, c );
        palette.setColor( QPalette::Highlight, c );
        palette.setColor( QPalette::HighlightedText, Qt::black );

        palette.setColor( QPalette::Active, QPalette::Button, gray.darker() );
        palette.setColor( QPalette::Disabled, QPalette::ButtonText, gray );
        palette.setColor( QPalette::Disabled, QPalette::WindowText, gray );
        palette.setColor( QPalette::Disabled, QPalette::Text, gray );
        palette.setColor( QPalette::Disabled, QPalette::Light, darkGray );

        a.setPalette( palette );
    }
    else if ( "LIGHT" == theme )
    {
        static const QColor lightGray( 202, 202, 202 );
        static const QColor gray( 127, 127, 127 );
        static const QColor white( 230, 230, 230 );

        QPalette palette;
        palette.setColor( QPalette::Window, lightGray );
        palette.setColor( QPalette::WindowText, Qt::black );
        palette.setColor( QPalette::Base, white );
        palette.setColor( QPalette::AlternateBase, lightGray );
        palette.setColor( QPalette::ToolTipBase, c );
        palette.setColor( QPalette::ToolTipText, Qt::black );
        palette.setColor( QPalette::Text, Qt::black );
        palette.setColor( QPalette::Button, lightGray );
        palette.setColor( QPalette::ButtonText, Qt::black );
        palette.setColor( QPalette::Link, c );
        palette.setColor( QPalette::Highlight, c );
        palette.setColor( QPalette::HighlightedText, Qt::white );

        palette.setColor( QPalette::Active, QPalette::Button, gray.lighter() );
        palette.setColor( QPalette::Disabled, QPalette::ButtonText, gray );
        palette.setColor( QPalette::Disabled, QPalette::WindowText, gray );
        palette.setColor( QPalette::Disabled, QPalette::Text, gray );
        palette.setColor( QPalette::Disabled, QPalette::Light, lightGray );

        a.setPalette( palette );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[] )
{
#if defined( HAVE_CLIO_H )
    CLIO_INIT_WITH_INTERVAL( SYS_CONF_DIR "logging.config", 30 * 1000 );
#endif

    QApplication::setAttribute( Qt::AA_DisableWindowContextHelpButton );
    QApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
    QApplication a( argc, argv );

#if defined( Q_OS_WIN )
    // prevent computer from idle sleep mode
    SetThreadExecutionState( ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED );
#endif

#if defined( Q_OS_LINUX )
    static const QString config( USER_CONF_DIR );
    static const QString cache( USER_CACHE_DIR );

    QDir d;

    // create config directory
    if (( !d.exists( config ) ) && ( !d.mkdir( config ) ))
    {
        LOG_ERROR << "failed to make config dir " << qPrintable( config );
        return -1;
    }

    // create cache directory
    if (( !d.exists( cache ) ) && ( !d.mkdir( cache ) ))
    {
        LOG_ERROR << "failed to make cache dir " << qPrintable( cache );
        return -1;
    }
#endif

    // workaround for Qt 5.15.2 bug
    QDateTime start( QDateTime::currentDateTime() );
    start.toString( Qt::ISODateWithMs );

    // init database
    AppDatabase *db( AppDatabase::instance() );

    if ( !db->isReady() )
    {
        LOG_ERROR << "db not ready!";
        return -1;
    }

    // set app sytle
    setStyle( a, db->palette(), db->paletteHighlight() );

    // ---- //

    NetworkAccess *net( new NetworkAccess );

    // setup us dept of the treas api
    DeptOfTheTreasury *usdot( new DeptOfTheTreasury );
    usdot->setNetworkAccessManager( net );

    DeptOfTheTreasuryDatabaseAdapter *usdotadapter( new DeptOfTheTreasuryDatabaseAdapter );
    QObject::connect( usdot, &DeptOfTheTreasury::dailyTreasuryBillRatesReceived, usdotadapter, &DeptOfTheTreasuryDatabaseAdapter::transformDailyTreasuryBillRates );
    QObject::connect( usdot, &DeptOfTheTreasury::dailyTreasuryYieldCurveRatesReceived, usdotadapter, &DeptOfTheTreasuryDatabaseAdapter::transformDailyTreasuryYieldCurveRates );

    QObject::connect( usdotadapter, &DeptOfTheTreasuryDatabaseAdapter::transformComplete, db, &AppDatabase::processData );

    // setup td ameritrade api
    TDAmeritrade *tda( new TDAmeritrade );
    tda->setNetworkAccessManager( net );

    TDAmeritradeDatabaseAdapter *tdaadapter( new TDAmeritradeDatabaseAdapter );
    QObject::connect( tda, &TDAmeritrade::optionChainReceived, tdaadapter, &TDAmeritradeDatabaseAdapter::transformOptionChain );
    QObject::connect( tda, &TDAmeritrade::quotesReceived, tdaadapter, &TDAmeritradeDatabaseAdapter::transformQuotes );
    QObject::connect( tda, &TDAmeritrade::priceHistoryReceived, tdaadapter, &TDAmeritradeDatabaseAdapter::transformPriceHistory );
    QObject::connect( tda, &TDAmeritrade::marketHoursReceived, tdaadapter, &TDAmeritradeDatabaseAdapter::transformMarketHours );
    QObject::connect( tda, &TDAmeritrade::accountsReceived, tdaadapter, &TDAmeritradeDatabaseAdapter::transformAccounts );

    QObject::connect( tdaadapter, &TDAmeritradeDatabaseAdapter::transformComplete, db, &AppDatabase::processData );

    // setup daemon
    [[maybe_unused]] TDAmeritradeDaemon *daemon( new TDAmeritradeDaemon( tda, usdot ) );

    // ---- //

    // create window
    MainWindow w;
    w.showMaximized();

    return a.exec();
}

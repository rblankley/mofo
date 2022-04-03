/**
 * @file mainwindow.cpp
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

#include "accountsdialog.h"
#include "analysiswidget.h"
#include "common.h"
#include "configdialog.h"
#include "filtersdialog.h"
#include "filterselectiondialog.h"
#include "mainwindow.h"
#include "optionanalyzer.h"
#include "optionviewertabwidget.h"
#include "watchlistdialog.h"
#include "widgetstatesdialog.h"

#include "db/appdb.h"
#include "db/optiontradingitemmodel.h"

#include "util/tests.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>

static const QString applicationName( "Money 4 Options" );
static const QString applicationVersion( "0.1.1" );

static const QString EQUITY_OPTION_PRODUCT( "EQO" );
static const QString INDEX_OPTION_PRODUCT( "IND" );

static const QString MOFO_SOURCES( "https://github.com/rblankley/mofo" );
static const QString MOFO_PAYPAL_DONATION( "https://www.paypal.com/donate/?business=YW7LNTG6J452G&no_recurring=0&item_name=Thank+you+for+your+donation%21&currency_code=USD" );

///////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow( QWidget *parent ) :
    _Mybase( parent ),
    daemon_( AbstractDaemon::instance() ),
    db_( AppDatabase::instance() ),
    analysis_( nullptr ),
    analysisModel_( new OptionTradingItemModel( this ) )
{
    // init
    initialize();
    createLayout();
    translate();

    // update states
    updateMarketHours();
    updateMenuState();
    updateTransmitState( false );

    onConnectedStateChanged( daemon_->connectedState() );

    // create timer for updating market hours
    marketHoursTimer_ = new QTimer( this );
    marketHoursTimer_->setInterval( 15 * 1000 );
    marketHoursTimer_->setSingleShot( false );
    marketHoursTimer_->start();

    connect( marketHoursTimer_, &QTimer::timeout, this, &_Myt::updateMarketHours );

    // connect signals/slots
    connect( daemon_, &AbstractDaemon::activeChanged, this, &_Myt::updateMenuState );
    connect( daemon_, &AbstractDaemon::connectedStateChanged, this, &_Myt::onConnectedStateChanged );
    connect( daemon_, &AbstractDaemon::pausedChanged, this, &_Myt::updateMenuState );
    connect( daemon_, &AbstractDaemon::requestsPendingChanged, this, &_Myt::onRequestsPendingChanged );

    connect( daemon_, &AbstractDaemon::statusMessageChanged, statusBar_, &QStatusBar::showMessage );

    connect( db_, &AppDatabase::accountsChanged, this, &_Myt::onAccountsChanged, Qt::QueuedConnection );

    connect( analysis_, &OptionAnalyzer::activeChanged, this, &_Myt::updateMenuState );
    connect( analysis_, &OptionAnalyzer::complete, this, &_Myt::updateMenuState );
    connect( analysis_, &OptionAnalyzer::statusMessageChanged, statusBar_, &QStatusBar::showMessage );

    connect( QApplication::instance(), &QApplication::aboutToQuit, this, &_Myt::onAboutToQuit );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::~MainWindow()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize MainWindow::sizeHint() const
{
    return QSize( 1280, 720 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::translate()
{
    setWindowTitle( QString( "%1 (mofo v%2)" ).arg( applicationName, applicationVersion ) );

    fileMenu_->setTitle( tr( "&File" ) );
    exit_->setText( tr( "E&xit" ) );

    viewMenu_->setTitle( tr( "&View" ) );
    accountNames_->setText( tr( "&Accounts..." ) );
    config_->setText( tr( "&Configuration..." ) );
    filters_->setText( tr( "&Filters..." ) );
    layouts_->setText( tr( "&Layouts..." ) );
    watchlists_->setText( tr( "&Watchlists..." ) );

    marketDaemonMenu_->setTitle( daemon_->name() );
    authenticate_->setText( tr( "&Authenticate (Login)" ) );
    credentials_->setText( tr( "Cre&dentials..." ) );
    refreshAccountData_->setText( tr( "&Refresh Account" ) );
    singleOptionChain_->setText( tr( "View &Option Chain..." ) );
    startDaemon_->setText( tr( "&Start Daemon" ) );
    stopDaemon_->setText( tr( "St&op Daemon" ) );
    pauseDaemon_->setText( tr( "&Pause Daemon" ) );
    runWhenMarketsClosed_->setText( tr( "Allow When Markets &Closed" ) );

    results_->setTitle( tr( "&Analysis" ) );
    viewAnalysis_->setText( tr( "&View Results" ) );
    customScan_->setText( tr( "&Custom Scan..." ) );

    helpMenu_->setTitle( tr( "&Help" ) );
    about_->setText( tr( "&About" ) );
    validate_->setText( tr( "&Validate" ) );
    testPerf_->setText( tr( "Test &Performance" ) );
    testGreeks_->setText( tr( "Test &Option Pricing Methods" ) );

    accountsLabel_->setText( tr( "Account:" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::updateMarketHours()
{
    const QStringList marketTypes( db_->marketTypes() );
    const QDateTime now( db_->currentDateTime() );

    bool haveMarketHours( true );

    // update each market
    foreach ( const QString& market, marketTypes )
    {
        QLabel *m( marketHours_[market] );

        if ( !m )
            continue;

        // update font
        QFont f( m->font() );
        f.setBold( true );

        m->setFont( f );

        // update palette
        QPalette p( m->palette() );

        if ( !db_->marketHoursExist( now.date(), market ) )
        {
            // hours don't exist, yet...
            p.setColor( m->foregroundRole(), Qt::white );
            p.setColor( m->backgroundRole(), Qt::red );

            haveMarketHours = false;
        }
        else
        {
            bool ext;

            // check open
            if ( db_->isMarketOpen( now, market, QString(), &ext ) )
            {
                p.setColor( m->foregroundRole(), ext ? Qt::black :Qt::white );
                p.setColor( m->backgroundRole(), ext ? Qt::yellow : Qt::darkGreen );
            }
            else
            {
                p.setColor( m->foregroundRole(), Qt::darkGray );
                p.setColor( m->backgroundRole(), Qt::transparent );
            }
        }

        m->setPalette( p );

    } // for each market

    // check update tool tips
    if ( !haveMarketHours )
        return;
    else if (( marketHoursStamp_.isValid() ) && ( marketHoursStamp_.date() == now.date() ))
        return;

    // update each tool tip
    foreach ( const QString& market, marketTypes )
    {
        QLabel *m( marketHours_[market] );

        if ( !m )
            continue;

        const bool open( db_->isMarketOpen( now, market ) );

        // fetch market hours
        QMap<QString, MarketProductHours> hours;
        QDate d( now.date() );

        if ( open )
            hours = db_->marketHours( now.date(), market );
        else
        {
            // find next open date
            for ( int i( 1 ); i <= 10; ++i )
            {
                hours = db_->marketHours( (d = now.date().addDays( i )), market );

                if ( hours.size() )
                    break;
            }
        }

        // update tool tip
        QString toolTip;

        if ( !open )
        {
            toolTip += tr( "MARKET CLOSED" ) + "\n";
            toolTip += "\n";
        }

        if ( hours.size() )
        {
            toolTip += tr( "Hours for" ) + " " + d.toString() + "\n";
            toolTip += "\n";

            // combine like hours
            using CombinedMarketProductHours = QPair<MarketProductHours, QString>;

            QList<CombinedMarketProductHours> hoursCombined;

            for ( QMap<QString, MarketProductHours>::const_iterator j( hours.constBegin() ); j != hours.constEnd(); ++j )
                if (( j->regularMarketStart.isValid() ) && ( j->regularMarketEnd.isValid() ))
                {
                    bool found( false );

                    for ( QList<CombinedMarketProductHours>::iterator i( hoursCombined.begin() ); i != hoursCombined.end(); ++i )
                        if ( i->first == j.value() )
                        {
                            i->second += ", " + j.key();

                            found = true;
                            break;
                        }

                    if ( !found )
                        hoursCombined.append( CombinedMarketProductHours( j.value(), j.key() ) );
                }

            // update tooltip
            for ( QList<CombinedMarketProductHours>::const_iterator i( hoursCombined.constBegin() ); i != hoursCombined.constEnd(); ++i )
            {
                const QString marketName( i->second );
                const MarketProductHours marketHours( i->first );

                const bool preMarketValid(( marketHours.preMarketStart.isValid() ) && ( marketHours.preMarketEnd.isValid() ));
                const bool postMarketValid(( marketHours.postMarketStart.isValid() ) && ( marketHours.postMarketEnd.isValid() ));

                QString indent;

                if ( 1 < hoursCombined.size() )
                {
                    indent = "    ";

                    if ( EQUITY_OPTION_PRODUCT == marketName )
                        toolTip += tr( "Equity Options" ) + "\n";
                    else if ( INDEX_OPTION_PRODUCT == marketName )
                        toolTip += tr( "Index Options" ) + "\n";
                    else
                        toolTip += marketName + "\n";
                }

                // pre or post market hours exist
                if (( preMarketValid ) || ( postMarketValid ))
                {
                    if ( preMarketValid )
                        toolTip += indent + tr( "Pre:" ) + " " + marketHours.preMarketStart.time().toString() + " - " + marketHours.preMarketEnd.time().toString() + "\n";

                    toolTip += indent + tr( "Regular:" ) + " " + marketHours.regularMarketStart.time().toString() + " - " + marketHours.regularMarketEnd.time().toString() + "\n";

                    if ( postMarketValid )
                        toolTip += indent + tr( "Post:" ) + " " + marketHours.postMarketStart.time().toString() + " - " + marketHours.postMarketEnd.time().toString() + "\n";
                }
                else
                {
                    toolTip += indent + marketHours.regularMarketStart.time().toString() + " - " + marketHours.regularMarketEnd.time().toString();
                }

                toolTip += "\n";
            }

        } // hours exist

        m->setToolTip( toolTip );

    } // for each market

    marketHoursStamp_ = now;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::updateMenuState()
{
    const bool online( AbstractDaemon::Online == daemon_->connectedState() );
    const bool offline( AbstractDaemon::Offline == daemon_->connectedState() );
    const bool active( daemon_->isActive() );

    const bool accountsExist( accounts_->count() );

    authenticate_->setEnabled( offline );
    credentials_->setEnabled( offline && daemon_->canEditCredentials() );

    refreshAccountData_->setEnabled( online && accountsExist );
    singleOptionChain_->setEnabled( online );

    startDaemon_->setEnabled( online && !active );
    stopDaemon_->setEnabled( online && active );
    pauseDaemon_->setEnabled( online && active );

    accountsLabel_->setEnabled( accountsExist );
    accounts_->setEnabled( accountsExist );

    customScan_->setEnabled( online && active && !analysis_->isActive() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::onAboutToQuit()
{
    // stop background daemon
    if ( !daemon_->isActive() )
        return;

    statusBar_->showMessage( tr( "Shutting Daemon Down..." ) );

    daemon_->setActive( false );

    LOG_INFO << "waiting for analysis to complete...";
    analysis_->halt();

    LOG_DEBUG << "analysis complete";
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::onAccountsChanged()
{
    // not online, ignore for now
    if ( AbstractDaemon::Offline == daemon_->connectedState() )
        return;

    // parse accounts
    const QStringList accounts( db_->accounts() );

    foreach ( const QString& account, accounts )
    {
        const QStringList parts( account.split( ';' ) );

        if ( parts.size() < 4 )
            continue;

        const QString text( QString( "%1 (%2)" ).arg( parts[2], parts[1] ) );

        // combo already contains account
        int index;

        if ( 0 <= (index = accounts_->findData( parts[0] )) )
        {
            accounts_->setItemText( index, text );
            continue;
        }

        // add account to list
        accounts_->addItem( text, parts[0] );

        if ( "1" == parts[3] )
            accounts_->setCurrentIndex( accounts_->count() - 1 );
    }

    updateMenuState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::onActionTriggered()
{
    // account namess
    if ( accountNames_ == sender() )
    {
        LOG_TRACE << "accounts dialog...";

        AccountsDialog d( this );
        d.exec();
    }

    // config
    else if ( config_ == sender() )
    {
        LOG_TRACE << "config dialog...";

        ConfigurationDialog d( this );
        d.exec();
    }

    // filters
    else if ( filters_ == sender() )
    {
        LOG_TRACE << "filters dialog...";

        FiltersDialog d( this );
        d.exec();
    }

    // layouts
    else if ( layouts_ == sender() )
    {
        LOG_TRACE << "layouts dialog...";

        WidgetStatesDialog d( this );
        d.exec();
    }

    // watchlists
    else if ( watchlists_ == sender() )
    {
        LOG_TRACE << "watchlist dialog...";

        WatchlistDialog d( this );
        d.exec();
    }

    // authenticate
    else if ( authenticate_ == sender() )
    {
        LOG_TRACE << "authorize...";
        daemon_->authorize();
    }

    // credentials
    else if ( credentials_ == sender() )
    {
        if ( daemon_->canEditCredentials() )
        {
            LOG_TRACE << "credentials...";
            daemon_->editCredentials();
        }
    }

    // start daemon
    else if ( startDaemon_ == sender() )
    {
        LOG_TRACE << "setting active...";
        daemon_->setActive( true );
    }

    // stop daemon
    else if ( stopDaemon_ == sender() )
    {
        LOG_TRACE << "setting inactive...";
        daemon_->setActive( false );
    }

    // pause daemon
    else if ( pauseDaemon_ == sender() )
    {
        const bool newValue( !daemon_->isPaused() );

        LOG_TRACE << "set paused " << newValue;
        daemon_->setPaused( newValue );
    }

    // run when markets closed
    else if ( runWhenMarketsClosed_ == sender() )
    {
        const bool newValue( !daemon_->processOutsideMarketHours() );

        LOG_TRACE << "set process outside of market hours " << newValue;

        daemon_->setProcessOutsideMarketHours( newValue );
        runWhenMarketsClosed_->setChecked( newValue );
    }

    // refresh account
    else if ( refreshAccountData_ == sender() )
    {
        LOG_TRACE << "fetch accounts...";
        daemon_->getAccounts();
    }

    // analyze single option chain
    else if ( singleOptionChain_ == sender() )
    {
        bool okay;

        const QString str(
            QInputDialog::getText(
                this,
                tr( "Enter Symbol" ),
                tr( "Please enter option chain symbol:" ),
                QLineEdit::Normal,
                QString(),
                &okay ) );

        if (( okay ) && ( str.length() ))
        {
            const QString symbol( str.toUpper() ); // symbols should be upper case

            OptionViewerTabWidget *w( qobject_cast<OptionViewerTabWidget*>( centralWidget() ) );

            // set central widget
            if ( !w )
                setCentralWidget( w = new OptionViewerTabWidget( this ) );

            // create option chain
            w->createUnderlying( symbol );

            // retrieve option chain
            daemon_->getOptionChain( symbol );
        }
    }

    // option analysis results
    else if ( viewAnalysis_ == sender() )
    {
        AnalysisWidget *w( qobject_cast<AnalysisWidget*>( centralWidget() ) );

        // set central widget
        if ( !w )
            setCentralWidget( new AnalysisWidget( analysisModel_, this ) );
    }

    // custom scan and analysis
    else if ( customScan_ == sender() )
    {
        LOG_INFO << "custom scan...";

        FilterSelectionDialog d( this );
        d.setDefaultFilter( AppDatabase::instance()->optionAnalysisFilter() );
        d.setDefaultWatchLists( AppDatabase::instance()->optionAnalysisWatchLists() );
        d.setWatchListsVisible( true );

        if ( QDialog::Accepted != d.exec() )
            return;

        analysis_->setCustomFilter( d.selected() );

        daemon_->scan( d.watchLists() );
    }

    // about app
    else if ( about_ == sender() )
    {
        const QString href( "<a href=\"%1\">%1</a>" );
        const QString href2( "<a href=\"%1\">%2</a>" );

        const QString t(
            tr( "Application Version: %1<br>"
                "Database Version: %2<br>"
                "<br>"
                "Built on %3 %4<br>"
                "<br>"
                "%5<br>"
                "<br>"
                "%6<br>"
                "<br>"
                "%7<br>"
                "%8<br>"
                "<br>"
                "%9<br>" ) );

        QMessageBox about( this );
        about.setWindowTitle( tr( "About" ) + " " + applicationName );

        about.setIconPixmap( QPixmap( ":/res/icon.png" ).scaledToWidth( 128, Qt::SmoothTransformation ) );

        about.setTextFormat( Qt::RichText );
        about.setText( t.arg(
                applicationVersion,
                db_->version(),
                __DATE__,
                __TIME__,
                tr( "Copyright (C) 2022 Randy Blankley. All rights reserved." ),
                tr( "The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE." ),
                tr( "Full source code for this application can be found here:" ),
                href.arg( MOFO_SOURCES ),
                tr( "Like what you see? Consider making a" ) + " " + href2.arg( MOFO_PAYPAL_DONATION, tr( "donation" ) ) + " " + tr( "to this project." ) ) );

        LOG_TRACE << "about dialog...";
        about.exec();
    }

#if defined( QT_DEBUG )
    // validate options pricing
    else if ( validate_ == sender() )
    {
        LOG_TRACE << "validation...";

        QApplication::setOverrideCursor( Qt::WaitCursor );
        validateOptionPricing();

        QApplication::restoreOverrideCursor();

        LOG_TRACE << "validation... complete";
    }

    // test performance
    else if ( testPerf_ == sender() )
    {
        LOG_TRACE << "test performance...";

        QApplication::setOverrideCursor( Qt::WaitCursor );
        optionPricingPerf( 512 );

        QApplication::restoreOverrideCursor();

        LOG_TRACE << "test performance... complete";
    }

    // test options pricing
    else if ( testGreeks_ == sender() )
    {
        LOG_TRACE << "test option pricing...";

        QApplication::setOverrideCursor( Qt::WaitCursor );
        calculatePartials();

        QApplication::restoreOverrideCursor();

        LOG_TRACE << "test option pricing... complete";
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::onConnectedStateChanged( AbstractDaemon::ConnectedState newState )
{
    const bool online( AbstractDaemon::Online == newState );

    LOG_INFO << "connection state changed " << newState;

    // update font
    QFont f( connectionState_->font() );
    f.setBold( true );

    connectionState_->setFont( f );

    // update palette
    QPalette p( connectionState_->palette() );
    p.setColor( connectionState_->foregroundRole(), Qt::white );

    if ( online )
    {
        p.setColor( connectionState_->backgroundRole(), Qt::darkGreen );
        connectionState_->setText( tr( "ONLINE" ) );

        // refresh accounts
        daemon_->getAccounts();
    }
    else if ( AbstractDaemon::Offline == newState )
    {
        p.setColor( connectionState_->backgroundRole(), Qt::red );
        connectionState_->setText( tr( "OFFLINE" ) );
    }
    else
    {
        p.setColor( connectionState_->foregroundRole(), Qt::black );
        p.setColor( connectionState_->backgroundRole(), Qt::yellow );
        connectionState_->setText( tr( "AUTH..." ) );
    }

    connectionState_->setPalette( p );

    // ---- //

    updateMenuState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::onRequestsPendingChanged( int pending )
{
    updateTransmitState( pending );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::initialize()
{
    // icons from:
    // https://www.flaticon.com/packs/ecommerce-33
    // https://www.flaticon.com/packs/music-225
    // https://www.flaticon.com/packs/social-network-14
    // https://www.flaticon.com/packs/web-essentials-8

    setWindowIcon( QIcon( ":/res/icon.png" ) );

    // setup analyzer
    analysis_ = new OptionAnalyzer( analysisModel_, this );

    // file menu
    // ---------

    exit_ = new QAction( QIcon( ":/res/cancel.png" ), QString(), this );
    exit_->setShortcuts( QKeySequence::Quit );

    connect( exit_, &QAction::triggered, this, &_Myt::close );

    fileMenu_ = menuBar()->addMenu( QString() );
    fileMenu_->addAction( exit_ );

    // view menu
    // ------------

    accountNames_ = new QAction( QIcon( ":/res/accounts.png" ), QString(), this );

    config_ = new QAction( QIcon( ":/res/cogwheel.png" ), QString(), this );

    filters_ = new QAction( QIcon( ":/res/filter.png" ), QString(), this );

    layouts_ = new QAction( QIcon( ":/res/picture.png" ), QString(), this );

    watchlists_ = new QAction( QIcon( ":/res/list.png" ), QString(), this );

    connect( accountNames_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( config_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( filters_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( layouts_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( watchlists_, &QAction::triggered, this, &_Myt::onActionTriggered );

    viewMenu_ = menuBar()->addMenu( QString() );
    viewMenu_->addAction( accountNames_ );
    viewMenu_->addAction( config_ );
    viewMenu_->addAction( filters_ );
    viewMenu_->addAction( layouts_ );
    viewMenu_->addAction( watchlists_ );

    // market daemon menu
    // ------------------

    authenticate_ = new QAction( QIcon( ":/res/padlock.png" ), QString(), this );

    credentials_ = new QAction( QIcon( ":/res/key.png" ), QString(), this );
    credentials_->setVisible( daemon_->canEditCredentials() );

    refreshAccountData_ = new QAction( QIcon( ":/res/refresh.png" ), QString(), this );

    singleOptionChain_ = new QAction( QIcon( ":/res/chains.png" ), QString(), this );

    startDaemon_ = new QAction( QIcon( ":/res/play-button.png" ), QString(), this );

    stopDaemon_ = new QAction( QIcon( ":/res/stop-button.png" ), QString(), this );

    pauseDaemon_ = new QAction( QIcon( ":/res/pause-button.png" ), QString(), this );
    pauseDaemon_->setCheckable( true );

    runWhenMarketsClosed_ = new QAction( QIcon(), QString(), this );
    runWhenMarketsClosed_->setCheckable( true );

    connect( authenticate_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( credentials_, &QAction::triggered, this, &_Myt::onActionTriggered );

    connect( refreshAccountData_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( singleOptionChain_, &QAction::triggered, this, &_Myt::onActionTriggered );

    connect( startDaemon_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( stopDaemon_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( pauseDaemon_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( runWhenMarketsClosed_, &QAction::triggered, this, &_Myt::onActionTriggered );


    marketDaemonMenu_ = menuBar()->addMenu( QString() );
    marketDaemonMenu_->addAction( authenticate_ );
    marketDaemonMenu_->addAction( credentials_ );
    marketDaemonMenu_->addSeparator();
    marketDaemonMenu_->addAction( refreshAccountData_ );
    marketDaemonMenu_->addAction( singleOptionChain_ );
    marketDaemonMenu_->addSeparator();
    marketDaemonMenu_->addAction( startDaemon_ );
    marketDaemonMenu_->addAction( stopDaemon_ );
    marketDaemonMenu_->addAction( pauseDaemon_ );
    marketDaemonMenu_->addAction( runWhenMarketsClosed_ );

    // analysis menu
    // -------------

    viewAnalysis_ = new QAction( QIcon( ":/res/bar-chart.png" ), QString(), this );

    customScan_ = new QAction( QIcon( ":/res/loupe.png" ), QString(), this );
    customScan_->setEnabled( false );

    connect( viewAnalysis_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( customScan_, &QAction::triggered, this, &_Myt::onActionTriggered );

    results_ = menuBar()->addMenu( QString() );
    results_->addAction( viewAnalysis_ );
    results_->addAction( customScan_ );

    // help menu
    // ---------

    about_ = new QAction( QIcon( ":/res/information.png" ), QString(), this );

    validate_ = new QAction( QIcon(), QString(), this );
    testPerf_ = new QAction( QIcon(), QString(), this );
    testGreeks_ = new QAction( QIcon(), QString(), this );

    connect( about_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( validate_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( testPerf_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( testGreeks_, &QAction::triggered, this, &_Myt::onActionTriggered );

    helpMenu_ = menuBar()->addMenu( QString() );
    helpMenu_->addAction( about_ );

#if defined( QT_DEBUG )
    helpMenu_->addSeparator();
    helpMenu_->addAction( validate_ );
    helpMenu_->addAction( testPerf_ );
    helpMenu_->addAction( testGreeks_ );
#else
    helpMenu_->addAction( validate_ );
    helpMenu_->addAction( testPerf_ );
    helpMenu_->addAction( testGreeks_ );

    validate_->setVisible( false );
    testPerf_->setVisible( false );
    testGreeks_->setVisible( false );
#endif

    // status bar
    // ----------

    const QStringList marketTypes( db_->marketTypes() );

    // connection and transmit indicators
    QWidget *w0( new QWidget( this ) );

    connectionState_ = new QLabel( w0 );
    connectionState_->setAutoFillBackground( true );

    xmit_ = new QLabel( w0 );
    xmit_->setAutoFillBackground( true );

    QHBoxLayout *w0_layout( new QHBoxLayout( w0 ) );
    w0_layout->setContentsMargins( QMargins() );
    w0_layout->addWidget( connectionState_ );
    w0_layout->addWidget( xmit_ );

    // account
    QWidget *w1( new QWidget( this ) );

    accountsLabel_ = new QLabel( w1 );
    accountsLabel_->setEnabled( false );

    accounts_ = new QComboBox( w1 );
    accounts_->setEnabled( false );
    accounts_->setMinimumWidth( 150 );

    QHBoxLayout *w1_layout( new QHBoxLayout( w1 ) );
    w1_layout->setContentsMargins( QMargins() );
    w1_layout->addWidget( accountsLabel_ );
    w1_layout->addWidget( accounts_ );

    // markets
    // skip futures market as it takes forever to determine if any sub-market is open
    QWidget *w2( new QWidget( this ) );

    foreach ( const QString& market, marketTypes )
    {
        QLabel *m = new QLabel( market.toUpper(), w2 );
        m->setAutoFillBackground( true );

        marketHours_[market] = m;
    }

    QHBoxLayout *w2_layout( new QHBoxLayout( w2 ) );
    w2_layout->setContentsMargins( QMargins() );

    foreach ( const QString& market, marketTypes )
        if ( marketHours_.contains( market ) )
            w2_layout->addWidget( marketHours_[market] );

    // status bar
    statusBar_ = new QStatusBar( this );
    statusBar_->setSizeGripEnabled( false );
    statusBar_->addPermanentWidget( w2 );
    statusBar_->addPermanentWidget( w1 );
    statusBar_->addPermanentWidget( w0 );

    setStatusBar( statusBar_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::createLayout()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::updateTransmitState( int pending )
{
    // update font
    QFont f( xmit_->font() );
    f.setBold( true );

    xmit_->setFont( f );

    // update palette
    QPalette p( xmit_->palette() );
    p.setColor( xmit_->foregroundRole(), pending ? Qt::white : Qt::darkGray );
    p.setColor( xmit_->backgroundRole(), pending ? Qt::darkGreen : Qt::transparent );

    xmit_->setPalette( p );

    // update text
    QString text( tr( "XMIT" ) );

    if ( pending )
        text += QString( " [%0]" ).arg( pending );

    xmit_->setText( text );
}

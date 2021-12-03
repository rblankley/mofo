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

static const QString applicationName( "Money 4 Options" );
static const QString applicationVersion( "0.0.10" );

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
    updateMenuState();
    updateTransmitState( false );

    onConnectedStateChanged( daemon_->connectedState() );

    // connect signals/slots
    connect( daemon_, &AbstractDaemon::activeChanged, this, &_Myt::updateMenuState );
    connect( daemon_, &AbstractDaemon::connectedStateChanged, this, &_Myt::onConnectedStateChanged );
    connect( daemon_, &AbstractDaemon::pausedChanged, this, &_Myt::updateMenuState );
    connect( daemon_, &AbstractDaemon::requestsPendingChanged, this, &_Myt::onRequestsPendingChanged );

    connect( daemon_, &AbstractDaemon::statusMessageChanged, statusBar_, &QStatusBar::showMessage );

    connect( db_, &AppDatabase::accountsChanged, this, &_Myt::onAccountsChanged );

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
    setWindowTitle( QString( "%1 (mofo v%2)" ).arg( applicationName ).arg( applicationVersion ) );;

    fileMenu_->setTitle( tr( "&File" ) );
    exit_->setText( tr( "E&xit" ) );

    viewMenu_->setTitle( tr( "&View" ) );
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
    const QStringList accounts( db_->accounts() );

    foreach ( const QString& account, accounts )
    {
        // combo already contains account
        if ( 0 <= accounts_->findText( account ) )
            continue;

        const QStringList parts( account.split( " " ) );

        // add account to list
        accounts_->addItem( account, parts[0] );
    }

    updateMenuState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::onActionTriggered()
{
    // config
    if ( config_ == sender() )
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
            setCentralWidget( w = new AnalysisWidget( analysisModel_, this ) );
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
        LOG_TRACE << "about dialog...";

        QMessageBox::about(
            this,
            applicationName,
            tr( "Application Version: %1\n"
                "Database Version: %2\n"
                "\n"
                "Built on %3 %4\n"
                "\n"
                "%5\n"
                "\n"
                "%6" )
                .arg( applicationVersion )
                .arg( db_->version() )
                .arg( __DATE__ )
                .arg( __TIME__ )
                .arg( tr( "Copyright (C) 2021 Randy Blankley. All rights reserved." ) )
                .arg( tr( "The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE." ) ) );
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
        p.setColor( connectionState_->backgroundRole(), Qt::green );
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

    config_ = new QAction( QIcon( ":/res/cogwheel.png" ), QString(), this );

    filters_ = new QAction( QIcon( ":/res/filter.png" ), QString(), this );

    layouts_ = new QAction( QIcon( ":/res/picture.png" ), QString(), this );

    watchlists_ = new QAction( QIcon( ":/res/list.png" ), QString(), this );

    connect( config_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( filters_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( layouts_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( watchlists_, &QAction::triggered, this, &_Myt::onActionTriggered );

    viewMenu_ = menuBar()->addMenu( QString() );
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

    QWidget *w0( new QWidget( this ) );

    connectionState_ = new QLabel( w0 );
    connectionState_->setAutoFillBackground( true );

    xmit_ = new QLabel( w0 );
    xmit_->setAutoFillBackground( true );

    QHBoxLayout *w0_layout( new QHBoxLayout( w0 ) );
    w0_layout->setContentsMargins( QMargins() );
    w0_layout->addWidget( connectionState_ );
    w0_layout->addWidget( xmit_ );

    QWidget *w1( new QWidget( this ) );

    accountsLabel_ = new QLabel( w1 );

    accounts_ = new QComboBox( w1 );
    accounts_->setEnabled( false );
    accounts_->setMinimumWidth( 150 );

    QHBoxLayout *w1_layout( new QHBoxLayout( w1 ) );
    w1_layout->setContentsMargins( QMargins() );
    w1_layout->addWidget( accountsLabel_ );
    w1_layout->addWidget( accounts_ );

    statusBar_ = new QStatusBar( this );
    statusBar_->setSizeGripEnabled( false );
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
    p.setColor( xmit_->backgroundRole(), pending ? Qt::green : Qt::transparent );

    xmit_->setPalette( p );

    // update text
    QString text( tr( "XMIT" ) );

    if ( pending )
        text += QString( " [%0]" ).arg( pending );

    xmit_->setText( text );
}

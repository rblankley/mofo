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

#include "common.h"
#include "configdialog.h"
#include "mainwindow.h"
#include "optionviewertabwidget.h"
#include "watchlistdialog.h"

#include "db/appdb.h"

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
static const QString applicationVersion( "0.0.2" );

///////////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow( QWidget *parent ) :
    _Mybase( parent ),
    daemon_( AbstractDaemon::instance() ),
    db_( AppDatabase::instance() )
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
    watchlists_->setText( tr( "&Watchlists..." ) );

    marketDaemonMenu_->setTitle( daemon_->name() );
    authenticate_->setText( tr( "&Authenticate (Login)" ) );
    refreshAccountData_->setText( tr( "&Refresh Account" ) );
    singleOptionChain_->setText( tr( "View &Option Chain" ) );
    startDaemon_->setText( tr( "&Start Daemon" ) );
    stopDaemon_->setText( tr( "St&op Daemon" ) );
    pauseDaemon_->setText( tr( "&Pause Daemon" ) );
    runWhenMarketsClosed_->setText( tr( "Allow When Markets &Closed" ) );

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

    refreshAccountData_->setEnabled( online && accountsExist );
    singleOptionChain_->setEnabled( online );

    startDaemon_->setEnabled( online && !active );
    stopDaemon_->setEnabled( online && active );
    pauseDaemon_->setEnabled( online && active );

    accounts_->setEnabled( accountsExist );
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

        const QString symbol(
            QInputDialog::getText(
                this,
                tr( "Enter Symbol" ),
                tr( "Please enter option chain symbol:" ),
                QLineEdit::Normal,
                QString(),
                &okay ) );

        if (( okay ) && ( symbol.length() ))
        {
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
/*
    // option analysis results
    else if ( optionAnalysisResults_ == sender() )
    {
        OptionTradingView *w( qobject_cast<OptionTradingView*>( centralWidget() ) );

        // set central widget
        if ( !w )
            setCentralWidget( w = new OptionTradingView( optionAnalysis_->model(), this ) );
    }
*/
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
    setWindowIcon( QIcon( ":/res/icon.png" ) );

    // file menu
    // ---------

    exit_ = new QAction( style()->standardIcon( QStyle::SP_BrowserStop ), QString(), this );
    exit_->setShortcuts( QKeySequence::Quit );

    connect( exit_, &QAction::triggered, this, &_Myt::close );

    fileMenu_ = menuBar()->addMenu( QString() );
    fileMenu_->addAction( exit_ );

    // view menu
    // ------------

    config_ = new QAction( QString(), this );

    watchlists_ = new QAction( QString(), this );

    connect( config_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( watchlists_, &QAction::triggered, this, &_Myt::onActionTriggered );

    viewMenu_ = menuBar()->addMenu( QString() );
    viewMenu_->addAction( config_ );
    viewMenu_->addAction( watchlists_ );

    // market daemon menu
    // ------------------

    authenticate_ = new QAction( QIcon( ":/res/lock.png" ), QString(), this );

    refreshAccountData_ = new QAction( QIcon( ":/res/refresh.jpg" ), QString(), this );

    singleOptionChain_ = new QAction( QIcon(), QString(), this );

    startDaemon_ = new QAction( style()->standardIcon( QStyle::SP_MediaPlay ), QString(), this );

    stopDaemon_ = new QAction( style()->standardIcon( QStyle::SP_MediaStop ), QString(), this );

    pauseDaemon_ = new QAction( style()->standardIcon( QStyle::SP_MediaPause ), QString(), this );
    pauseDaemon_->setCheckable( true );

    runWhenMarketsClosed_ = new QAction( QIcon(), QString(), this );
    runWhenMarketsClosed_->setCheckable( true );

    connect( authenticate_, &QAction::triggered, this, &_Myt::onActionTriggered );

    connect( refreshAccountData_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( singleOptionChain_, &QAction::triggered, this, &_Myt::onActionTriggered );

    connect( startDaemon_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( stopDaemon_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( pauseDaemon_, &QAction::triggered, this, &_Myt::onActionTriggered );
    connect( runWhenMarketsClosed_, &QAction::triggered, this, &_Myt::onActionTriggered );


    marketDaemonMenu_ = menuBar()->addMenu( QString() );
    marketDaemonMenu_->addAction( authenticate_ );
    marketDaemonMenu_->addSeparator();
    marketDaemonMenu_->addAction( refreshAccountData_ );
    marketDaemonMenu_->addAction( singleOptionChain_ );
    marketDaemonMenu_->addSeparator();
    marketDaemonMenu_->addAction( startDaemon_ );
    marketDaemonMenu_->addAction( stopDaemon_ );
    marketDaemonMenu_->addAction( pauseDaemon_ );
    marketDaemonMenu_->addAction( runWhenMarketsClosed_ );
/*
    // trade bot menu
    // --------------

    autoTradeActive_ = new QAction( QIcon(), QString(), this );
    autoTradeActive_->setCheckable( true );

    tradeBotMenu_ = menuBar()->addMenu( QString() );
    tradeBotMenu_->addAction( autoTradeActive_ );
    tradeBotMenu_->setEnabled( false );
*/
    // help menu
    // ---------

    about_ = new QAction( style()->standardIcon( QStyle::SP_MessageBoxInformation ), QString(), this );

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

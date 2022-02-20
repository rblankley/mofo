/**
 * @file optionviewerwidget.cpp
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

#include "abstractdaemon.h"
#include "common.h"
#include "filterselectiondialog.h"
#include "optionchainview.h"
#include "optionprofitcalc.h"
#include "optiontradingview.h"
#include "optionviewerwidget.h"
#include "symbolpricehistorywidget.h"

#include "db/appdb.h"
#include "db/fundamentalstablemodel.h"
#include "db/optionchaintablemodel.h"
#include "db/optiontradingitemmodel.h"
#include "db/quotetablemodel.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QModelIndex>
#include <QSplitter>
#include <QSqlError>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

const QString OptionViewerWidget::STATE_GROUP_NAME( "optionViewer" );
const QString OptionViewerWidget::STATE_NAME( "[[default]]" );

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionViewerWidget::OptionViewerWidget( const QString& symbol, QWidget *parent ) :
    _Mybase( parent ),
    symbol_( symbol ),
    chartTab_( -1 )
{
    // setup model
    model_ = new QuoteTableModel( symbol, QDateTime(), this );

    connect( model_, &QuoteTableModel::dataChanged, this, &_Myt::refreshData );
    connect( model_, &QuoteTableModel::modelReset, this, &_Myt::refreshData );

    tradingModel_ = new OptionTradingItemModel( this );

    // init
    initialize();
    createLayout();
    translate();

    connect( AbstractDaemon::instance(), &AbstractDaemon::optionChainUpdated, this, &_Myt::onOptionChainUpdated );
    connect( AbstractDaemon::instance(), &AbstractDaemon::quotesUpdated, this, &_Myt::onQuotesUpdated );

    // restore
    restoreState( splitter_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionViewerWidget::~OptionViewerWidget()
{
    // save
    saveState( splitter_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::translate()
{
    lastLabel_->setText( tr( "Last" ) );
    lastChangeLabel_->setText( tr( "Change" ) );
    bidLabel_->setText( tr( "Bid" ) );
    askLabel_->setText( tr( "Ask" ) );
    sizeLabel_->setText( tr( "Size" ) );
    markLabel_->setText( tr( "Mark" ) );
    markChangeLabel_->setText( tr( "Mark Chng" ) );
    volumeLabel_->setText( tr( "Volume" ) );
    openLabel_->setText( tr( "Open" ) );
    closeLabel_->setText( tr( "Close" ) );
    dayRangeLabel_->setText( tr( "Day Range" ) );
    yearRangeLabel_->setText( tr( "52w Range" ) );
    divLabel_->setText( tr( "Dividend" ) );
    divDateLabel_->setText( tr( "Div Date" ) );

    clear_->setText( tr( "Clear" ) );
    analysisOne_->setText( tr( "Analyze\nOne Expiry" ) );
    analysisAll_->setText( tr( "Analyze\nAll Expirys" ) );
    refresh_->setText( tr( "Refresh" ) );

    expiryDates_->setTabText( chartTab_, tr( "Chart" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::onButtonClicked()
{
    // refresh
    if ( refresh_ == sender() )
    {
        // refresh underlying and option chains chains
        AbstractDaemon::instance()->getOptionChain( symbol_ );

        // refresh chart
        SymbolPriceHistoryWidget *priceHistory( nullptr );

        if (( 0 < chartTab_ ) && ( chartTab_ < expiryDates_->count() ))
            priceHistory = qobject_cast<SymbolPriceHistoryWidget*>( expiryDates_->widget( chartTab_ ) );

        if ( priceHistory )
            priceHistory->refreshData();
    }

    // clear
    else if ( clear_ == sender() )
    {
        // clear model
        tradingModel_->removeAllRows();

        // hide analysis results
        tradeAnalysis_->hide();

        clear_->setVisible( false );
    }

    // analysis of single expiry
    // analysis of all expirys
    else if (( analysisOne_ == sender() ) || ( analysisAll_ == sender() ))
    {
        OptionProfitCalculatorFilter calcFilter;

        // select which filter to use
        FilterSelectionDialog d( this );

        if ( d.filtersExist() )
        {
            if ( QDialog::Accepted != d.exec() )
                return;

            const QString f( d.selected() );

            // load selected filter
            if ( f.length() )
                calcFilter.restoreState( AppDatabase::instance()->filter( f ) );
        }

        // retrieve fundamentals
        FundamentalsTableModel fundamentals( symbol() );

        if ( !fundamentals.refreshData() )
        {
            LOG_WARN << "error refreshing fundamentals table data";
            return;
        }
        else if ( !calcFilter.check( model_, &fundamentals ) )
        {
            LOG_DEBUG << "filtered out from underlying";
            return;
        }

        // show analysis results
        tradeAnalysis_->show();

        // this could take a while...
        QApplication::setOverrideCursor( Qt::WaitCursor );

        for ( int i( 0 ); i < expiryDates_->count(); ++i )
        {
            // retrieve chains
            const OptionChainView *view( qobject_cast<const OptionChainView*>( expiryDates_->widget( i ) ) );

            OptionChainTableModel *viewModel( (view ? view->model() : nullptr) );

            if ( !viewModel )
                continue;

            if ( analysisOne_ == sender() )
                if ( expiryDates_->currentWidget() != view )
                    continue;

            // check DTE
            if (( calcFilter.minDaysToExpiry() ) && ( viewModel->daysToExpiration() < calcFilter.minDaysToExpiry() ))
                continue;
            else if (( calcFilter.maxDaysToExpiry() ) && ( calcFilter.maxDaysToExpiry() < viewModel->daysToExpiration() ))
                continue;

            // refresh stale data
            if ( !viewModel->ready() )
                if ( !viewModel->refreshData() )
                {
                    LOG_WARN << "error refreshing chain table data";
                    continue;
                }

            // create a calculator
            OptionProfitCalculator *calc( OptionProfitCalculator::create( model_->tableData( QuoteTableModel::MARK ).toDouble(), viewModel, tradingModel_ ) );

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

        // done
        QApplication::restoreOverrideCursor();

        clear_->setVisible( true );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::refreshData()
{
    if ( !model_->rowCount() )
        return;

    const QDateTime stamp( QDateTime::fromString( model_->tableData( QuoteTableModel::QUOTE_TIME ).toString(), Qt::ISODateWithMs ) );

    stamp_->setText( stamp.toString() );
    description_->setText( model_->tableData( QuoteTableModel::DESCRIPTION ).toString() );
    exchange_->setText( model_->tableData( QuoteTableModel::EXCHANGE_NAME ).toString() );

    last_->setText( model_->tableData( QuoteTableModel::LAST_PRICE ).toString() );

    lastChange_->setText(
        QString( "%0\n%1" )
            .arg( model_->tableData( QuoteTableModel::CHANGE ).toString() )
            .arg( model_->tableData( QuoteTableModel::PERCENT_CHANGE ).toString() + "%" ) );

    // update color of last change
    QPalette p( palette() );

    if ( model_->tableData( QuoteTableModel::CHANGE ).toDouble() < 0.0 )
        p.setColor( lastChange_->foregroundRole(), Qt::red );
    else if ( 0.0 < model_->tableData( QuoteTableModel::CHANGE ).toDouble() )
        p.setColor( lastChange_->foregroundRole(), Qt::darkGreen );

    last_->setPalette( p );
    lastChange_->setPalette( p );

    bid_->setText( model_->tableData( QuoteTableModel::BID_PRICE ).toString() );
    ask_->setText( model_->tableData( QuoteTableModel::ASK_PRICE ).toString() );
    size_->setText( model_->tableData( QuoteTableModel::BID_ASK_SIZE ).toString() );

    mark_->setText( model_->tableData( QuoteTableModel::MARK ).toString() );

    markChange_->setText(
        QString( "%0\n%1" )
            .arg( model_->tableData( QuoteTableModel::MARK_CHANGE ).toString() )
            .arg( model_->tableData( QuoteTableModel::MARK_PERCENT_CHANGE ).toString() + "%" ) );

    volume_->setText( model_->tableData( QuoteTableModel::TOTAL_VOLUME ).toString() );
    open_->setText( model_->tableData( QuoteTableModel::OPEN_PRICE ).toString() );
    close_->setText( model_->tableData( QuoteTableModel::CLOSE_PRICE ).toString() );

    dayRange_->setText(
        QString( "%0 - %1" )
            .arg( model_->tableData( QuoteTableModel::LOW_PRICE ).toString() )
            .arg( model_->tableData( QuoteTableModel::HIGH_PRICE ).toString() ) );

    yearRange_->setText(
        QString( "%0 - %1" )
            .arg( model_->tableData( QuoteTableModel::FIFTY_TWO_WEEK_LOW ).toString() )
            .arg( model_->tableData( QuoteTableModel::FIFTY_TWO_WEEK_HIGH ).toString() ) );

    QString divYield( model_->tableData( QuoteTableModel::DIV_YIELD ).toString() );

    if ( divYield.length() )
        divYield.append( "%" );

    div_->setText(
        QString( "%0\n%1" )
            .arg( model_->tableData( QuoteTableModel::DIV_AMOUNT ).toString() )
            .arg( divYield ) );

    const QDateTime divDate( QDateTime::fromString( model_->tableData( QuoteTableModel::DIV_DATE ).toString(), Qt::ISODateWithMs ) );
    const QString divFreq( model_->tableData( QuoteTableModel::DIV_FREQUENCY ).toString() );

    QString divDateStr( divDate.date().toString() );

    if ( divFreq.length() )
        divDateStr.append( QString( " (%0)" ).arg( divFreq ) );

    divDate_->setText( divDateStr );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::onOptionChainUpdated( const QString& underlying, const QList<QDate>& expiryDates, bool background )
{
    Q_UNUSED( background );

    // nothing to do
    if ( symbol() != underlying )
        return;

    LOG_TRACE << "refresh table data";

    // refresh model
    if ( !model_->refreshData() )
    {
        LOG_WARN << "error refreshing quote table data";
        return;
    }

    const bool empty( 0 == expiryDates_->count() );

    // no expiration dates
    // probably symbol without options
    if ( expiryDates.isEmpty() )
    {
        analysisAll_->setEnabled( false );

        // request quote instead
        AbstractDaemon::instance()->getQuote( symbol_ );
    }

    else
    {
        analysisAll_->setEnabled( true );

        // iterate all expiration dates
        foreach ( const QDate& d, expiryDates )
        {
            bool found( false );

            int index( expiryDates_->count() );

            // check tabs for instance of this date
            for ( int i( expiryDates_->count() ); i--; )
            {
                const OptionChainView *view( qobject_cast<OptionChainView*>( expiryDates_->widget( i ) ) );

                OptionChainTableModel *viewModel( (view ? view->model() : nullptr) );

                if ( !viewModel )
                    continue;

                // found!
                if ( d == viewModel->expirationDate() )
                {
                    LOG_TRACE << "existing model";

                    // reset ready
                    viewModel->resetReady();

                    // refresh view model
                    if ( view->isVisible() )
                        if ( !viewModel->refreshData() )
                        {
                            LOG_WARN << "error refreshing chain table data";
                            return;
                        }

                    expiryDates_->setTabText( i, view->title() );

                    found = true;
                    break;
                }

                // otherwise check date so we insert at proper location
                if ( d <= viewModel->expirationDate() )
                    index = i;
            }

            // no instance found; create one
            if ( !found )
            {
                LOG_TRACE << "create new model";

                OptionChainTableModel *viewModel( new OptionChainTableModel( underlying, d, QDateTime() ) );
                OptionChainView *view( new OptionChainView( viewModel, this ) );

                expiryDates_->insertTab( index, view, view->title() );
            }
        }
    }

    // when displaying first time, make sure first tab is shown.
    if (( empty ) && ( expiryDates_->count() ))
        expiryDates_->setCurrentIndex( 0 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::onQuotesUpdated( const QStringList& symbols, bool background )
{
    Q_UNUSED( background );

    // nothing to do
    if ( !symbols.contains( symbol() ) )
        return;

    // refresh model
    model_->refreshData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::onTabCurrentChanged( int index )
{
    analysisOne_->setEnabled( chartTab_ != index );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::initialize()
{
    description_ = new QLabel( this );
    exchange_ = new QLabel( this );
    stamp_ = new QLabel( this );

    QFont f0( description_->font() );
    f0.setPointSize( f0.pointSize() + 4 );

    description_->setFont( f0 );
    stamp_->setFont( f0 );
    exchange_->setFont( f0 );

    lastLabel_ = new QLabel( this );
    last_ = new QLabel( this );

    lastChangeLabel_ = new QLabel( this );
    lastChange_ = new QLabel( this );

    QFont f1( lastChange_->font() );
    f1.setPointSize( f1.pointSize() - 2 );

    lastChange_->setFont( f1 );

    bidLabel_ = new QLabel( this );
    bid_ = new QLabel( this );

    askLabel_ = new QLabel( this );
    ask_ = new QLabel( this );

    sizeLabel_ = new QLabel( this );
    size_ = new QLabel( this );

    markLabel_ = new QLabel( this );
    mark_ = new QLabel( this );

    markChangeLabel_ = new QLabel( this );
    markChange_ = new QLabel( this );
    markChange_->setFont( f1 );

    volumeLabel_ = new QLabel( this );
    volume_ = new QLabel( this );

    openLabel_ = new QLabel( this );
    open_ = new QLabel( this );

    closeLabel_ = new QLabel( this );
    close_ = new QLabel( this );

    dayRangeLabel_ = new QLabel( this );
    dayRange_ = new QLabel( this );

    yearRangeLabel_ = new QLabel( this );
    yearRange_ = new QLabel( this );

    divLabel_ = new QLabel( this );
    div_ = new QLabel( this );
    div_->setFont( f1 );

    divDateLabel_ = new QLabel( this );
    divDate_ = new QLabel( this );

    clear_ = new QToolButton( this );
    clear_->setMinimumWidth( 70 );
    clear_->setIcon( QIcon( ":/res/clear.png" ) );
    clear_->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    clear_->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    clear_->setVisible( false );

    connect( clear_, &QToolButton::clicked, this, &_Myt::onButtonClicked );

    analysisOne_ = new QToolButton( this );
    analysisOne_->setMinimumWidth( 70 );
    analysisOne_->setIcon( QIcon( ":/res/analysis.png" ) );
    analysisOne_->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    analysisOne_->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    analysisOne_->setEnabled( false );

    connect( analysisOne_, &QToolButton::clicked, this, &_Myt::onButtonClicked );

    analysisAll_ = new QToolButton( this );
    analysisAll_->setMinimumWidth( 70 );
    analysisAll_->setIcon( QIcon( ":/res/analysis.png" ) );
    analysisAll_->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    analysisAll_->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

    connect( analysisAll_, &QToolButton::clicked, this, &_Myt::onButtonClicked );

    refresh_ = new QToolButton( this );
    refresh_->setMinimumWidth( 70 );
    refresh_->setIcon( QIcon( ":/res/refresh.png" ) );
    refresh_->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    refresh_->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

    connect( refresh_, &QToolButton::clicked, this, &_Myt::onButtonClicked );

    // ---- //

    splitter_ = new QSplitter( this );
    splitter_->setOrientation( Qt::Vertical );

    expiryDates_ = new QTabWidget( splitter_ );
    expiryDates_->setTabShape( QTabWidget::Triangular );
    expiryDates_->setTabPosition( QTabWidget::North );

    // add chart to tabs
    chartTab_ = expiryDates_->addTab( new SymbolPriceHistoryWidget( symbol() ), QString() );

    connect( expiryDates_, &QTabWidget::currentChanged, this, &_Myt::onTabCurrentChanged );

    tradeAnalysis_ = new OptionTradingView( tradingModel_, splitter_ );
    tradeAnalysis_->setVisible( false );

    splitter_->addWidget( expiryDates_ );
    splitter_->addWidget( tradeAnalysis_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::createLayout()
{
    QHBoxLayout *desc( new QHBoxLayout() );
    desc->addWidget( description_ );
    desc->addStretch();
    desc->addWidget( exchange_ );
    desc->addStretch();
    desc->addWidget( stamp_ );

    QVBoxLayout *last( new QVBoxLayout() );
    last->setContentsMargins( QMargins() );
    last->setSpacing( 0 );
    last->addWidget( lastLabel_ );
    last->addWidget( last_ );

    QVBoxLayout *lastChange( new QVBoxLayout() );
    lastChange->setContentsMargins( QMargins() );
    lastChange->setSpacing( 0 );
    lastChange->addWidget( lastChangeLabel_ );
    lastChange->addWidget( lastChange_ );

    QVBoxLayout *bid( new QVBoxLayout() );
    bid->setContentsMargins( QMargins() );
    bid->setSpacing( 0 );
    bid->addWidget( bidLabel_ );
    bid->addWidget( bid_ );

    QVBoxLayout *ask( new QVBoxLayout() );
    ask->setContentsMargins( QMargins() );
    ask->setSpacing( 0 );
    ask->addWidget( askLabel_ );
    ask->addWidget( ask_ );

    QVBoxLayout *size( new QVBoxLayout() );
    size->setContentsMargins( QMargins() );
    size->setSpacing( 0 );
    size->addWidget( sizeLabel_ );
    size->addWidget( size_ );

    QVBoxLayout *mark( new QVBoxLayout() );
    mark->setContentsMargins( QMargins() );
    mark->setSpacing( 0 );
    mark->addWidget( markLabel_ );
    mark->addWidget( mark_ );

    QVBoxLayout *markChange( new QVBoxLayout() );
    markChange->setContentsMargins( QMargins() );
    markChange->setSpacing( 0 );
    markChange->addWidget( markChangeLabel_ );
    markChange->addWidget( markChange_ );

    QVBoxLayout *volume( new QVBoxLayout() );
    volume->setContentsMargins( QMargins() );
    volume->setSpacing( 0 );
    volume->addWidget( volumeLabel_ );
    volume->addWidget( volume_ );

    QVBoxLayout *open( new QVBoxLayout() );
    open->setContentsMargins( QMargins() );
    open->setSpacing( 0 );
    open->addWidget( openLabel_ );
    open->addWidget( open_ );

    QVBoxLayout *close( new QVBoxLayout() );
    close->setContentsMargins( QMargins() );
    close->setSpacing( 0 );
    close->addWidget( closeLabel_ );
    close->addWidget( close_ );

    QVBoxLayout *dayRange( new QVBoxLayout() );
    dayRange->setContentsMargins( QMargins() );
    dayRange->setSpacing( 0 );
    dayRange->addWidget( dayRangeLabel_ );
    dayRange->addWidget( dayRange_ );

    QVBoxLayout *yearRange( new QVBoxLayout() );
    yearRange->setContentsMargins( QMargins() );
    yearRange->setSpacing( 0 );
    yearRange->addWidget( yearRangeLabel_ );
    yearRange->addWidget( yearRange_ );

    QVBoxLayout *div( new QVBoxLayout() );
    div->setContentsMargins( QMargins() );
    div->setSpacing( 0 );
    div->addWidget( divLabel_ );
    div->addWidget( div_ );

    QVBoxLayout *divDate( new QVBoxLayout() );
    divDate->setContentsMargins( QMargins() );
    divDate->setSpacing( 0 );
    divDate->addWidget( divDateLabel_ );
    divDate->addWidget( divDate_ );

    QHBoxLayout *underlying( new QHBoxLayout() );
    underlying->addLayout( last, 1 );
    underlying->addLayout( lastChange, 1 );
    underlying->addLayout( bid, 1 );
    underlying->addLayout( ask, 1 );
    underlying->addLayout( size, 1 );
    underlying->addLayout( mark, 1 );
    underlying->addLayout( markChange, 1 );
    underlying->addLayout( volume, 1 );
    underlying->addLayout( open, 1 );
    underlying->addLayout( close, 1 );
    underlying->addLayout( dayRange, 1 );
    underlying->addLayout( yearRange, 1 );
    underlying->addLayout( div, 1 );
    underlying->addLayout( divDate, 1 );

    QVBoxLayout *underlyingInfo( new QVBoxLayout() );
    underlyingInfo->addLayout( desc );
    underlyingInfo->addLayout( underlying, 1 );

    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->setContentsMargins( QMargins() );
    buttons->addWidget( clear_ );
    buttons->addWidget( analysisOne_ );
    buttons->addWidget( analysisAll_ );
    buttons->addWidget( refresh_ );

    QHBoxLayout *header( new QHBoxLayout() );
    header->setContentsMargins( QMargins() );
    header->addLayout( underlyingInfo, 1 );
    header->addLayout( buttons );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addLayout( header );
    form->addWidget( splitter_, 1 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::saveState( QSplitter *w ) const
{
    if ( !w )
        return;

    AppDatabase::instance()->setWidgetState( AppDatabase::Splitter, STATE_GROUP_NAME, STATE_NAME, w->saveState() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::restoreState( QSplitter *w ) const
{
    if ( !w )
        return;

    splitter_->restoreState( AppDatabase::instance()->widgetState( AppDatabase::Splitter, STATE_GROUP_NAME, STATE_NAME ) );
}

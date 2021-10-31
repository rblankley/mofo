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

#include "db/appdb.h"
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

static const QString SPLITTER_STATE( "optionViewerSplitterState" );

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionViewerWidget::OptionViewerWidget( const QString& symbol, QWidget *parent ) :
    _Mybase( parent ),
    symbol_( symbol )
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
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionViewerWidget::~OptionViewerWidget()
{
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
    analysis_->setText( tr( "Analysis" ) );
    refresh_->setText( tr( "Refresh" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::onButtonPressed()
{
    // refresh
    if ( refresh_ == sender() )
        AbstractDaemon::instance()->getOptionChain( symbol_ );

    // clear
    else if ( clear_ == sender() )
    {
        // clear model
        tradingModel_->removeAllRows();

        // hide analysis results
        tradeAnalysis_->hide();

        clear_->setVisible( false );
    }

    // analysis
    else if ( analysis_ == sender() )
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

        // show view
        const OptionChainView *view( qobject_cast<const OptionChainView*>( expiryDates_->currentWidget() ) );

        if ( view )
        {
            // create a calculator
            OptionProfitCalculator *calc( OptionProfitCalculator::create( model_->tableData( QuoteTableModel::MARK ).toDouble(), view->model(), tradingModel_ ) );

            if ( !calc )
                LOG_WARN << "no calculator";
            else
            {
                // show analysis results
                tradeAnalysis_->show();

                // this could take a while...
                QApplication::setOverrideCursor( Qt::WaitCursor );

                // setup calculator
                calc->setFilter( calcFilter );
                calc->setOptionTradeCost( AppDatabase::instance()->optionTradeCost() );

                // analyze
                calc->analyze( OptionTradingItemModel::SINGLE );
                calc->analyze( OptionTradingItemModel::VERT_BEAR_CALL );
                calc->analyze( OptionTradingItemModel::VERT_BULL_PUT );

                // done
                QApplication::restoreOverrideCursor();

                clear_->setVisible( true );

                OptionProfitCalculator::destroy( calc );
            }
        }
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
    Q_UNUSED( expiryDates );
    Q_UNUSED( background );

    // nothing to do
    if ( symbol() != underlying )
        return;

    // refresh model
    model_->refreshTableData();

    const bool empty( 0 == expiryDates_->count() );

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
                // refresh model
                viewModel->refreshTableData();

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
            OptionChainTableModel *viewModel( new OptionChainTableModel( underlying, d, QDateTime() ) );
            OptionChainView *view( new OptionChainView( viewModel, this ) );

            // refresh model
            viewModel->refreshTableData();

            expiryDates_->insertTab( index, view, view->title() );
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
    model_->refreshTableData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionViewerWidget::onSplitterMoved( int pos, int index )
{
    Q_UNUSED( pos );
    Q_UNUSED( index );

    AppDatabase::instance()->setHeaderState( SPLITTER_STATE, splitter_->saveState() );
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
    clear_->setVisible( false );

    connect( clear_, &QToolButton::clicked, this, &_Myt::onButtonPressed );

    analysis_ = new QToolButton( this );
    analysis_->setMinimumWidth( 70 );
    analysis_->setIcon( QIcon( ":/res/analysis.png" ) );
    analysis_->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

    connect( analysis_, &QToolButton::clicked, this, &_Myt::onButtonPressed );

    refresh_ = new QToolButton( this );
    refresh_->setMinimumWidth( 70 );
    refresh_->setIcon( QIcon( ":/res/refresh.png" ) );
    refresh_->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

    connect( refresh_, &QToolButton::clicked, this, &_Myt::onButtonPressed );

    // ---- //

    splitter_ = new QSplitter( this );
    splitter_->setOrientation( Qt::Vertical );

    connect( splitter_, &QSplitter::splitterMoved, this, &_Myt::onSplitterMoved );

    expiryDates_ = new QTabWidget( splitter_ );
    expiryDates_->setTabShape( QTabWidget::Triangular );
    expiryDates_->setTabPosition( QTabWidget::North );

    tradeAnalysis_ = new OptionTradingView( tradingModel_, splitter_ );
    tradeAnalysis_->setVisible( false );

    splitter_->addWidget( expiryDates_ );
    splitter_->addWidget( tradeAnalysis_ );

    // ---- //

    const QByteArray a( AppDatabase::instance()->headerState( SPLITTER_STATE ) );

    if ( !a.isNull() )
        splitter_->restoreState( a );
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

    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->setContentsMargins( QMargins() );
    buttons->addWidget( clear_ );
    buttons->addWidget( analysis_ );
    buttons->addWidget( refresh_ );

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
    underlying->addLayout( buttons );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addLayout( desc );
    form->addLayout( underlying );
    form->addWidget( splitter_ );
}

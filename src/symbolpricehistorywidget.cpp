/**
 * @file symbolpricehistorywidget.cpp
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
#include "symbolpricehistorywidget.h"

#include "db/appdb.h"
#include "db/symboldbs.h"

#include <cmath>

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QCryptographicHash>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QStandardItem>
#include <QStandardItemModel>

const QString SymbolPriceHistoryWidget::STATE_GROUP_NAME( "symbolPriceHistory" );
const QString SymbolPriceHistoryWidget::STATE_NAME( "[[default]]" );

// periods
static const QString DAY( "day" );
static const QString MONTH( "month" );
static const QString YEAR( "year" );
static const QString YTD( "ytd" );

// frequency
static const QString MINUTE( "minute" );
static const QString DAILY( "daily" );
static const QString WEEKLY( "weekly" );
static const QString MONTHLY( "monthly" );

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolPriceHistoryWidget::SymbolPriceHistoryWidget( const QString& symbol, QWidget *parent ) :
    _Mybase( parent ),
    init_( false ),
    symbol_( symbol )
{
    // init
    initialize();
    createLayout();
    translate();

    // restore state
    // use state from symbol first... upon failure use default state
    QString state( AppDatabase::instance()->widgetState( AppDatabase::PriceHistory, STATE_GROUP_NAME, symbol ) );

    if ( state.isEmpty() )
        state = AppDatabase::instance()->widgetState( AppDatabase::PriceHistory, STATE_GROUP_NAME, STATE_NAME );

    if ( state.length() )
    {
        const QStringList settings( state.split( '/' ) );

        if ( 4 == settings.size() )
        {
            period_->setCurrentIndex( period_->findData( settings[0] ) );

            freqMin_->setCurrentIndex( freqMin_->findData( settings[1] ) );
            freqDayWeek_->setCurrentIndex( freqDayWeek_->findData( settings[2] ) );
            freqDayWeekMonth_->setCurrentIndex( freqDayWeekMonth_->findData( settings[3] ) );
        }
    }

    // connect candle data signal
    connect( SymbolDatabases::instance(), &SymbolDatabases::candleDataChanged, this, &_Myt::onCandleDataChanged, Qt::QueuedConnection );

    init_ = true;

    // refresh history
    refreshData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolPriceHistoryWidget::~SymbolPriceHistoryWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::translate()
{
    period_->setItemText( 0, tr( "1D" ) );
    period_->setItemText( 1, tr( "2D" ) );
    period_->setItemText( 2, tr( "3D" ) );
    period_->setItemText( 3, tr( "4D" ) );
    period_->setItemText( 4, tr( "5D" ) );
    period_->setItemText( 5, tr( "10D" ) );
    period_->setItemText( 6, tr( "1M" ) );
    period_->setItemText( 7, tr( "2M" ) );
    period_->setItemText( 8, tr( "3M" ) );
    period_->setItemText( 9, tr( "6M" ) );
    period_->setItemText( 10, tr( "1Y" ) );
    period_->setItemText( 11, tr( "2Y" ) );
    period_->setItemText( 12, tr( "3Y" ) );
    period_->setItemText( 13, tr( "5Y" ) );
    period_->setItemText( 14, tr( "10Y" ) );
    period_->setItemText( 15, tr( "15Y" ) );
    period_->setItemText( 16, tr( "20Y" ) );
    period_->setItemText( 17, tr( "YTD" ) );

    freqMin_->setItemText( 0, tr( "1m" ) );
    freqMin_->setItemText( 1, tr( "5m" ) );
    freqMin_->setItemText( 2, tr( "10m" ) );
    freqMin_->setItemText( 3, tr( "15m" ) );
    freqMin_->setItemText( 4, tr( "30m" ) );

    freqDayWeek_->setItemText( 0, tr( "Day" ) );
    freqDayWeek_->setItemText( 1, tr( "Week" ) );

    freqDayWeekMonth_->setItemText( 0, tr( "Day" ) );
    freqDayWeekMonth_->setItemText( 1, tr( "Week" ) );
    freqDayWeekMonth_->setItemText( 2, tr( "Month" ) );

    translateOverlays( overlays_ );

    translateLowers( lowers_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::refreshData()
{
    int p, f;
    QString ptype, ftype;

    // check we are initialized
    if ( !init_ )
        return;

    // retrieve period and frequency
    if (( currentPeriod( p, ptype ) ) && ( currentFrequency( f, ftype ) ))
    {
        // clear graph
        candles_.clear();
        graph_ = margin_ = QPixmap();

        ma_.clear();

        hv_.clear();
        macd_.clear();
        rsi_.clear();

        // fetch
        AbstractDaemon::instance()->getCandles( symbol(), p, ptype, f, ftype );

        // refresh
        update();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::paintEvent( QPaintEvent *e )
{
    QPainter painter;
    painter.begin( this );

    // fill background color
    painter.fillRect( rect(), palette().base().color() );

    // graph
    if ( !graph_.isNull() )
    {
        int offset( 0 );

        if ( !margin_.isNull() )
            offset += margin_.width();

        if ( scrollBarVisible() )
            offset -= scroll_->maximum() - scroll_->value();

        painter.drawPixmap( width() - graph_.width() - offset, 0, graph_ );
    }

    // margin
    if ( !margin_.isNull() )
        painter.drawPixmap( width() - margin_.width(), 0, margin_ );

    painter.end();

    _Mybase::paintEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::resizeEvent( QResizeEvent *e )
{
    // new graph
    drawGraph();

    _Mybase::resizeEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::onCandleDataChanged( const QString& s, const QDateTime& start, const QDateTime& stop, int period, const QString& periodType, int freq, const QString& freqType, const QList<CandleData>& candles )
{
    Q_UNUSED( start )
    Q_UNUSED( stop )

    // check symbol
    if ( s != symbol() )
        return;

    // check period and freq
    int p, f;
    QString ptype, ftype;

    if (( currentPeriod( p, ptype ) ) && ( period == p ) && ( periodType == ptype ) &&
        ( currentFrequency( f, ftype ) ) && ( freq == f ) && ( freqType == ftype ))
    {
        // set candles
        candles_ = candles;

        // reset scroll bar
        scroll_->setValue( 0 );

        // draw new graph
        drawGraph();

        // default scroll to far right
        if ( scrollBarVisible() )
            scroll_->setValue( scroll_->maximum() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::onCurrentIndexChanged( int index )
{
    Q_UNUSED( index )

    // for lower graph, just need to draw
    if ( lowers_ == sender() )
    {
        drawGraph();
        return;
    }

    // hide/show freq boxes
    if ( period_ == sender() )
    {
        int p;
        QString ptype;

        // fetch period
        if ( !currentPeriod( p, ptype ) )
            return;

        freqMin_->hide();
        freqDayWeek_->hide();
        freqDayWeekMonth_->hide();

        if ( DAY == ptype )
            freqMin_->show();
        else if ( YEAR == ptype )
            freqDayWeekMonth_->show();
        else
            freqDayWeek_->show();
    }

    // check frequency selection
    int f;
    QString ftype;

    if ( currentFrequency( f, ftype ) )
    {
        const bool isDaily( DAILY == ftype );

        overlays_->setVisible( isDaily );
        lowers_->setVisible( isDaily );
    }

    // refresh
    refreshData();

    // save state
    const QString state(
        period_->currentData().toString() + "/" +
        freqMin_->currentData().toString() + "/" +
        freqDayWeek_->currentData().toString() + "/" +
        freqDayWeekMonth_->currentData().toString() );

    AppDatabase::instance()->setWidgetState( AppDatabase::PriceHistory, STATE_GROUP_NAME, symbol(), state.toLatin1() );
    AppDatabase::instance()->setWidgetState( AppDatabase::PriceHistory, STATE_GROUP_NAME, STATE_NAME, state.toLatin1() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::onItemChanged( QStandardItem *item )
{
    Q_UNUSED( item )

    // update graph
    drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::onValueChanged( int value )
{
    Q_UNUSED( value )

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::initialize()
{
    period_ = new QComboBox( this );
    period_->addItem( QString(), "1:" + DAY );
    period_->addItem( QString(), "2:" + DAY );
    period_->addItem( QString(), "3:" + DAY );
    period_->addItem( QString(), "4:" + DAY );
    period_->addItem( QString(), "5:" + DAY );
    period_->addItem( QString(), "10:" + DAY );
    period_->addItem( QString(), "1:" + MONTH );
    period_->addItem( QString(), "2:" + MONTH );
    period_->addItem( QString(), "3:" + MONTH );
    period_->addItem( QString(), "6:" + MONTH );
    period_->addItem( QString(), "1:" + YEAR );
    period_->addItem( QString(), "2:" + YEAR );
    period_->addItem( QString(), "3:" + YEAR );
    period_->addItem( QString(), "5:" + YEAR );
    period_->addItem( QString(), "10:" + YEAR );
    period_->addItem( QString(), "15:" + YEAR );
    period_->addItem( QString(), "20:" + YEAR );
    period_->addItem( QString(), "1:" + YTD );

    freqMin_ = new QComboBox( this );
    freqMin_->addItem( QString(), "1:" + MINUTE );
    freqMin_->addItem( QString(), "5:" + MINUTE );
    freqMin_->addItem( QString(), "10:" + MINUTE );
    freqMin_->addItem( QString(), "15:" + MINUTE );
    freqMin_->addItem( QString(), "30:" + MINUTE );

    freqDayWeek_ = new QComboBox( this );
    freqDayWeek_->addItem( QString(), "1:" + DAILY );
    freqDayWeek_->addItem( QString(), "1:" + WEEKLY );

    freqDayWeekMonth_ = new QComboBox( this );
    freqDayWeekMonth_->addItem( QString(), "1:" + DAILY );
    freqDayWeekMonth_->addItem( QString(), "1:" + WEEKLY );
    freqDayWeekMonth_->addItem( QString(), "1:" + MONTHLY );

    overlays_ = new QComboBox( this );

    lowers_ = new QComboBox( this );

    scroll_ = new QScrollBar( Qt::Horizontal, this );
    scroll_->hide();

    // set defaults
    period_->setCurrentIndex( 5 );
    freqMin_->setCurrentIndex( 4 );
    freqDayWeek_->hide();
    freqDayWeekMonth_->hide();

    // connect
    connect( period_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );
    connect( freqMin_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );
    connect( freqDayWeek_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );
    connect( freqDayWeekMonth_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );

    connect( lowers_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );

    connect( scroll_, &QScrollBar::valueChanged, this, &_Myt::onValueChanged );

    //
    // overlays
    //

    QStringList otypes;
    otypes.append( "SMA" );
    otypes.append( "EMA" );

    QStringList otypedepths;
    otypedepths.append( "5" );
    otypedepths.append( "10" );
    otypedepths.append( "12" );
    otypedepths.append( "15" );
    otypedepths.append( "20" );
    otypedepths.append( "26" );
    otypedepths.append( "30" );
    otypedepths.append( "50" );
    otypedepths.append( "100" );
    otypedepths.append( "200" );

    QStandardItemModel *overlaysModel( new QStandardItemModel( 0, 1, this ) );
    QStandardItem *item;

    item = new QStandardItem( QString() );
    overlaysModel->appendRow( item );

    foreach ( const QString& otype, otypes )
        foreach ( const QString& otypedepth, otypedepths )
        {
            // SMA12 and SMA26 are not supported
            if (( "SMA" == otype ) && (( "12" == otypedepth ) || ( "26" == otypedepth )))
                continue;

            item = new QStandardItem();
            item->setData( otype + otypedepth, Qt::UserRole );
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
            item->setCheckState( Qt::Unchecked );
            overlaysModel->appendRow( item );
        }

    overlays_->setModel( overlaysModel );

    connect( overlaysModel, &QStandardItemModel::itemChanged, this, &_Myt::onItemChanged );

    //
    // lowers
    //

    lowers_->addItem( QString(), "NONE" );
    lowers_->addItem( QString(), "MACD" );

    QStringList rsidepths;
    rsidepths.append( "2" );
    rsidepths.append( "3" );
    rsidepths.append( "4" );
    rsidepths.append( "5" );
    rsidepths.append( "6" );
    rsidepths.append( "10" );
    rsidepths.append( "14" );
    rsidepths.append( "20" );
    rsidepths.append( "50" );

    foreach ( const QString& rsidepth, rsidepths )
        lowers_->addItem( QString(), "RSI" + rsidepth );

    QStringList hvdepths;
    hvdepths.append( "5" );
    hvdepths.append( "10" );
    hvdepths.append( "20" );
    hvdepths.append( "30" );
    hvdepths.append( "60" );
    hvdepths.append( "90" );
    hvdepths.append( "120" );
    hvdepths.append( "240" );
    hvdepths.append( "480" );

    foreach ( const QString& hvdepth, hvdepths )
        lowers_->addItem( QString(), "HV" + hvdepth );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::createLayout()
{
    QHBoxLayout *boxes( new QHBoxLayout() );
    boxes->addWidget( period_ );
    boxes->addWidget( freqMin_ );
    boxes->addWidget( freqDayWeek_ );
    boxes->addWidget( freqDayWeekMonth_ );
    boxes->addWidget( overlays_ );
    boxes->addWidget( lowers_ );
    boxes->addStretch();

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addLayout( boxes );
    form->addStretch();
    form->addWidget( scroll_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolPriceHistoryWidget::currentPeriod( int& period, QString& periodType ) const
{
    // fetch period
    const QString pstr( period_->currentData().toString() );
    const QStringList p( pstr.split( ':' ) );

    if ( 2 != p.size() )
        return false;

    period = p[0].toInt();
    periodType = p[1];
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolPriceHistoryWidget::currentFrequency( int& freq, QString& freqType ) const
{
    int p;
    QString ptype;

    if ( !currentPeriod( p, ptype ) )
        return false;

    // fetch frequency
    QString fstr;

    if ( DAY == ptype )
        fstr = freqMin_->currentData().toString();
    else if ( YEAR == ptype )
        fstr = freqDayWeekMonth_->currentData().toString();
    else
        fstr = freqDayWeek_->currentData().toString();

    const QStringList f( fstr.split( ':' ) );

    if ( 2 != f.size() )
        return false;

    freq = f[0].toInt();
    freqType = f[1];
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int SymbolPriceHistoryWidget::scrollBarMaximum() const
{
    if ( candles_.isEmpty() )
        return 0;
    else if (( graph_.isNull() ) || ( margin_.isNull() ))
        return 0;

    return qMax( 0, (graph_.width() + margin_.width()) - width() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolPriceHistoryWidget::haveHistoricalVolatilities()
{
    if ( 0 == lowers_->currentIndex() )
        return false;

    // check for lower graph HV selected
    const QString data( lowers_->currentData().toString() );

    if (( data.startsWith( "HV" ) ) && ( candles_.size() ))
    {
        const QDate start( candles_.first().stamp.date() );
        const QDate end( candles_.last().stamp.date() );

        // fetch historical volatilties
        if ( hv_.isEmpty() )
        {
            QApplication::setOverrideCursor( Qt::WaitCursor );

            // fetch!
            SymbolDatabases::instance()->historicalVolatilities( symbol(), start, end, hv_ );

            QApplication::restoreOverrideCursor();
        }

        return (( 2 <= hv_.size() ) && ( hv_.size() <= candles_.size() ) && ( hv_.last().date == end ));
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolPriceHistoryWidget::haveMovingAverages()
{
    bool check( false );

    // check for overlays selected
    const QAbstractItemModel *model( overlays_->model() );
    const QModelIndexList indexes( model->match( model->index( 1, 0 ), Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly ) );

    foreach ( const QModelIndex& index, indexes )
    {
        const QString data( model->data( index, Qt::UserRole ).toString() );

        if (( "EMA12" != data ) && ( "EMA26" != data ))
        {
            check = true;
            break;
        }
    }

    if (( check ) && ( candles_.size() ))
    {
        const QDate start( candles_.first().stamp.date() );
        const QDate end( candles_.last().stamp.date() );

        // fetch moving averages
        if ( ma_.isEmpty() )
        {
            QApplication::setOverrideCursor( Qt::WaitCursor );

            // fetch!
            SymbolDatabases::instance()->movingAverages( symbol(), start, end, ma_ );

            QApplication::restoreOverrideCursor();
        }

        return (( 2 <= ma_.size() ) && ( ma_.size() <= candles_.size() ) && ( ma_.last().date == end ));
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolPriceHistoryWidget::haveMovingAveragesConvergenceDivergence( bool emaOnly )
{
    bool check( false );

    // check EMA12 or EMA26 selected
    if ( emaOnly )
    {
        // check for overlays selected
        const QAbstractItemModel *model( overlays_->model() );
        const QModelIndexList indexes( model->match( model->index( 1, 0 ), Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly ) );

        foreach ( const QModelIndex& index, indexes )
        {
            const QString data( model->data( index, Qt::UserRole ).toString() );

            if (( "EMA12" == data ) || ( "EMA26" == data ))
            {
                check = true;
                break;
            }
        }
    }

    // check for lower graph MACD selected
    else if ( lowers_->currentIndex() )
    {
        const QString data( lowers_->currentData().toString() );

        check = ("MACD" == data);
    }

    if (( check ) && ( candles_.size() ))
    {
        const QDate start( candles_.first().stamp.date() );
        const QDate end( candles_.last().stamp.date() );

        // fetch MACD
        if ( macd_.isEmpty() )
        {
            QApplication::setOverrideCursor( Qt::WaitCursor );

            // fetch!
            SymbolDatabases::instance()->movingAveragesConvergenceDivergence( symbol(), start, end, macd_ );

            QApplication::restoreOverrideCursor();
        }

        return (( 2 <= macd_.size() ) && ( macd_.size() <= candles_.size() ) && ( macd_.last().date == end ));
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolPriceHistoryWidget::haveRelativeStrengthIndexes()
{
    if ( 0 == lowers_->currentIndex() )
        return false;

    // check for lower graph RSI selected
    const QString data( lowers_->currentData().toString() );

    if (( data.startsWith( "RSI" ) ) && ( candles_.size() ))
    {
        const QDate start( candles_.first().stamp.date() );
        const QDate end( candles_.last().stamp.date() );

        // fetch relative strength indexes
        if ( rsi_.isEmpty() )
        {
            QApplication::setOverrideCursor( Qt::WaitCursor );

            // fetch!
            SymbolDatabases::instance()->relativeStrengthIndex( symbol(), start, end, rsi_ );

            QApplication::restoreOverrideCursor();
        }

        return (( 2 <= rsi_.size() ) && ( rsi_.size() <= candles_.size() ) && ( rsi_.last().date == end ));
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <>
bool SymbolPriceHistoryWidget::calcMinMaxValues( const QList<CandleData>& values, double& min, double& max, unsigned long long& vmax ) const
{
    min = 999999.99;
    max = 0.0;

    vmax = 0;

    foreach ( const CandleData& cd, values )
    {
        min = qMin( min, cd.lowPrice );
        max = qMax( max, cd.highPrice );

        vmax = qMax( vmax, cd.totalVolume );
    }

    return (min <= max);
}

template <>
bool SymbolPriceHistoryWidget::calcMinMaxValues( const QList<HistoricalVolatilities>& values, double& min, double& max, unsigned long long& vmax ) const
{
    Q_UNUSED( vmax );

    min = 999999.99;
    max = 0.0;

    const QString data( lowers_->currentData().toString() );
#if QT_VERSION_CHECK( 5, 15, 2 ) <= QT_VERSION
    const int index( QStringView{ data }.mid( 2 ).toInt() );
#else
    const int index( data.midRef( 2 ).toInt() );
#endif

    foreach ( const HistoricalVolatilities& hv, values )
        if ( hv.volatilities.contains( index ) )
        {
            const double val( 100.0 * hv.volatilities[index] );

            min = qMin( min, val );
            max = qMax( max, val );
        }

    return (min <= max);
}

template <>
bool SymbolPriceHistoryWidget::calcMinMaxValues( const QList<MovingAverages>& values, double& min, double& max, unsigned long long& vmax ) const
{
    Q_UNUSED( vmax );

    min = 999999.99;
    max = 0.0;

    // check for overlays selected
    const QAbstractItemModel *model( overlays_->model() );
    const QModelIndexList indexes( model->match( model->index( 1, 0 ), Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly ) );

    foreach ( const MovingAverages& ma, values )
        foreach ( const QModelIndex& index, indexes )
        {
            const QString data( model->data( index, Qt::UserRole ).toString() );
#if QT_VERSION_CHECK( 5, 15, 2 ) <= QT_VERSION
            const int d( QStringView{ data }.mid( 3 ).toInt() );
#else
            const int d( data.midRef( 3 ).toInt() );
#endif

            double val;

            if (( data.startsWith( "SMA" ) ) && ( ma.sma.contains( d ) ))
                val = ma.sma[d];
            else if (( data.startsWith( "EMA" ) ) && ( ma.ema.contains( d ) ))
                val = ma.ema[d];
            else
            {
                continue;
            }

            min = qMin( min, val );
            max = qMax( max, val );
        }

    return (min <= max);
}

template <>
bool SymbolPriceHistoryWidget::calcMinMaxValues( const QList<MovingAveragesConvergenceDivergence>& values, double& min, double& max, unsigned long long& vmax ) const
{
    min =  999999.99;
    max = -999999.99;

    // vmax == 0 - MACD min/max values
    // vmax != 0 - EMA min/max values
    if ( 0 == vmax )
    {
        foreach ( const MovingAveragesConvergenceDivergence& macd, values )
        {
            min = qMin( min, macd.macd );
            min = qMin( min, macd.signal );
            min = qMin( min, macd.histogram );

            max = qMax( max, macd.macd );
            max = qMax( max, macd.signal );
            max = qMax( max, macd.histogram );
        }
    }
    else
    {
        // check for overlays selected
        const QAbstractItemModel *model( overlays_->model() );
        const QModelIndexList indexes( model->match( model->index( 1, 0 ), Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly ) );

        foreach ( const MovingAveragesConvergenceDivergence& macd, values )
            foreach ( const QModelIndex& index, indexes )
            {
                const QString data( model->data( index, Qt::UserRole ).toString() );
#if QT_VERSION_CHECK( 5, 15, 2 ) <= QT_VERSION
                const int d( QStringView{ data }.mid( 3 ).toInt() );
#else
                const int d( data.midRef( 3 ).toInt() );
#endif

                double val;

                if ( macd.ema.contains( d ) )
                    val = macd.ema[d];
                else
                {
                    continue;
                }

                min = qMin( min, val );
                max = qMax( max, val );
            }

    }

    return (min <= max);
}

template <>
bool SymbolPriceHistoryWidget::calcMinMaxValues( const QList<RelativeStrengthIndexes>& values, double& min, double& max, unsigned long long& vmax ) const
{
    Q_UNUSED( vmax );

    min = 999999.99;
    max = 0.0;

    const QString data( lowers_->currentData().toString() );
#if QT_VERSION_CHECK( 5, 15, 2 ) <= QT_VERSION
    const int index( QStringView{ data }.mid( 3 ).toInt() );
#else
    const int index( data.midRef( 3 ).toInt() );
#endif

    foreach ( const RelativeStrengthIndexes& rsi, values )
        if ( rsi.values.contains( index ) )
        {
            const double val( rsi.values[index] );

            min = qMin( min, val );
            max = qMax( max, val );
        }

    return (min <= max);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::calcIntervalValues( double min, double max, double gheight, double div, double& interval, int& numDecimals ) const
{
    static const int FOOTER( 25 );
    static const double MAX_MULT( 1000.0 );

    const double MIN_INTERVAL_HEIGHT( 50.0 / div );

    // determine price interval
    QList<double> intervals;
    intervals.append( 1.0 );
    intervals.append( 2.0 );
    intervals.append( 5.0 );

    double mult( 0.0001 );

    interval = 0.0;

    do
    {
        foreach ( double val, intervals )
        {
            const double i( val*mult );
            const double h( (gheight - FOOTER) / ((max - min) / i) );

            if (( MIN_INTERVAL_HEIGHT <= h ) || ( MAX_MULT <= mult ))
            {
                interval = i;
                break;
            }
        }

        mult *= 10.0;

    } while ( interval <= 0.0 );

    // number of decimal places
    numDecimals = 2;

    if ( interval < 0.0009 )
        numDecimals = 4;
    else if ( interval < 0.009 )
        numDecimals = 3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::drawGraph()
{
    unsigned long long unused;

    // no data
    if ( candles_.isEmpty() )
        return;

    // check period and frequency (for labels)
    int p, f;
    QString ptype, ftype;

    if (( !currentPeriod( p, ptype ) ) || ( !currentFrequency( f, ftype ) ))
        return;

    // height to small
    if ( height() < 128 )
        return;

    QPainter painter;

    // determine candles min/max and volume max
    double gmin;
    double gmax;

    unsigned long long vmax;

    calcMinMaxValues( candles_, gmin, gmax, vmax );

    // check overlay min/max
    const bool maOverlayValid( haveMovingAverages() );
    const bool macdOverlayValid( haveMovingAveragesConvergenceDivergence( true ) );

    if ( maOverlayValid )
    {
        double min;
        double max;

        if ( calcMinMaxValues( ma_, min, max, unused ) )
        {
            gmin = qMin( gmin, min );
            gmax = qMax( gmax, max );
        }
    }

    if ( macdOverlayValid )
    {
        double min;
        double max;

        if ( calcMinMaxValues( macd_, min, max, (unused = 1) ) )
        {
            gmin = qMin( gmin, min );
            gmax = qMax( gmax, max );
        }
    }

    // determine interval
    double ginterval;
    int numDecimalPlaces;

    calcIntervalValues( gmin, gmax, height(), 1.0, ginterval, numDecimalPlaces );

    // graph constants
    gmin = ginterval * std::floor( gmin / ginterval );
    gmax = ginterval * std::ceil( gmax / ginterval );

    const QFontMetrics fm( fontMetrics() );

    int marginWidth( SPACING + fm.boundingRect( QString::number( gmax, 'f', numDecimalPlaces ) ).width() );
    int marginHeight( SPACING + fm.boundingRect( "0123456789/:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ).height() );

    // check for lower existance
    const bool hvLowerValid( haveHistoricalVolatilities() );
    const bool rsiLowerValid( haveRelativeStrengthIndexes() );
    const bool macdLowerValid( haveMovingAveragesConvergenceDivergence() );

    bool lowerExists(
        hvLowerValid ||
        rsiLowerValid ||
        macdLowerValid );

    double lmin( 0.0 );
    double lmax( 0.0 );

    double linterval( 0.0 );
    int numDecimalPlacesLower( 2 );

    if ( lowerExists )
    {
        // determine lower min/max
        if ( hvLowerValid )
            lowerExists &= calcMinMaxValues( hv_, lmin, lmax, unused );
        else if ( rsiLowerValid )
            lowerExists &= calcMinMaxValues( rsi_, lmin, lmax, unused );
        else if ( macdLowerValid )
            lowerExists &= calcMinMaxValues( macd_, lmin, lmax, (unused = 0) );

        if ( lowerExists )
        {
            // determine interval
            calcIntervalValues( lmin, lmax, height() / 4, 2.0, linterval, numDecimalPlacesLower );

            // graph constants
            lmin = linterval * std::floor( lmin / linterval );
            lmax = linterval * std::ceil( lmax / linterval );

            // adjust margin width
            marginWidth = qMax( marginWidth, qMax(
                SPACING + fm.boundingRect( QString::number( lmin, 'f', numDecimalPlacesLower ) ).width(),
                SPACING + fm.boundingRect( QString::number( lmax, 'f', numDecimalPlacesLower ) ).width() ) );
        }
    }

    // compute candles width
    int cwidth( MIN_CANDLE_WIDTH + 2 );

    while ( (marginWidth + (cwidth * candles_.size())) < width() )
        cwidth += 2;

    cwidth -= 2;

    int gheight( height() );
    int gwidth( cwidth * candles_.size() );

    // check scroll bar visible
    int smax( qMax( 0, (gwidth + marginWidth) - width()) );

    if ( 0 < smax )
    {
        scroll_->setRange( 0, smax );
        scroll_->setPageStep( gwidth );
        scroll_->setSingleStep( cwidth );
        scroll_->show();

        gheight -= scroll_->height();
    }
    else
    {
        scroll_->hide();
    }

    // -----
    // graph
    // -----

    int gbottom( gheight - marginHeight );
    int lbottom( 0 );

    if ( lowerExists )
    {
        lbottom = gbottom;

        gbottom *= 3;
        gbottom /= 4;
    }

    graph_ = QPixmap( gwidth, gheight );
    graph_.fill( palette().base().color() );

    painter.begin( &graph_ );

    // price intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );

    for ( double i( gmin ); i <= gmax; i += ginterval )
    {
        const int y( gbottom - scaled( i, gmin, gmax, gbottom ) );

        painter.drawLine( 0, y, gwidth, y );
    }

    // candles
    const int coffset( (cwidth - MIN_CANDLE_WIDTH) / 2 );

    QDate dprev;

    int x( coffset );
    int xfooter( -1 );

    for ( int idx( 0 ); idx < candles_.size(); ++idx )
    {
        const CandleData cd( candles_[idx] );

        const QColor fill( (cd.openPrice <= cd.closePrice) ? Qt::darkGreen : Qt::red );

        // total volume
        const QRectF vrect(
            QPointF( x+1, gbottom ),
            QPointF( x+2, gbottom - scaled( cd.totalVolume, 0.0, 2.0*vmax, gbottom ) ) );

        painter.setPen( QPen( Qt::darkGray, 0 ) );
        painter.setBrush( QBrush( Qt::darkGray ) );
        painter.drawRect( vrect );

        // high/low price
        const QRectF hlrect(
            QPointF( x+1, gbottom - scaled( cd.lowPrice, gmin, gmax, gbottom ) ),
            QPointF( x+2, gbottom - scaled( cd.highPrice, gmin, gmax, gbottom ) ) );

        painter.setPen( QPen( fill, 0 ) );
        painter.setBrush( QBrush( fill ) );
        painter.drawRect( hlrect );

        // open/close price
        const QRectF ocrect(
            QPointF( x-coffset, gbottom - scaled( cd.openPrice, gmin, gmax, gbottom ) ),
            QPointF( x+coffset+3, gbottom - scaled( cd.closePrice, gmin, gmax, gbottom ) ) );

        painter.drawRect( ocrect );

        // label
        QString label;

        // monthly
        if ( MONTHLY == ftype )
        {
            const QDate d( cd.stamp.date() );

            // every year
            if ( 1 == d.month() )
                label = d.toString( "yyyy" );
        }
        // weekly
        else if ( WEEKLY == ftype )
        {
            const QDate d( cd.stamp.date() );

            // every 3 months
            if ((( YTD == ptype ) && ( !dprev.isValid() )) ||
                (( dprev.isValid() ) && ( dprev.month() != d.month() ) && ( 0 == ((d.month() - 1) % 3) )))
            {
                if ( 1 == d.month() )
                    label = d.toString( "d MMM yy" );
                else
                    label = d.toString( "d MMM" );
            }

            dprev = d;
        }
        // daily
        else if ( DAILY == ftype )
        {
            const QDate d( cd.stamp.date() );

            // every month
            if ((( YTD == ptype ) && ( !dprev.isValid() )) ||
                (( dprev.isValid() ) && ( dprev.month() != d.month() )))
            {
                if ( 1 == d.month() )
                    label = d.toString( "MMM yy" );
                else
                    label = d.toString( "MMM" );
            }

            dprev = d;
        }
        // minute
        else if ( MINUTE == ftype )
        {
            const QDate d( cd.stamp.date() );
            const QTime t( cd.stamp.time() );

            if (( dprev.isValid() ) && ( dprev != d ))
                label = cd.stamp.toString( "ddd" );
            else if (( 1 == f ) && (( 15 == t.minute() ) || ( 45 == t.minute() ) || ( 30 == t.minute() )))
                label = t.toString( "HH:mm" );
            else if ( 0 == t.minute() )
            {
                if ((( 0 == (t.hour() % 4) ) && ( f <= 30 )) ||
                    (( 0 == (t.hour() % 2) ) && ( f <= 10 )) ||
                    ( f <= 5 ))
                {
                    label = t.toString( "HH:mm" );
                }
            }

            dprev = d;
        }

        // draw label
        if ( label.length() )
        {
            bool overlap( true );

            // update location so labels do not overlap
            if ( xfooter < x )
            {
                xfooter = x + fm.boundingRect( label ).width();
                overlap = false;
            }

            // draw for each graph (upper and lower)
            QList<int> bottoms;
            bottoms.append( gbottom );

            if ( lowerExists )
                bottoms.append( lbottom );

            painter.setPen( QPen( Qt::darkGray, 0 ) );

            foreach ( int bottom, bottoms )
            {
                const QRectF lrect(
                    QPointF( x+1, bottom ),
                    QPointF( x+2, bottom+2 ) );

                painter.drawRect( lrect );

                if ( !overlap )
                    painter.drawText( x, bottom+4, 50, marginHeight-SPACING, Qt::AlignLeft | Qt::AlignTop, label );
            }
        }

        // overlays
        const QAbstractItemModel *model( overlays_->model() );
        const QModelIndexList indexes( model->match( model->index( 1, 0 ), Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly ) );

        foreach ( const QModelIndex& index, indexes )
        {
            const QString data( model->data( index, Qt::UserRole ).toString() );
            const QVariant c( model->data( index, Qt::ForegroundRole ) );

            if ( 0 < idx )
            {
#if QT_VERSION_CHECK( 5, 15, 2 ) <= QT_VERSION
                const int d( QStringView{ data }.mid( 3 ).toInt() );
#else
                const int d( data.midRef( 3 ).toInt() );
#endif
                const bool macdVal(( 12 == d ) || ( 26 == d ));

                double pval;
                double val;

                if (( data.startsWith( "SMA" ) ) && ( maOverlayValid ))
                {
                    const int idxo( idx + (ma_.size() - candles_.size()) );

                    if ( idxo < 1 )
                        continue;
                    else if (( !ma_[idxo-1].sma.contains( d ) ) || ( !ma_[idxo].sma.contains( d ) ))
                        continue;

                    pval = ma_[idxo-1].sma[d];
                    val = ma_[idxo].sma[d];
                }
                else if (( data.startsWith( "EMA" ) ) && ( macdVal ) && ( macdOverlayValid ))
                {
                    const int idxo( idx + (macd_.size() - candles_.size()) );

                    if ( idxo < 1 )
                        continue;
                    else if (( !macd_[idxo-1].ema.contains( d ) ) || ( !macd_[idxo].ema.contains( d ) ))
                        continue;

                    pval = macd_[idxo-1].ema[d];
                    val = macd_[idxo].ema[d];
                }
                else if (( data.startsWith( "EMA" ) ) && ( !macdVal ) && ( maOverlayValid ))
                {
                    const int idxo( idx + (ma_.size() - candles_.size()) );

                    if ( idxo < 1 )
                        continue;
                    else if (( !ma_[idxo-1].ema.contains( d ) ) || ( !ma_[idxo].ema.contains( d ) ))
                        continue;

                    pval = ma_[idxo-1].ema[d];
                    val = ma_[idxo].ema[d];
                }
                else
                {
                    continue;
                }

                painter.setPen( QPen( c.value<QBrush>(), 2 ) );

                painter.drawLine(
                    QPoint( x+2-cwidth, gbottom - scaled( pval, gmin, gmax, gbottom ) ),
                    QPoint( x+2,        gbottom - scaled(  val, gmin, gmax, gbottom ) ) );
            }
        }

        // next candle
        x += cwidth;
    }

    // -----
    // lower
    // -----

    int lheight( 0 );
    int lwidth( 0 );

    if ( lowerExists )
    {
        // graph constants
        lheight = (gheight / 4) - ((marginHeight * 3) / 2);
        lwidth = gwidth;

        // value intervals
        painter.setPen( QPen( Qt::darkGray, 0 ) );

        for ( double i( lmin ); i <= lmax; i += linterval )
        {
            const int y( lbottom - scaled( i, lmin, lmax, lheight ) );

            painter.drawLine( 0, y, lwidth, y );
        }

        // draw values
        const QString data( lowers_->currentData().toString() );

        painter.setPen( QPen( palette().windowText(), 2 ) );

        int x( coffset );

        for ( int idx( 0 ); idx < candles_.size(); ++idx )
        {
            // HV
            if ( hvLowerValid )
            {
                const int idxl( idx + (hv_.size() - candles_.size()) );
#if QT_VERSION_CHECK( 5, 15, 2 ) <= QT_VERSION
                const int d( QStringView{ data }.mid( 2 ).toInt() );
#else
                const int d( data.midRef( 2 ).toInt() );
#endif

                if (( 0 < idxl ) &&
                    ( hv_[idxl-1].volatilities.contains( d ) ) &&
                    ( hv_[idxl].volatilities.contains( d ) ))
                {
                    const double pval( 100.0 * hv_[idxl-1].volatilities[d] );
                    const double val( 100.0 * hv_[idxl].volatilities[d] );

                    painter.drawLine(
                        QPoint( x+2-cwidth, lbottom - scaled( pval, lmin, lmax, lheight ) ),
                        QPoint( x+2,        lbottom - scaled(  val, lmin, lmax, lheight ) ) );
                }
            }

            // MACD
            else if ( macdLowerValid )
            {
                const int idxl( idx + (macd_.size() - candles_.size()) );

                if ( 0 <= idxl )
                {
                    const MovingAveragesConvergenceDivergence macd( macd_[idxl] );

                    const QColor fill( 0.0 <= macd.histogram ? Qt::darkGreen : Qt::red );

                    // histogram
                    const QRectF hrect(
                        QPointF( x-coffset,   lbottom - scaled( 0.0,            lmin, lmax, lheight ) ),
                        QPointF( x+coffset+3, lbottom - scaled( macd.histogram, lmin, lmax, lheight ) ) );

                    painter.setPen( QPen( fill, 0 ) );
                    painter.setBrush( QBrush( fill ) );
                    painter.drawRect( hrect );

                    if ( 0 < idxl )
                    {
                        // MACD signal
                        painter.drawLine(
                            QPoint( x+2-cwidth, lbottom - scaled( macd_[idxl-1].signal, lmin, lmax, lheight ) ),
                            QPoint( x+2,        lbottom - scaled( macd.signal,          lmin, lmax, lheight ) ) );

                        // MACD
                        painter.setPen( QPen( palette().windowText(), 2 ) );
                        painter.drawLine(
                            QPoint( x+2-cwidth, lbottom - scaled( macd_[idxl-1].macd, lmin, lmax, lheight ) ),
                            QPoint( x+2,        lbottom - scaled( macd.macd,          lmin, lmax, lheight ) ) );
                    }
                }
            }

            // RSI
            else if ( rsiLowerValid )
            {
                const int idxl( idx + (rsi_.size() - candles_.size()) );
#if QT_VERSION_CHECK( 5, 15, 2 ) <= QT_VERSION
                const int d( QStringView{ data }.mid( 3 ).toInt() );
#else
                const int d( data.midRef( 3 ).toInt() );
#endif

                if (( 0 < idxl ) &&
                    ( rsi_[idxl-1].values.contains( d ) ) &&
                    ( rsi_[idxl].values.contains( d ) ))
                {
                    const double pval( rsi_[idxl-1].values[d] );
                    const double val( rsi_[idxl].values[d] );

                    painter.drawLine(
                        QPoint( x+2-cwidth, lbottom - scaled( pval, lmin, lmax, lheight ) ),
                        QPoint( x+2,        lbottom - scaled(  val, lmin, lmax, lheight ) ) );
                }
            }

            // next value
            x += cwidth;
        }
    }

    painter.end();

    // ------
    // margin
    // ------

    margin_ = QPixmap( marginWidth, gheight );
    margin_.fill( palette().base().color() );

    painter.begin( &margin_ );

    // price intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );
    painter.drawLine( 0, 0, 0, gbottom );

    for ( double i( gmin ); i <= gmax; i += ginterval )
    {
        const int y( gbottom - scaled( i, gmin, gmax, gbottom ) );

        painter.drawLine( 0, y, 2, y );
        painter.drawText( 4, y-25, marginWidth-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( i, 'f', numDecimalPlaces ) );
    }

    // lower value intervals
    if ( lowerExists )
    {
        painter.drawLine( 0, lbottom, 0, lbottom - lheight );

        for ( double i( lmin ); i <= lmax; i += linterval )
        {
            const int y( lbottom - scaled( i, lmin, lmax, lheight ) );

            painter.drawLine( 0, y, 2, y );
            painter.drawText( 4, y-25, marginWidth-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( i, 'f', numDecimalPlaces ) );
        }
    }

    painter.end();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int SymbolPriceHistoryWidget::scaled( double p, double min, double max, int height )
{
    return std::round( ((p - min) / (max - min)) * height );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::translateOverlays( QComboBox *w )
{
    QAbstractItemModel *model( w->model() );

    model->setData( model->index( 0, 0 ), tr( "OVERLAYS" ), Qt::DisplayRole );

    for ( int i( 1 ); i < model->rowCount(); ++i )
    {
        const QString data( model->data( model->index( i, 0 ), Qt::UserRole ).toString() );

        QString text;

        if ( data.startsWith( "SMA" ) )
            text = tr( "SMA(%0)" ).arg( QStringView{ data }.mid( 3 ) );
        else if ( data.startsWith( "EMA" ) )
            text = tr( "EMA(%0)" ).arg( QStringView{ data }.mid( 3 ) );

        model->setData( model->index( i, 0 ), text, Qt::DisplayRole );
        model->setData( model->index( i, 0 ), overlayColor( data ), Qt::ForegroundRole );
    }

    // adjust view width to fit contents
    // add room for check box
    w->view()->setMinimumWidth(
        24 + w->view()->sizeHintForColumn( 0 ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::translateLowers( QComboBox *w )
{
    for ( int i( 0 ); i < w->count(); ++i )
    {
        const QString data( w->itemData( i ).toString() );

        QString text;

        if ( "NONE" == data )
            text = tr( "LOWERS" );
        else if ( "MACD" == data )
            text = tr( "MACD" );
        else if ( data.startsWith( "RSI" ) )
            text = tr( "RSI(%0)" ).arg( QStringView{ data }.mid( 3 ) );
        else if ( data.startsWith( "HV" ) )
            text = tr( "HV(%0)" ).arg( QStringView{ data }.mid( 2 ) );

        w->setItemText( i, text );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QColor SymbolPriceHistoryWidget::overlayColor( const QString& desc )
{
    QCryptographicHash h( QCryptographicHash::Md5 );
    h.addData( desc.toLatin1() );

    const QByteArray a( h.result() );

    return QColor( (unsigned char) a[0], (unsigned char) a[1], (unsigned char) a[2] );
}

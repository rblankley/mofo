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

#include <cmath>

#include <QComboBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>

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
    connect( AppDatabase::instance(), &AppDatabase::candleDataChanged, this, &_Myt::onCandleDataChanged, Qt::QueuedConnection );

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
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::refreshData()
{
    int p, f;
    QString ptype, ftype;

    // retrieve period and frequency
    if (( currentPeriod( p, ptype ) ) && ( currentFrequency( f, ftype ) ))
    {
        // clear graph
        candles_.clear();
        graph_ = margin_ = QPixmap();

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
        const int offset( scrollBarVisible() ? scroll_->value() : graph_.width() - width() );

        painter.drawPixmap( 0 - offset, 0, graph_ );
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

    static const int FOOTER( 25 );
    static const double MIN_INTERVAL_HEIGHT( 50 );

    // check symbol
    if ( s != symbol() )
        return;

    // check period and freq
    int p, f;
    QString ptype, ftype;

    if (( currentPeriod( p, ptype ) ) && ( period == p ) && ( periodType == ptype ) &&
        ( currentFrequency( f, ftype ) ) && ( freq == f ) && ( freqType == ftype ))
    {
        const QFontMetrics fm( font() );

        // set candles
        candles_ = candles;

        // determine graph min/max values
        double min( 999999.99 );
        double max( 0.0 );

        vmax_ = 0;

        foreach ( const CandleData& cd, candles_ )
        {
            min = qMin( min, cd.lowPrice );
            max = qMax( max, cd.highPrice );

            vmax_ = qMax( vmax_, cd.totalVolume );
        }

        // determine price interval
        QList<double> intervals;
        intervals.append( 1.0 );
        intervals.append( 2.0 );
        intervals.append( 5.0 );

        double interval( 0.0 );
        double mult( 0.0001 );

        do
        {
            foreach ( double val, intervals )
            {
                const double i( val*mult );
                const double h( (height() - FOOTER) / ((max - min) / i) );

                if ( MIN_INTERVAL_HEIGHT <= h )
                {
                    interval = i;
                    break;
                }
            }

            mult *= 10.0;

        } while ( interval <= 0.0 );

        // number of decimal places
        numDecimalPlaces_ = 2;

        if ( interval < 0.0009 )
            numDecimalPlaces_ = 4;
        else if ( interval < 0.009 )
            numDecimalPlaces_ = 3;

        // graph constants
        gmin_ = interval * std::floor( min / interval );
        gmax_ = interval * std::ceil( max / interval );
        ginterval_ = interval;

        marginWidth_ = SPACING + fm.boundingRect( QString::number( gmax_, 'f', numDecimalPlaces_ ) ).width();
        marginHeight_ = SPACING + fm.boundingRect( "0123456789/:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ).height();

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

    connect( scroll_, &QScrollBar::valueChanged, this, &_Myt::onValueChanged );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::createLayout()
{
    QHBoxLayout *boxes( new QHBoxLayout() );
    boxes->addWidget( period_ );
    boxes->addWidget( freqMin_ );
    boxes->addWidget( freqDayWeek_ );
    boxes->addWidget( freqDayWeekMonth_ );
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

    return qMax( 0, (candlesWidth_ + marginWidth_) - width() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolPriceHistoryWidget::drawGraph()
{
    int footer( marginHeight_ );

    // no data
    if ( candles_.isEmpty() )
        return;

    // check period and frequency (for labels)
    int p, f;
    QString ptype, ftype;

    if (( !currentPeriod( p, ptype ) ) || ( !currentFrequency( f, ftype ) ))
        return;

    // compute candles width
    int cwidth( MIN_CANDLE_WIDTH + 2 );

    while ( (marginWidth_ + (cwidth * candles_.size())) < width() )
        cwidth += 2;

    cwidth -= 2;

    candlesWidth_ = cwidth * candles_.size();

    // check scroll bar visible
    if ( scrollBarVisible() )
    {
        scroll_->setMaximum( scrollBarMaximum() );
        scroll_->setPageStep( candlesWidth_ );
        scroll_->setSingleStep( cwidth );
        scroll_->show();

        footer += scroll_->height();
    }
    else
    {
        scroll_->hide();
    }

    QPainter painter;

    // -----
    // graph
    // -----

    const int gwidth( candlesWidth_ + marginWidth_ );               // px
    const int gheight( height() );                                  // px

    const int gbottom( gheight - footer );                          // px
    const int gright( candlesWidth_ );                              // px

    graph_ = QPixmap( gwidth, gheight );
    graph_.fill( palette().base().color() );

    painter.begin( &graph_ );

    // price intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );

    for ( double i( gmin_ ); i <= gmax_; i += ginterval_ )
    {
        const int y( gbottom - scaled( i, gmin_, gmax_, gbottom ) );

        painter.drawLine( 0, y, gright, y );
    }

    // candles
    const int coffset( (cwidth - MIN_CANDLE_WIDTH) / 2 );

    QDate dprev;

    int x( coffset );
    int xfooter( -1 );

    foreach ( const CandleData& cd, candles_ )
    {
        const QColor fill( (cd.openPrice <= cd.closePrice) ? Qt::darkGreen : Qt::red );

        // total volume
        const QRectF vrect(
            QPointF( x+1, gbottom ),
            QPointF( x+2, gbottom - scaled( cd.totalVolume, 0.0, 2.0*vmax_, gbottom ) ) );

        painter.setPen( QPen( Qt::darkGray, 0 ) );
        painter.setBrush( QBrush( Qt::darkGray ) );
        painter.drawRect( vrect );

        // high/low price
        const QRectF hlrect(
            QPointF( x+1, gbottom - scaled( cd.lowPrice, gmin_, gmax_, gbottom ) ),
            QPointF( x+2, gbottom - scaled( cd.highPrice, gmin_, gmax_, gbottom ) ) );

        painter.setPen( QPen( fill, 0 ) );
        painter.setBrush( QBrush( fill ) );
        painter.drawRect( hlrect );

        // open/close price
        const QRectF ocrect(
            QPointF( x-coffset, gbottom - scaled( cd.openPrice, gmin_, gmax_, gbottom ) ),
            QPointF( x+coffset+3, gbottom - scaled( cd.closePrice, gmin_, gmax_, gbottom ) ) );

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

        if ( label.length() )
        {
            // draw label
            painter.setPen( QPen( Qt::darkGray, 0 ) );

            const QRectF lrect(
                QPointF( x+1, gbottom ),
                QPointF( x+2, gbottom+2 ) );

            painter.drawRect( lrect );

            if ( xfooter < x )
            {
                // update location to labels do not overlap
                const QFontMetrics fm( painter.fontMetrics() );

                xfooter = x + fm.boundingRect( label ).width();

                painter.drawText( x, gbottom+4, 50, marginHeight_-SPACING, Qt::AlignLeft | Qt::AlignTop, label );
            }
        }

        // next candle
        x += cwidth;
    }

    painter.end();

    // ------
    // margin
    // ------

    margin_ = QPixmap( marginWidth_, gheight );
    margin_.fill( palette().base().color() );

    painter.begin( &margin_ );

    // price intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );
    painter.drawLine( 0, 0, 0, gbottom );

    for ( double i( gmin_ ); i <= gmax_; i += ginterval_ )
    {
        const int y( gbottom - scaled( i, gmin_, gmax_, gbottom ) );

        painter.drawLine( 0, y, 2, y );
        painter.drawText( 4, y-25, marginWidth_-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( i, 'f', numDecimalPlaces_ ) );
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

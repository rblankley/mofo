/**
 * @file riskfreeinterestrateswidget.cpp
 *
 * @copyright Copyright (C) 2022 Randy Blankley. All rights reserved.
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
#include "riskfreeinterestrateswidget.h"

#include "db/appdb.h"

#include <cmath>

#include <QComboBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>

const QString RiskFreeInterestRatesWidget::STATE_GROUP_NAME( "riskFreeInterestRates" );
const QString RiskFreeInterestRatesWidget::STATE_NAME( "[[default]]" );

static const QString DAY( "day" );
static const QString WEEK( "week" );
static const QString MONTH( "month" );
static const QString YEAR( "year" );

///////////////////////////////////////////////////////////////////////////////////////////////////
RiskFreeInterestRatesWidget::RiskFreeInterestRatesWidget( QWidget *parent ) :
    _Mybase( parent ),
    init_( false ),
    scrollVisible_( false )
{
    // init
    initialize();
    createLayout();
    translate();

    // restore state
    const QString state( AppDatabase::instance()->widgetState( AppDatabase::PriceHistory, STATE_GROUP_NAME, STATE_NAME ) );

    if ( state.length() )
    {
        const QStringList settings( state.split( '/' ) );

        if ( 2 != settings.size() )
            LOG_WARN << "bad state " << qPrintable( state );
        else
        {
            const int termIndex( term_->findData( settings[0] ) );
            const int periodIndex( period_->findData( settings[1] ) );

            if (( termIndex < 0 ) || ( periodIndex < 0 ))
                LOG_WARN << "unknown index " << termIndex << " " << periodIndex;
            else
            {
                term_->setCurrentIndex( termIndex );
                period_->setCurrentIndex( periodIndex );
            }
        }
    }

    init_ = true;

    // refresh history
    refreshData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
RiskFreeInterestRatesWidget::~RiskFreeInterestRatesWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesWidget::translate()
{
    term_->setItemText( 0, tr( "1M" ) );
    term_->setItemText( 1, tr( "3M" ) );
    term_->setItemText( 2, tr( "6M" ) );
    term_->setItemText( 3, tr( "1Y" ) );
    term_->setItemText( 4, tr( "2Y" ) );
    term_->setItemText( 5, tr( "3Y" ) );
    term_->setItemText( 6, tr( "5Y" ) );
    term_->setItemText( 7, tr( "7Y" ) );
    term_->setItemText( 8, tr( "10Y" ) );
    term_->setItemText( 9, tr( "20Y" ) );
    term_->setItemText( 10, tr( "30Y" ) );

    period_->setItemText( 0, tr( "Daily" ) );
    period_->setItemText( 1, tr( "Weekly" ) );
    period_->setItemText( 2, tr( "Monthly" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesWidget::refreshData()
{
    // check we are initialized
    if ( !init_ )
        return;

    // reset graph
    graph_ = QPixmap();

    // fetch data
    const QString val( term_->currentData().toString() );

    if ( !rates_.contains( val ) )
    {
        // determine term
        const QStringList vals( val.split( ":" ) );

        if ( 2 != vals.size() )
        {
            LOG_WARN << "bad term " << qPrintable( val );
            return;
        }

        double mult( 1.0 );

        if ( MONTH == vals[1] )
            mult /= 12.0;

        const double term( mult * vals[0].toInt() );

        // fetch data
        AppDatabase::RiskFreeRatesMap rates;

        AppDatabase::instance()->riskFreeRates( term, rates );

        if ( rates.isEmpty() )
        {
            LOG_WARN << "no rates for " << term;
            return;
        }

        // multiply all rates by 100, we want to show as a percentage
        for ( AppDatabase::RiskFreeRatesMap::iterator rate( rates.begin() ); rate != rates.end(); ++rate )
            rate.value() *= 100.0;

        LOG_TRACE << "have rates for " << qPrintable( val );
        rates_[val] = rates;
    }

    // draw
    drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesWidget::paintEvent( QPaintEvent *e )
{
    QPainter painter;
    painter.begin( this );

    // fill background color
    painter.fillRect( rect(), palette().base().color() );

    // graph
    if ( !graph_.isNull() )
        painter.drawPixmap( 0, 0, graph_ );

    painter.end();

    _Mybase::paintEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesWidget::resizeEvent( QResizeEvent *e )
{
    const bool visible( scrollBarVisible() );

    // new graph
    drawGraph();

    // when the scroll bar shows up from resize this must be the first size
    // event from dialog going visible
    // default scroll to far right
    if ( visible != scrollBarVisible() )
        scroll_->setValue( scroll_->maximum() );

    _Mybase::resizeEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesWidget::onCurrentIndexChanged( int index )
{
    Q_UNUSED( index )

    // reset scroll bar
    scroll_->setValue( 0 );

    // refresh data (and draw graph)
    refreshData();

    // default scroll to far right
    if ( scrollBarVisible() )
        scroll_->setValue( scroll_->maximum() );

    // save state
    const QString state(
        term_->currentData().toString() + "/" +
        period_->currentData().toString() );

    LOG_TRACE << "save state " << qPrintable( state );

    AppDatabase::instance()->setWidgetState( AppDatabase::PriceHistory, STATE_GROUP_NAME, STATE_NAME, state.toLatin1() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesWidget::onValueChanged( int value )
{
    Q_UNUSED( value )

    drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesWidget::initialize()
{
    term_ = new QComboBox( this );
    term_->addItem( QString(), "1:" + MONTH );
    term_->addItem( QString(), "3:" + MONTH );
    term_->addItem( QString(), "6:" + MONTH );
    term_->addItem( QString(), "1:" + YEAR );
    term_->addItem( QString(), "2:" + YEAR );
    term_->addItem( QString(), "3:" + YEAR );
    term_->addItem( QString(), "5:" + YEAR );
    term_->addItem( QString(), "7:" + YEAR );
    term_->addItem( QString(), "10:" + YEAR );
    term_->addItem( QString(), "20:" + YEAR );
    term_->addItem( QString(), "30:" + YEAR );

    period_ = new QComboBox( this );
    period_->addItem( QString(), DAY );
    period_->addItem( QString(), WEEK );
    period_->addItem( QString(), MONTH );

    scroll_ = new QScrollBar( Qt::Horizontal, this );
    scroll_->hide();

    // connect
    connect( term_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );
    connect( period_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );

    connect( scroll_, &QScrollBar::valueChanged, this, &_Myt::onValueChanged );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesWidget::createLayout()
{
    QHBoxLayout *boxes( new QHBoxLayout() );
    boxes->addStretch();
    boxes->addWidget( term_ );
    boxes->addWidget( period_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addLayout( boxes );
    form->addStretch();
    form->addWidget( scroll_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool RiskFreeInterestRatesWidget::calcMinMaxValues( const ValuesMap& values, QDate& kmin, QDate& kmax, double& vmin, double& vmax ) const
{
    vmin = 999999.99;
    vmax = -999999.99;

    for ( ValuesMap::const_iterator i( values.constBegin() ); i != values.constEnd(); ++i )
    {
        if ( kmin.isValid() )
        {
            kmin = qMin( kmin, i.key() );
            kmax = qMax( kmax, i.key() );
        }
        else
        {
            kmin = kmax = i.key();
        }

        vmin = qMin( vmin, i.value() );
        vmax = qMax( vmax, i.value() );
    }

    return (kmin <= kmax);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesWidget::calcIntervalValues( double min, double max, double gheight, double ints, double& interval, int& numDecimals ) const
{
    static const int FOOTER( 25 );
    static const double MAX_MULT( 1000.0 );

    // determine graph interval
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

            // check this interval height is smaller than requested interval size
            if (( ints <= h )  || ( MAX_MULT <= mult ))
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
void RiskFreeInterestRatesWidget::drawGraph()
{
    const QString val( term_->currentData().toString() );
    const QString p( period_->currentData().toString() );

    // no data
    if ( !rates_.contains( val ) )
    {
        LOG_WARN << "no rates for term " << qPrintable( val );
        return;
    }

    const ValuesMap& m( rates_[val] );

    if ( m.isEmpty() )
    {
        LOG_WARN << "no rate data for term " << qPrintable( val );
        return;
    }

    // determine what we are graphing
    ValuesMap values;

    if ( DAY == p )
        values = m;
    else
    {
        int prevWeek( m.constBegin().key().weekNumber() );
        int prevMonth( m.constBegin().key().month() );

        ValuesMap::const_iterator vprev;

        for ( ValuesMap::const_iterator v( m.constBegin() ); v != m.constEnd(); ++v )
        {
            const int week( v.key().weekNumber() );
            const int month( v.key().month() );

            // graph last value of each week
            //  - or -
            // graph last value of each month
            if ((( WEEK == p ) && ( prevWeek != week )) ||
                (( MONTH == p ) && ( prevMonth != month )))
            {
                values[vprev.key()] = vprev.value();

                prevWeek = week;
                prevMonth = month;
            }

            vprev = v;
        }
    }

    LOG_TRACE << "have " << values.size() << " rates to graph";

    // height to small
    if ( height() < 128 )
        return;

    // determine min/max values
    // x axis = dates
    // y axis = interest rate
    QDate xmin;
    QDate xmax;

    double ymin;
    double ymax;

    calcMinMaxValues( values, xmin, xmax, ymin, ymax );

    const QFontMetrics fm( fontMetrics() );

    // determine intervals
    double yinterval;
    int numDecimalPlacesInterestRate;

    calcIntervalValues( ymin, ymax, height(), 50.0, yinterval, numDecimalPlacesInterestRate );

    int numValues( values.size() );

    if ( DAY == p )
        numValues = xmin.daysTo( xmax ) + 1;

    // graph constants
    ymin = yinterval * std::floor( ymin / yinterval );
    ymax = yinterval * std::ceil( ymax / yinterval );

    const int marginWidth( SPACING + fm.boundingRect( QString::number( ymax, 'f', numDecimalPlacesInterestRate ) ).width() );
    const int marginHeight( SPACING + fm.boundingRect( "0123456789/:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ).height() );

    // -----
    // graph
    // -----

    int rateWidth( (DAY == p) ? MIN_RATE_WIDTH_DAY : MIN_RATE_WIDTH );

    // increase width of date if graph is smaller than screen (to fit better)
    while ( (marginWidth + ((rateWidth+2) * numValues) + SPACING) <= width() )
        rateWidth += 2;

    int gwidth( qMax( width(), marginWidth + (rateWidth * numValues) + SPACING ) );
    int gheight( height() );

    int gtop( SPACING );
    int gleft( marginWidth );
    int gbottom( gheight - marginHeight );
    int gright( marginWidth + (rateWidth * numValues) );

    QPainter painter;

    // check scroll bar visible
    const int smax( qMax( 0, gwidth - width() ) );

    if ( 0 < smax )
    {
        scrollVisible_ = true;

        scroll_->setRange( 0, smax );
        scroll_->setPageStep( gwidth );
        scroll_->setSingleStep( rateWidth * ((DAY == p) ? 7 : 4) ); // step by week or month/quarter
        scroll_->show();

        // FIXME
        // scroll height is unreliable... you do not get a "true" height until the dialog is
        // shown and resized... which causes some whitespace on the very first graph
        gbottom -= scroll_->height();
    }
    else
    {
        scrollVisible_ = false;

        scroll_->hide();
    }

    // -----
    // graph
    // -----

    QPixmap pixmap( QPixmap( gwidth, gheight ) );
    pixmap.fill( palette().base().color() );

    painter.begin( &pixmap );

    // y axis
    // rate intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );

    for ( double i( ymin ); i <= ymax; i += yinterval )
    {
        const int y( gbottom - scaled( i, ymin, ymax, (gbottom - gtop) ) );

        painter.drawLine( gleft, y, gright, y );
    }

    // rates
    int xprev( -1 );
    int yprev( -1 );

    int idx( 0 );

    int month( values.constBegin().key().month() );
    int year( values.constBegin().key().year() );

    for ( ValuesMap::const_iterator v( values.constBegin() ); v != values.constEnd(); ++v )
    {
        const int y( gbottom - scaled( v.value(), ymin, ymax, (gbottom - gtop) ) );

        int x( gleft + rateWidth/2 + 1 );

        if ( DAY == p )
            x += rateWidth * xmin.daysTo( v.key() );
        else
            x += rateWidth * idx;

        // rates
        if ( 0 <= xprev )
        {
            painter.setPen( QPen( palette().windowText(), 2 ) );
            painter.drawLine( xprev, yprev, x, y );
        }

        // axis label
        QString label;

        if ( MONTH == p )
        {
            // every year
            if ( year != v.key().year() )
                label = v.key().toString( "yyyy" );
        }
        else
        {
            // every month
            if ( month != v.key().month() )
            {
                if (( WEEK == p ) && ( 0 < ((v.key().month()-1) % 3) ))
                    ; // every quarter for weekly period
                else if ( year != v.key().year() )
                    label = v.key().toString( "MMM yyyy" );
                else
                    label = v.key().toString( "MMM" );
            }
        }

        if ( !label.isEmpty() )
        {
            const int xloc( x - rateWidth/2 + 1 );

            painter.setPen( QPen( Qt::darkGray, 0 ) );
            painter.drawLine( xloc, gbottom, xloc, gbottom+2 );
            painter.drawText( xloc, gbottom+4, 50, marginHeight-SPACING, Qt::AlignLeft | Qt::AlignTop, label );
        }

        xprev = x;
        yprev = y;
        ++idx;

        month = v.key().month();
        year = v.key().year();
    }

    painter.end();

    // ------
    // widget
    // ------

    graph_ = QPixmap( width(), height() );
    graph_.fill( palette().base().color() );

    painter.begin( &graph_ );

    // copy graph to widget
    if ( scrollVisible_ )
        painter.drawPixmap( 0, 0, pixmap, scroll_->value(), 0, width(), gheight );
    else
        painter.drawPixmap( 0, 0, pixmap );

    // y axis
    // interest rates
    painter.setPen( palette().base().color() );
    painter.setBrush( palette().base().color() );
    painter.drawRect( 0, 0, gleft, height() );

    painter.setPen( QPen( Qt::darkGray, 0 ) );
    painter.drawLine( gleft, gtop, gleft, gbottom );

    for ( double i( ymin ); i <= ymax; i += yinterval )
    {
        const int y( gbottom - scaled( i, ymin, ymax, (gbottom - gtop) ) );

        painter.drawLine( gleft, y, gleft-2, y );
        painter.drawText( 4, y-25, marginWidth-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( i, 'f', numDecimalPlacesInterestRate ) );
    }

    painter.end();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int RiskFreeInterestRatesWidget::scaled( double p, double min, double max, int height )
{
    return std::round( ((p - min) / (max - min)) * height );
}

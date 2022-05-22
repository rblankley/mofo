/**
 * @file optionchainopenintwidget.cpp
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
#include "optionchainopenintwidget.h"

#include "db/appdb.h"
#include "db/symboldbs.h"

#include <cmath>

#include <QAbstractItemView>
#include <QComboBox>
#include <QCryptographicHash>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QToolButton>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionChainOpenInterestWidget::OptionChainOpenInterestWidget( const QString& underlying, double underlyingPrice, const QDate& expiryDate, const QDateTime& stamp, QWidget *parent ) :
    _Mybase( parent ),
    underlying_( underlying ),
    price_( underlyingPrice ),
    end_( stamp ),
    expiryDate_( expiryDate ),
    zoom_( 0 )
{
    // init
    initialize();
    createLayout();
    translate();

    // refresh
    refreshData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionChainOpenInterestWidget::~OptionChainOpenInterestWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainOpenInterestWidget::translate()
{
    zout_->setText( "-" );
    zout_->setToolTip( tr( "Zoom Out" ) );

    zin_->setText( "+" );
    zin_->setToolTip( tr( "Zoom In" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainOpenInterestWidget::refreshData()
{
    // check expiry date
    if ( !expiryDate_.isValid() )
    {
        LOG_WARN << "missing expiry date";
        return;
    }

    // fetch open interest
    openInt_.callOpenInterest.clear();
    openInt_.putOpenInterest.clear();

    LOG_TRACE << "fetch open interest...";
    stamp_ = SymbolDatabases::instance()->optionChainOpenInterest( underlying(), expiryDate_, openInt_, QDateTime(), end_ );

    if ( !haveCurveData() )
    {
        LOG_WARN << "no open interest data for " << qPrintable( expiryDate_.toString() );
        return;
    }

    // determine step between strike prices
    const QList<double> callStrikePrices( openInt_.callOpenInterest.keys() );

    double diff( 999999.99 );

    for ( int i( callStrikePrices.size() ); 0 < --i; )
        diff = qMin( diff, std::abs( callStrikePrices[i] - callStrikePrices[i-1] ) );

    multiplier_ = 1000.00;
    step_ = std::round( diff * multiplier_ );

    do
    {
        bool valid( true );

        foreach ( const double& strike, callStrikePrices )
            valid &= (0 == ((int)(std::round( strike * multiplier_ )) % step_));

        if ( valid )
            break;

        --step_;

    } while ( 0 < step_ );

    if ( step_ <= 0 )
    {
        LOG_WARN << "could not compute step for strike prices " << qPrintable( expiryDate_.toString() );
        return;
    }

    // draw!
    drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainOpenInterestWidget::paintEvent( QPaintEvent *e )
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
void OptionChainOpenInterestWidget::resizeEvent( QResizeEvent *e )
{
    // new graph
    drawGraph();

    _Mybase::resizeEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainOpenInterestWidget::onButtonClicked()
{
    if ( zout_ == sender() )
    {
        zoom_ = qMin( ++zoom_, MAX_ZOOM );
        drawGraph();
    }
    else if ( zin_ == sender() )
    {
        zoom_ = qMax( --zoom_, MIN_ZOOM );
        drawGraph();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainOpenInterestWidget::onValueChanged( int value )
{
    Q_UNUSED( value )

    drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainOpenInterestWidget::initialize()
{
    // zoom out
    zout_ = new QToolButton( this );

    connect( zout_, &QToolButton::clicked, this, &_Myt::onButtonClicked );

    // zoom in
    zin_ = new QToolButton( this );

    connect( zin_, &QToolButton::clicked, this, &_Myt::onButtonClicked );

    // scroll bar
    scroll_ = new QScrollBar( Qt::Horizontal, this );
    scroll_->hide();

    connect( scroll_, &QScrollBar::valueChanged, this, &_Myt::onValueChanged );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainOpenInterestWidget::createLayout()
{
    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->addWidget( zout_ );
    buttons->addWidget( zin_ );
    buttons->addStretch();

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addLayout( buttons );
    form->addStretch();
    form->addWidget( scroll_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionChainOpenInterestWidget::haveCurveData() const
{
    return (( !openInt_.callOpenInterest.isEmpty() ) && ( !openInt_.putOpenInterest.isEmpty() ));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionChainOpenInterestWidget::calcMinMaxValues( const ValuesMap& values, double& kmin, double& kmax, int& vmin, int& vmax ) const
{
    kmin = 999999.99;
    kmax = 0.0;

    vmin = 999999999;
    vmax = 0;

    for ( ValuesMap::const_iterator i( values.constBegin() ); i != values.constEnd(); ++i )
    {
        kmin = qMin( kmin, i.key() );
        kmax = qMax( kmax, i.key() );

        vmin = qMin( vmin, i.value() );
        vmax = qMax( vmax, i.value() );
    }

    return (kmin <= kmax);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainOpenInterestWidget::calcIntervalValues( double min, double max, double gheight, double ints, double& interval, int& numDecimals ) const
{
    const int FOOTER( 25 );

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

            // check this interval height is smaller than requested interval size
            if ( ints <= h )
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
void OptionChainOpenInterestWidget::drawGraph()
{
    // clear graph
    graph_ = QPixmap();

    // no data
    if ( !haveCurveData() )
    {
        graph_ = QPixmap( size() );
        graph_.fill( palette().base().color() );

        QPainter painter( &graph_ );
        painter.setPen( QPen( palette().text().color(), 0 ) );
        painter.drawText( 0, 0, width(), height(), Qt::AlignCenter, tr( "No data to display, run analysis on this underlying" ) );

        return;
    }

    // height to small
    if ( height() < 128 )
        return;

    // determine min/max values
    // x axis = strike prices
    // y axit = implied volatility
    double xmin;
    double xmax;

    double ymin = xmin = 999999999.99;
    double ymax = xmax = 0.0;

    QList<const ValuesMap*> valuesList;
    valuesList.append( &openInt_.callOpenInterest );
    valuesList.append( &openInt_.putOpenInterest );

    foreach ( const ValuesMap *values, valuesList )
    {
        double kmin;
        double kmax;
        int vmin;
        int vmax;

        if ( calcMinMaxValues( (*values), kmin, kmax, vmin, vmax ) )
        {
            xmin = qMin( xmin, kmin );
            xmax = qMax( xmax, kmax );

            ymin = qMin( ymin, (double) vmin );
            ymax = qMax( ymax, (double) vmax );
        }
    }

    if ( xmax < xmin )
    {
        LOG_WARN << "invalid coordinates";
        return;
    }

    const QFontMetrics fm( fontMetrics() );

    // determine bar width
    const int bcount = (int)(std::round( multiplier_ * (xmax - xmin) )) / step_;
    const int bwidth = qMax( BAR_WIDTH_MIN, BAR_WIDTH - zoom_ );
    const int bwidthtotal = bcount * (BAR_SEPARATION + (2 * bwidth));

    // determine intervals
    double xinterval;
    int numDecimalPlacesStrike;

    // width of maximum strike price text element
    const double xmaxwidth( fm.boundingRect( QString::number( xmax, 'f', 4 ) ).width() );

    calcIntervalValues( xmin, xmax, bwidthtotal, xmaxwidth, xinterval, numDecimalPlacesStrike );

    numDecimalPlacesStrike = qMax( numDecimalPlacesStrike, 2 );

    double yinterval;
    int numDecimalPlacesOpenInt;

    calcIntervalValues( ymin, ymax, height(), 50.0, yinterval, numDecimalPlacesOpenInt );

    numDecimalPlacesOpenInt = 0; // force zero decimal places

    // graph constants
    xmin = xinterval * std::floor( xmin / xinterval );
    xmax = xinterval * std::ceil( xmax / xinterval ) + xinterval; // additional interval to make room for offset bars

    ymin = yinterval * std::floor( ymin / yinterval );
    ymax = yinterval * std::ceil( ymax / yinterval );

    const int marginWidth( SPACING + fm.boundingRect( QString::number( ymax, 'f', numDecimalPlacesOpenInt ) ).width() );
    const int marginHeight( SPACING + fm.boundingRect( "0123456789/:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ).height() );

    // -----
    // graph
    // -----

    // dots per dollar ratio
    const double ratio = (BAR_SEPARATION + (2 * bwidth)) / (step_ / multiplier_);

    int gwidth( (int)(std::ceil( ratio * (xmax - xmin) )) + marginWidth + SPACING ); // width includes left margin and right margin
    int gheight( height() );

    int gtop( SPACING );
    int gleft( marginWidth );
    int gbottom( gheight - marginHeight );
    int gright( gwidth - SPACING );

    // ensure bar width is non-zero
    if (( bwidth <= 0 ) || ( (gright - gleft) < bwidthtotal ))
    {
        LOG_WARN << "invalid bar width";
        return;
    }

    // show/hide scroll bar
    int offsettotal = qMax( 0, gwidth - width() );
    int offset = 0;

    if ( offsettotal <= 0 )
        scroll_->hide();
    else
    {
        scroll_->show();
        scroll_->setRange( 0, offsettotal );

        offset = scroll_->value();

        gbottom -= scroll_->height();
    }

    graph_ = QPixmap( width(), gheight );
    graph_.fill( palette().base().color() );

    QPainter painter;
    painter.begin( &graph_ );

    // open interest intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );

    for ( double i( ymin ); i <= ymax; i += yinterval )
    {
        const int y( gbottom - scaled( i, ymin, ymax, gbottom-gtop ) );

        painter.drawLine( gleft-2, y, gright-offset, y );
        painter.drawText( 0, y-25, marginWidth-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( i, 'f', numDecimalPlacesOpenInt ) );
    }

    // open interest bars
    QList<QColor> penColor;
    penColor.append( Qt::blue );
    penColor.append( Qt::red );

    int p( 0 );

    foreach ( const ValuesMap *values, valuesList )
    {
        for ( ValuesMap::const_iterator i( values->constBegin() ); i != values->constEnd(); ++i )
        {
            int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );
            int y( gbottom - scaled( i.value(), ymin, ymax, gbottom-gtop ) );

            x += BAR_SEPARATION + bwidth;
            x -= offset;

            if ( &openInt_.callOpenInterest == values )
                x -= bwidth;

            painter.setPen( QPen( penColor[p], 0 ) );
            painter.setBrush( QBrush( penColor[p] ) );

            painter.drawRect( x, y, bwidth-1, gbottom-y );
        }

        ++p;
    }

    // strike prices
    painter.setPen( QPen( Qt::darkGray, 0 ) );

    int xprev = -999999;

    for ( ValuesMap::const_iterator i( openInt_.callOpenInterest.constBegin() ); i != openInt_.callOpenInterest.constEnd(); ++i )
    {
        int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );

        x += BAR_SEPARATION + bwidth;
        x -= offset;

        if ( xmaxwidth <= (x - xprev) )
        {
            painter.drawLine( x, gbottom, x, gbottom+2 );
            painter.drawText( x-4, gbottom+4, xmaxwidth, 50, Qt::AlignLeft | Qt::AlignTop, QString::number( i.key(), 'f', numDecimalPlacesStrike ) );

            xprev = x;
        }
    }

    // price
    if ( 0.0 < price_ )
    {
        int x( gleft + scaled( price_, xmin, xmax, gright-gleft ) );

        x += BAR_SEPARATION + bwidth;
        x -= offset;

        painter.setPen( QPen( palette().text().color(), 2, Qt::DashLine ) );

        painter.drawLine( x, gtop, x, gbottom );
    }

    // stamp
    painter.setPen( QPen( palette().text().color(), 0 ) );
    painter.drawText( 0, SPACING+4, width(), 50, Qt::AlignHCenter | Qt::AlignTop, stamp_.toString() );

    // labels
    painter.setPen( QPen( penColor[0], 0 ) );
    painter.drawText( 0, SPACING+4, width()-SPACING, 50, Qt::AlignRight | Qt::AlignTop, tr( "CALLS" ) );

    painter.setPen( QPen( penColor[1], 0 ) );
    painter.drawText( 0, SPACING+4+marginHeight, width()-SPACING, 50, Qt::AlignRight | Qt::AlignTop, tr( "PUTS" ) );

    painter.end();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int OptionChainOpenInterestWidget::scaled( double p, double min, double max, int height )
{
    return std::round( ((p - min) / (max - min)) * height );
}

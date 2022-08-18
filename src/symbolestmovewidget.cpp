/**
 * @file symbolestmovewidget.cpp
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
#include "symbolestmovewidget.h"

#include "db/appdb.h"
#include "db/symboldbs.h"

#include <cmath>

#include <QComboBox>
#include <QHBoxLayout>
#include <QPainter>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolEstimatedMovementWidget::SymbolEstimatedMovementWidget( const QString& symbol, double price, QWidget *parent ) :
    _Mybase( parent ),
    symbol_( symbol ),
    price_( price )
{
    // init
    initialize();
    createLayout();
    translate();

    // refresh
    refreshData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SymbolEstimatedMovementWidget::~SymbolEstimatedMovementWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolEstimatedMovementWidget::translate()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolEstimatedMovementWidget::refreshData()
{
    histMin_.clear();
    histMax_.clear();

    implMin_.clear();
    implMax_.clear();

    implStrikes_.clear();

    stamp_ = AppDatabase::instance()->currentDateTime();

    // fetch most recent volatility information
    LOG_TRACE << "fetch future volatility...";

    QMap<QDate, FutureVolatilities> vfuture;
    SymbolDatabases::instance()->futureVolatility( symbol(), vfuture, stamp_.addDays( -IV_RANGE_DAYS ), stamp_ );

    if ( vfuture.isEmpty() )
    {
        LOG_WARN << "no future volatility found";
        return;
    }

    // populate curves
    histMin_[0.0] = histMax_[0.0] =
        implMin_[0.0] = implMax_[0.0] = implStrikes_[0.0] = price_;

    for ( QMap<QDate, FutureVolatilities>::const_iterator f( vfuture.constBegin() ); f != vfuture.constEnd(); f++ )
    {
        // hist movement
        if ( 0.0 < f->hist )
        {
            const double volp( f->hist * sqrt( f->dte / AppDatabase::instance()->numTradingDays() ) );
            const double estMovement( price_ * volp );

            histMin_[f->dte] = price_ - estMovement;
            histMax_[f->dte] = price_ + estMovement;
        }

        // impl movement
        if ( 0.0 < f->impl )
        {
            const double volp( f->impl * sqrt( f->dte / AppDatabase::instance()->numTradingDays() ) );
            const double estMovement( price_ * volp );

            implMin_[f->dte] = price_ - estMovement;
            implMax_[f->dte] = price_ + estMovement;
        }

        // impl vol strike estimation
        if ( 0.0 < f->strike )
            implStrikes_[f->dte] = f->strike;
    }

    // draw!
    drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolEstimatedMovementWidget::paintEvent( QPaintEvent *e )
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
void SymbolEstimatedMovementWidget::resizeEvent( QResizeEvent *e )
{
    // new graph
    drawGraph();

    _Mybase::resizeEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolEstimatedMovementWidget::initialize()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolEstimatedMovementWidget::createLayout()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolEstimatedMovementWidget::haveCurveData() const
{
    if (( histMin_.size() <= 1 ) || ( histMax_.size() <= 1 ))
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolEstimatedMovementWidget::calcMinMaxValues( const ValuesMap& values, double& kmin, double& kmax, double& vmin, double& vmax ) const
{
    kmin = vmin = 999999.99;
    kmax = vmax = -999999.99;

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
void SymbolEstimatedMovementWidget::calcIntervalValues( double min, double max, double gheight, double ints, double& interval, int& numDecimals ) const
{
    static const int FOOTER( 25 );
    static const double MAX_MULT( 1000.0 );

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
void SymbolEstimatedMovementWidget::drawGraph()
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
    // x axis = days to expiration
    // y axis = implied volatility
    double xmin;
    double xmax;

    double ymin = xmin = 999999.99;
    double ymax = xmax = -999999.99;

    {
        QList<const ValuesMap*> values;
        values.append( &histMin_ );
        values.append( &histMax_ );
        values.append( &implMin_ );
        values.append( &implMax_ );
        values.append( &implStrikes_ );

        foreach ( const ValuesMap *v, values )
        {
            double kmin;
            double kmax;
            double vmin;
            double vmax;

            if ( calcMinMaxValues( (*v), kmin, kmax, vmin, vmax ) )
            {
                xmin = qMin( xmin, kmin );
                xmax = qMax( xmax, kmax );

                ymin = qMin( ymin, vmin );
                ymax = qMax( ymax, vmax );
            }
        }
    }

    if ( xmax < xmin )
    {
        graph_ = QPixmap( size() );
        graph_.fill( palette().base().color() );

        QPainter painter( &graph_ );
        painter.setPen( QPen( palette().text().color(), 0 ) );
        painter.drawText( 0, 0, width(), height(), Qt::AlignCenter, tr( "Select one or more expiration dates to display" ) );

        return;
    }

    const QFontMetrics fm( fontMetrics() );

    // determine intervals

    // width of maximum date text element
    const double xmaxwidth( fm.boundingRect( "XX XXX XX" ).width() );

    double yinterval;
    int numDecimalPlacesPrice;

    calcIntervalValues( ymin, ymax, height(), 50.0, yinterval, numDecimalPlacesPrice );

    numDecimalPlacesPrice = qMax( numDecimalPlacesPrice, 2 );

    // graph constants
    ymin = yinterval * std::floor( ymin / yinterval );
    ymax = yinterval * std::ceil( ymax / yinterval );

    const int marginWidth( SPACING +
        qMax( fm.boundingRect( QString::number( ymax, 'f', numDecimalPlacesPrice ) ).width() ,
              fm.boundingRect( QString::number( ymin, 'f', numDecimalPlacesPrice ) ).width() ) );

    const int marginHeight( SPACING + fm.boundingRect( "0123456789/:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ).height() );

    // -----
    // graph
    // -----

    int gwidth( width() );
    int gheight( height() );

    int gtop( SPACING );
    int gleft( marginWidth );
    int gbottom( gheight - marginHeight );
    int gright( gwidth - SPACING );

    graph_ = QPixmap( gwidth, gheight );
    graph_.fill( palette().base().color() );

    QPainter painter;
    painter.begin( &graph_ );

    // price intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );

    for ( double i( ymin ); i <= ymax; i += yinterval )
    {
        const int y( gbottom - scaled( i, ymin, ymax, gbottom-gtop ) );

        painter.drawLine( gleft-2, y, gright, y );
        painter.drawText( 0, y-25, marginWidth-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( i, 'f', numDecimalPlacesPrice ) );
    }

    // expiry dates
    int expiryxprev( -999 );

    painter.drawLine( gleft, gtop, gleft, gbottom );

    for ( ValuesMap::const_iterator i( histMin_.constBegin() ); i != histMin_.constEnd(); ++i )
    {
        const int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );

        painter.drawLine( x, gbottom, x, gbottom+2 );

        if (( 0.0 < i.key() ) && ( expiryxprev <= x ))
        {
            const QDate expiry( AppDatabase::instance()->currentDateTime().date().addDays( i.key() ) );
            const QString expiryStr( expiry.toString( "dd MMM yy" ) );

            if ( x < gright )
                painter.drawText( x-4, gbottom+4, xmaxwidth, 50, Qt::AlignLeft | Qt::AlignTop, expiryStr );
            else
                painter.drawText( x-fm.boundingRect( expiryStr ).width(), gbottom+4, xmaxwidth, 50, Qt::AlignLeft | Qt::AlignTop, expiryStr );

            expiryxprev = x + xmaxwidth;
        }
    }

    // impl vol
    int implxminprev( 0 );
    int implyminprev;

    for ( ValuesMap::const_iterator i( implMin_.constBegin() ); i != implMin_.constEnd(); ++i )
    {
        const int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );
        const int y( gbottom - scaled( i.value(), ymin, ymax, gbottom-gtop ) );

        if ( 0 < implxminprev )
        {
            QColor c( palette().text().color() );

            if ( histMin_.contains( i.key() ) )
                c = (i.value() < histMin_[i.key()]) ? Qt::darkGreen : Qt::red;

            painter.setPen( QPen( c, 1 ) );
            painter.drawLine( implxminprev, implyminprev, x, y );
        }

        implxminprev = x;
        implyminprev = y;
    }

    int implxmaxprev( 0 );
    int implymaxprev;

    for ( ValuesMap::const_iterator i( implMax_.constBegin() ); i != implMax_.constEnd(); ++i )
    {
        const int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );
        const int y( gbottom - scaled( i.value(), ymin, ymax, gbottom-gtop ) );

        if ( 0 < implxmaxprev )
        {
            QColor c( palette().text().color() );

            if ( histMax_.contains( i.key() ) )
                c = (histMax_[i.key()] < i.value()) ? Qt::darkGreen :  Qt::red;

            painter.setPen( QPen( c, 1 ) );
            painter.drawLine( implxmaxprev, implymaxprev, x, y );
        }

        implxmaxprev = x;
        implymaxprev = y;
    }

    // hist vol
    int histxminprev( 0 );
    int histyminprev;

    for ( ValuesMap::const_iterator i( histMin_.constBegin() ); i != histMin_.constEnd(); ++i )
    {
        const int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );
        const int y( gbottom - scaled( i.value(), ymin, ymax, gbottom-gtop ) );

        if ( 0 < histxminprev )
        {
            painter.setPen( QPen( palette().text().color(), 2 ) );
            painter.drawLine( histxminprev, histyminprev, x, y );
        }

        histxminprev = x;
        histyminprev = y;
    }

    int histxmaxprev( 0 );
    int histymaxprev;

    for ( ValuesMap::const_iterator i( histMax_.constBegin() ); i != histMax_.constEnd(); ++i )
    {
        const int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );
        const int y( gbottom - scaled( i.value(), ymin, ymax, gbottom-gtop ) );

        if ( 0 < histxmaxprev )
        {
            painter.setPen( QPen( palette().text().color(), 2 ) );
            painter.drawLine( histxmaxprev, histymaxprev, x, y );
        }

        histxmaxprev = x;
        histymaxprev = y;
    }


    // implied strikes
    int implsxprev( 0 );
    int implsyprev;

    for ( ValuesMap::const_iterator i( implStrikes_.constBegin() ); i != implStrikes_.constEnd(); ++i )
    {
        const int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );
        const int y( gbottom - scaled( i.value(), ymin, ymax, gbottom-gtop ) );

        if ( 0 < implsxprev )
        {
            painter.setPen( QPen( Qt::blue, 1, Qt::DashLine ) );
            painter.drawLine( implsxprev, implsyprev, x, y );
        }

        implsxprev = x;
        implsyprev = y;
    }

    // price
    if ( 0.0 < price_ )
    {
        const int y( gbottom - scaled( price_, ymin, ymax, gbottom-gtop ) );

        painter.setPen( QPen( palette().text().color(), 2, Qt::DashLine ) );
        painter.drawLine( gleft, y, gright, y );
    }

    // stamp
    painter.setPen( QPen( palette().text().color(), 0 ) );
    painter.drawText( 0, SPACING+4, gwidth, 50, Qt::AlignHCenter | Qt::AlignTop, stamp_.toString() );

    // labels
    painter.setPen( QPen( palette().text().color(), 0 ) );
    painter.drawText( gleft+4, SPACING+4+0*marginHeight, gwidth-SPACING, 50, Qt::AlignLeft | Qt::AlignTop, tr( "Hist. Vol." ) );

    painter.setPen( QPen( Qt::red, 0 ) );
    painter.drawText( gleft+4, SPACING+4+1*marginHeight, gwidth-SPACING, 50, Qt::AlignLeft | Qt::AlignTop, tr( "Impl. Vol. (< Hist. Vol.)" ) );

    painter.setPen( QPen( Qt::darkGreen, 0 ) );
    painter.drawText( gleft+4, SPACING+4+2*marginHeight, gwidth-SPACING, 50, Qt::AlignLeft | Qt::AlignTop, tr( "Impl. Vol. (>= Hist. Vol.)" ) );

    painter.setPen( QPen( Qt::blue, 0 ) );
    painter.drawText( gleft+4, SPACING+4+3*marginHeight, gwidth-SPACING, 50, Qt::AlignLeft | Qt::AlignTop, tr( "Direction" ) );

    painter.end();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int SymbolEstimatedMovementWidget::scaled( double p, double min, double max, int height )
{
    return std::round( ((p - min) / (max - min)) * height );
}

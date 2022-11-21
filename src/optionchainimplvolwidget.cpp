/**
 * @file optionchainimplvolwidget.cpp
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
#include "optionchainimplvolwidget.h"

#include "db/appdb.h"
#include "db/symboldbs.h"

#include <cmath>

#include <QAbstractItemView>
#include <QComboBox>
#include <QCryptographicHash>
#include <QHBoxLayout>
#include <QPainter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionChainImpliedVolatilityWidget::OptionChainImpliedVolatilityWidget( const QString& underlying, double underlyingPrice, const QDate& expiryDate, const QDateTime& stamp, QWidget *parent ) :
    _Mybase( parent ),
    underlying_( underlying ),
    price_( underlyingPrice ),
    end_( stamp ),
    expiryDate_( expiryDate )
{
    // init
    initialize();
    createLayout();
    translate();

    // refresh
    refreshData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionChainImpliedVolatilityWidget::~OptionChainImpliedVolatilityWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainImpliedVolatilityWidget::translate()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainImpliedVolatilityWidget::refreshData()
{
    // check expiry date
    if ( !expiryDate_.isValid() )
    {
        LOG_WARN << "missing expiry date";
        return;
    }

    // fetch curve data
    curve_.volatility.clear();

    LOG_TRACE << "fetch curves...";
    stamp_ = SymbolDatabases::instance()->optionChainCurves( underlying(), expiryDate_, curve_, QDateTime(), end_ );

    if ( !haveCurveData() )
    {
        LOG_WARN << "no volatility curve for " << qPrintable( expiryDate_.toString() );
        return;
    }

    // draw!
    drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainImpliedVolatilityWidget::paintEvent( QPaintEvent *e )
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
void OptionChainImpliedVolatilityWidget::resizeEvent( QResizeEvent *e )
{
    // new graph
    drawGraph();

    _Mybase::resizeEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainImpliedVolatilityWidget::initialize()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainImpliedVolatilityWidget::createLayout()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionChainImpliedVolatilityWidget::haveCurveData() const
{
    return !curve_.volatility.isEmpty();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionChainImpliedVolatilityWidget::calcMinMaxValues( const ValuesMap& values, double& kmin, double& kmax, double& vmin, double& vmax ) const
{
    kmin = vmin = 999999.99;
    kmax = vmax = 0.0;

    for ( ValuesMap::const_iterator i( values.constBegin() ); i != values.constEnd(); ++i )
    {
        const double vol( 100.0 * i.value() );

        kmin = qMin( kmin, i.key() );
        kmax = qMax( kmax, i.key() );

        if ( 0.0 < vol )
        {
            vmin = qMin( vmin, vol );
            vmax = qMax( vmax, vol );
        }
    }

    return (kmin <= kmax);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainImpliedVolatilityWidget::calcIntervalValues( double min, double max, double gheight, double ints, double& interval, int& numDecimals ) const
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
            if (( ints <= h ) || ( MAX_MULT <= mult ))
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
void OptionChainImpliedVolatilityWidget::drawGraph()
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

    double ymin = xmin = 999999.99;
    double ymax = xmax = 0.0;

    QList<const ValuesMap*> valuesList;
    valuesList.append( &curve_.callVolatility );
    valuesList.append( &curve_.putVolatility );
    valuesList.append( &curve_.volatility );

    foreach ( const ValuesMap *values, valuesList )
    {
        double kmin;
        double kmax;
        double vmin;
        double vmax;

        if ( calcMinMaxValues( (*values), kmin, kmax, vmin, vmax ) )
        {
            xmin = qMin( xmin, kmin );
            xmax = qMax( xmax, kmax );

            ymin = qMin( ymin, vmin );
            ymax = qMax( ymax, vmax );
        }
    }

    if ( xmax < xmin )
    {
        LOG_WARN << "invalid coordinates";
        return;
    }

    const QFontMetrics fm( fontMetrics() );

    // determine intervals
    double xinterval;
    int numDecimalPlacesStrike;

    // width of maximum strike price text element
    const double xmaxwidth( fm.boundingRect( QString::number( xmax, 'f', 4 ) ).width() );

    calcIntervalValues( xmin, xmax, width(), xmaxwidth, xinterval, numDecimalPlacesStrike );

    numDecimalPlacesStrike = qMax( numDecimalPlacesStrike, 2 );

    double yinterval;
    int numDecimalPlacesVI;

    calcIntervalValues( ymin, ymax, height(), 50.0, yinterval, numDecimalPlacesVI );

    // graph constants
    xmin = xinterval * std::floor( xmin / xinterval );
    xmax = xinterval * std::ceil( xmax / xinterval );

    ymin = yinterval * std::floor( ymin / yinterval );
    ymax = yinterval * std::ceil( ymax / yinterval );

    const int marginWidth( SPACING + fm.boundingRect( QString::number( ymax, 'f', numDecimalPlacesVI ) ).width() );
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

    // impl vol intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );

    for ( double i( ymin ); i <= ymax; i += yinterval )
    {
        const int y( gbottom - scaled( i, ymin, ymax, gbottom-gtop ) );

        painter.drawLine( gleft-2, y, gright, y );
        painter.drawText( 0, y-25, marginWidth-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( i, 'f', numDecimalPlacesVI ) );
    }

    // strike prices
    painter.drawLine( gleft, gtop, gleft, gbottom );

    for ( double i( xmin ); i <= xmax; i += xinterval )
    {
        const int x( gleft + scaled( i, xmin, xmax, gright-gleft ) );

        painter.drawLine( x, gbottom, x, gbottom+2 );
        painter.drawText( x-4, gbottom+4, 50, 50, Qt::AlignLeft | Qt::AlignTop, QString::number( i, 'f', numDecimalPlacesStrike ) );
    }

    // volatility curves
    QList<QColor> penColor;
    penColor.append( Qt::blue );
    penColor.append( Qt::red );
    penColor.append( palette().text().color() );

    QList<int> penWidth;
    penWidth.append( 0 );
    penWidth.append( 0 );
    penWidth.append( 2 );

    int p( 0 );

    foreach ( const ValuesMap *values, valuesList )
    {
        int xprev( 0.0 );
        int yprev( 0.0 );

        bool solid( true );

        for ( QMap<double, double>::const_iterator i( values->constBegin() ); i != values->constEnd(); ++i )
        {
            // skip over spots without a volatility
            // use dotted line to indicate spots were skipped
            if ( i.value() <= 0.0 )
            {
                if ( 0.0 < xprev )
                    solid = false;

                continue;
            }

            const int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );
            const int y( gbottom - scaled( 100.0 * i.value(), ymin, ymax, gbottom-gtop ) );

            painter.setPen( QPen( penColor[p], penWidth[p], solid ? Qt::SolidLine : Qt::DotLine ) );

            if ( 0.0 < xprev )
                painter.drawLine( xprev, yprev, x, y );

            xprev = x;
            yprev = y;

            solid = true;
        }

        ++p;
    }

    // price
    if ( 0.0 < price_ )
    {
        const int x( gleft + scaled( price_, xmin, xmax, gright-gleft ) );

        painter.setPen( QPen( palette().text().color(), 2, Qt::DashLine ) );

        painter.drawLine( x, gtop, x, gbottom );
    }

    // stamp
    painter.setPen( QPen( palette().text().color(), 0 ) );
    painter.drawText( 0, SPACING+4, gwidth, 50, Qt::AlignHCenter | Qt::AlignTop, stamp_.toString() );

    // labels
    painter.setPen( QPen( penColor[0], 0 ) );
    painter.drawText( 0, SPACING+4, gwidth-SPACING, 50, Qt::AlignRight | Qt::AlignTop, tr( "Calls" ) );

    painter.setPen( QPen( penColor[1], 0 ) );
    painter.drawText( 0, SPACING+4+marginHeight, gwidth-SPACING, 50, Qt::AlignRight | Qt::AlignTop, tr( "Puts" ) );

    painter.end();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int OptionChainImpliedVolatilityWidget::scaled( double p, double min, double max, int height )
{
    return std::round( ((p - min) / (max - min)) * height );
}

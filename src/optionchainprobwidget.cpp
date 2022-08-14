/**
 * @file optionchainprobwidget.cpp
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
#include "optionchainprobwidget.h"

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
OptionChainProbabilityWidget::OptionChainProbabilityWidget( const QString& underlying, double underlyingPrice, const QDate& expiryDate, const QDateTime& stamp, QWidget *parent ) :
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
OptionChainProbabilityWidget::~OptionChainProbabilityWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainProbabilityWidget::addLeg( const QString& name, double strike, bool isCall, bool isShort )
{
    Leg leg;
    leg.description = name;
    leg.strike = strike;
    leg.isCall = isCall;
    leg.isShort = isShort;

    legs_.append( leg );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainProbabilityWidget::translate()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainProbabilityWidget::refreshData()
{
    // check expiry date
    if ( !expiryDate_.isValid() )
    {
        LOG_WARN << "missing expiry date";
        return;
    }

    // fetch curve data
    curve_.itmProbability.clear();
    curve_.otmProbability.clear();

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
void OptionChainProbabilityWidget::paintEvent( QPaintEvent *e )
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
void OptionChainProbabilityWidget::resizeEvent( QResizeEvent *e )
{
    // new graph
    drawGraph();

    _Mybase::resizeEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionChainProbabilityWidget::isCall() const
{
    bool result( true );

    foreach ( const Leg& leg, legs_ )
        result &= leg.isCall;

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionChainProbabilityWidget::isPut() const
{
    bool result( true );

    foreach ( const Leg& leg, legs_ )
        result &= !leg.isCall;

    return result;;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainProbabilityWidget::initialize()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainProbabilityWidget::createLayout()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const OptionChainProbabilityWidget::ValuesMap *OptionChainProbabilityWidget::curveData() const
{
    if ( isCall() )
        return &curve_.otmProbability;
    else if ( isPut() )
        return &curve_.itmProbability;

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionChainProbabilityWidget::haveCurveData() const
{
    const ValuesMap *d( curveData() );

    return (( d ) && ( !d->isEmpty() ));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionChainProbabilityWidget::calcMinMaxValues( const ValuesMap& values, double& kmin, double& kmax, double& vmin, double& vmax ) const
{
    kmin = vmin = 999999.99;
    kmax = vmax = 0.0;

    for ( ValuesMap::const_iterator i( values.constBegin() ); i != values.constEnd(); ++i )
    {
        const double prob( 100.0 * i.value() );

        kmin = qMin( kmin, i.key() );
        kmax = qMax( kmax, i.key() );

        vmin = qMin( vmin, prob );
        vmax = qMax( vmax, prob );
    }

    return (kmin <= kmax);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainProbabilityWidget::calcIntervalValues( double min, double max, double gheight, double ints, double& interval, int& numDecimals ) const
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
void OptionChainProbabilityWidget::drawGraph()
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

    const ValuesMap *d( curveData() );

    {
        double kmin;
        double kmax;
        double vmin;
        double vmax;

        if ( calcMinMaxValues( (*d), kmin, kmax, vmin, vmax ) )
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

    // width of maximum stirke price text element
    const double xmaxwidth( fm.boundingRect( QString::number( xmax, 'f', 4 ) ).width() );

    calcIntervalValues( xmin, xmax, width(), xmaxwidth, xinterval, numDecimalPlacesStrike );

    numDecimalPlacesStrike = qMax( numDecimalPlacesStrike, 2 );

    double yinterval;
    int numDecimalPlacesProb;

    calcIntervalValues( ymin, ymax, height(), 50.0, yinterval, numDecimalPlacesProb );

    // graph constants
    xmin = xinterval * std::floor( xmin / xinterval );
    xmax = xinterval * std::ceil( xmax / xinterval );

    ymin = yinterval * std::floor( ymin / yinterval );
    ymax = yinterval * std::ceil( ymax / yinterval );

    const int marginWidth( SPACING + fm.boundingRect( QString::number( ymax, 'f', numDecimalPlacesProb ) ).width() );
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

    // probability intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );

    for ( double i( ymin ); i <= ymax; i += yinterval )
    {
        const int y( gbottom - scaled( i, ymin, ymax, gbottom-gtop ) );

        painter.drawLine( gleft-2, y, gright, y );
        painter.drawText( 0, y-25, marginWidth-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( i, 'f', numDecimalPlacesProb ) );
    }

    // strike prices
    painter.drawLine( gleft, gtop, gleft, gbottom );

    for ( double i( xmin ); i <= xmax; i += xinterval )
    {
        const int x( gleft + scaled( i, xmin, xmax, gright-gleft ) );

        painter.drawLine( x, gbottom, x, gbottom+2 );
        painter.drawText( x-4, gbottom+4, 50, 50, Qt::AlignLeft | Qt::AlignTop, QString::number( i, 'f', numDecimalPlacesStrike ) );
    }

    // probability curve
    int xprev( 0.0 );
    int yprev( 0.0 );

    painter.setPen( QPen( palette().text().color(), 2 ) );

    for ( ValuesMap::const_iterator i( d->constBegin() ); i != d->constEnd(); ++i )
    {
        const int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );
        const int y( gbottom - scaled( 100.0 * i.value(), ymin, ymax, gbottom-gtop ) );

        if ( 0.0 < xprev )
            painter.drawLine( xprev, yprev, x, y );

        xprev = x;
        yprev = y;
    }

    // price
    if ( 0.0 < price_ )
    {
        const int x( gleft + scaled( price_, xmin, xmax, gright-gleft ) );

        painter.setPen( QPen( palette().text().color(), 2, Qt::DashLine ) );

        painter.drawLine( x, gtop, x, gbottom );
    }

    // leg strike prices
    foreach ( const Leg& leg, legs_ )
    {
        const int x( gleft + scaled( leg.strike, xmin, xmax, gright-gleft ) );

        painter.setPen( QPen( legColor( leg ), 0 ) );

        painter.drawLine( x, gtop, x, gbottom );
    }

    // stamp
    painter.setPen( QPen( palette().text().color(), 0 ) );
    painter.drawText( 0, SPACING+4, gwidth, 50, Qt::AlignHCenter | Qt::AlignTop, stamp_.toString() );

    // labels
    const QLocale l( QLocale::system() );

    int ltop( SPACING+4 );

    painter.setPen( QPen( palette().text().color(), 0 ) );
    painter.drawText( 0, ltop, gwidth-SPACING, 50, Qt::AlignRight | Qt::AlignTop, tr( "OTM Probability" ) );

    foreach ( const Leg& leg, legs_ )
    {
        const double prob( 100 * calcStrikeProbability( leg.strike ) );

        const int x( gleft + scaled( leg.strike, xmin, xmax, gright-gleft ) );
        const int y( gbottom - scaled( prob, ymin, ymax, gbottom-gtop ) );

        painter.setPen( QPen( legColor( leg ), 0 ) );

        // draw description label
        ltop += marginHeight;
        painter.drawText( 0, ltop, gwidth-SPACING, 50, Qt::AlignRight | Qt::AlignTop, leg.description );

        // draw probability label
        const QString probLabel( QString( "%0%" ).arg( l.toString( prob, 'f', 2 ) ) );

        const int probWidth( 120 );
        const int probHeight( 2*marginHeight );

        int probLeft( x - (probWidth/2) );
        int probTop( 0 );

        // dot in top half
        if ( y <= (gheight/2) )
            probTop = gbottom - probHeight;

        painter.drawText( probLeft, probTop, probWidth, probHeight, Qt::AlignCenter, probLabel );
    }

    painter.end();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double OptionChainProbabilityWidget::calcStrikeProbability( double strike ) const
{
    if ( !haveCurveData() )
        return 0.0;

    const ValuesMap *d( curveData() );

    // check for exact match
    ValuesMap::const_iterator v( d->constFind( strike ) );

    if ( v != d->constEnd() )
        return v.value();

    // otherwise we need to interpolate
    ValuesMap::const_iterator above( d->constEnd() );
    ValuesMap::const_iterator below( d->constEnd() );

    // find probability of strike above and below
    for ( ValuesMap::const_iterator i( d->constBegin() ); i != d->constEnd(); ++i )
    {
        if ( strike <= i.key() )
            if (( d->constEnd() == above ) || ( i.key() < above.key() ))
                above = i;

        if ( i.key() <= strike )
            if (( d->constEnd() == below ) || ( below.key() < i.key() ))
                below = i;
    }

    if (( d->constEnd() == above ) && ( d->constEnd() != below ))
        return below.value();
    else if (( d->constEnd() != above ) && ( d->constEnd() == below ))
        return above.value();
    else if (( d->constEnd() != above ) && ( d->constEnd() != below ))
    {
        if ( above == below )
            return above.value();

        // linear fit
        double v( (strike - below.key()) / (above.key() - below.key()) );   // interpolate strike
        v *= (above.value() - below.value());                               // convert to probability
        v +=  below.value();                                                // adjust by probability minimum

        return v;
    }

    LOG_WARN << "could not calculate probability for strike " << strike;
    return 0.0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int OptionChainProbabilityWidget::scaled( double p, double min, double max, int height )
{
    return std::round( ((p - min) / (max - min)) * height );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QColor OptionChainProbabilityWidget::legColor( const Leg& leg )
{
    const QString desc( leg.description );

    QCryptographicHash h( QCryptographicHash::Md5 );
    h.addData( desc.toLatin1() );

    const QByteArray a( h.result() );

    return QColor( (unsigned char) a[0], (unsigned char) a[1], (unsigned char) a[2] );
}

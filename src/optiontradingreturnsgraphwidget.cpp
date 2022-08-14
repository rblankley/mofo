/**
 * @file optiontradingreturnsgraphwidget.cpp
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
#include "optiontradingreturnsgraphwidget.h"

#include "db/appdb.h"
#include "db/optiontradingitemmodel.h"
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
OptionTradingReturnsGraphWidget::OptionTradingReturnsGraphWidget( int index, model_type *model, QWidget *parent ) :
    _Mybase( parent ),
    model_( model ),
    index_( index )
{
    // grab model data
    underlying_ = modelData( model_type::UNDERLYING ).toString();
    underlyingPrice_ = modelData( model_type::UNDERLYING_PRICE ).toDouble();

    strat_ = modelData( model_type::STRATEGY ).toInt();

    // single
    shortStrikePrice_ = modelData( model_type::STRIKE_PRICE ).toDouble();
    longStrikePrice_ = 0.0;

    // spread
    if (( model_type::VERT_BULL_PUT == strat_ ) || ( model_type::VERT_BEAR_CALL == strat_ ))
    {
        // format is "short/long"
        const QStringList strikes( modelData( model_type::STRIKE_PRICE ).toString().split( "/" ) );

        if ( 2 == strikes.size() )
        {
            shortStrikePrice_ = strikes[0].toDouble();
            longStrikePrice_ = strikes[1].toDouble();
        }
    }

    breakEvenPrice_ = modelData( model_type::BREAK_EVEN_PRICE ).toDouble();

    expiryDate_ = QDate::fromString( modelData( model_type::EXPIRY_DATE ).toString(), Qt::ISODate );

    // init
    initialize();
    createLayout();
    translate();

    // refresh data
    refreshData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionTradingReturnsGraphWidget::~OptionTradingReturnsGraphWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsGraphWidget::translate()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsGraphWidget::refreshData()
{
    // clear existing returns info
    returns_.clear();

    // generate payouts
    // map of x to (m, c)
    LinearEquationMap payouts;

    const double maxGain( modelData( model_type::MAX_GAIN ).toDouble() );
    const double maxLoss( -modelData( model_type::MAX_LOSS ).toDouble() );

    // vertical
    if (( model_type::VERT_BULL_PUT == strat_ ) && ( 0.0 < longStrikePrice_ ) && ( longStrikePrice_ < shortStrikePrice_ ))
    {
        const double slope( (maxGain - maxLoss) / (shortStrikePrice_ - longStrikePrice_) );
        const double yintercept( maxLoss - slope * longStrikePrice_ );

        payouts[0.0] = LinearEquation( 0.0, maxLoss );
        payouts[longStrikePrice_] = LinearEquation( slope, yintercept );
        payouts[shortStrikePrice_] = LinearEquation( 0.0, maxGain );

        // add data point for break even
        if ( 0.0 < breakEvenPrice_ )
            returns_[breakEvenPrice_] = 0.0;
    }
    else if (( model_type::VERT_BEAR_CALL == strat_ ) && ( 0.0 < longStrikePrice_ ) && ( shortStrikePrice_ < longStrikePrice_ ))
    {
        const double slope( (maxLoss - maxGain) / (longStrikePrice_ - shortStrikePrice_) );
        const double yintercept( maxGain - slope * shortStrikePrice_ );

        payouts[0.0] = LinearEquation( 0.0, maxGain );
        payouts[shortStrikePrice_] = LinearEquation( slope, yintercept );
        payouts[longStrikePrice_] = LinearEquation( 0.0, maxLoss );

        // add data point for break even
        if ( 0.0 < breakEvenPrice_ )
            returns_[breakEvenPrice_] = 0.0;
    }
    // cash secured put
    // covered call
    else if ( model_type::SINGLE == strat_ )
    {
        if (( 0.0 < breakEvenPrice_ ) && ( breakEvenPrice_ <= shortStrikePrice_ ))
        {
            // [0.00, break even)
            const double slope0( -maxLoss / breakEvenPrice_ );
            const double yintercept0( maxLoss );

            // [break even, short strike price)
            const double slope1( maxGain / (shortStrikePrice_ - breakEvenPrice_) );
            const double yintercept1( -slope1 * breakEvenPrice_ );

            payouts[0.0] = LinearEquation( slope0, yintercept0 );
            payouts[breakEvenPrice_] = LinearEquation( slope1, yintercept1 );
            payouts[shortStrikePrice_] = LinearEquation( 0.0, maxGain );

            // add data point for break even
            returns_[breakEvenPrice_] = 0.0;
        }
        else
        {
            const double slope( (maxGain - maxLoss) / shortStrikePrice_ );
            const double yintercept( maxLoss );

            payouts[0.0] = LinearEquation( slope, yintercept );
            payouts[shortStrikePrice_] = LinearEquation( 0.0, maxGain );
        }
    }

    // fetch strike prices
    // use the open interest call to do this, we do not really care about the open interest data just the strikes
    OptionChainOpenInterest openInt;

    LOG_TRACE << "fetch strike prices...";
    stamp_ = SymbolDatabases::instance()->optionChainOpenInterest( underlying(), expiryDate_, openInt );

    if (( !stamp_.isValid() ) ||
        ( openInt.callTotalVolume.isEmpty() ) || ( openInt.putTotalVolume.isEmpty() ) ||
        ( openInt.callTotalVolume.size() != openInt.putTotalVolume.size() ))
    {
        LOG_WARN << "no strike price data for " << qPrintable( expiryDate_.toString() );
        return;
    }

    // iterate over each strike
    for ( QMap<double, int>::const_iterator i( openInt.callTotalVolume.constBegin() ); i != openInt.callTotalVolume.constEnd(); i++ )
    {
        const double strike( i.key() );

        // add data point for this strike
        for ( LinearEquationMap::const_iterator eq( payouts.constBegin() ); eq != payouts.constEnd(); eq++ )
        {
            if ( strike < eq.key() )
                break;

            returns_[strike] =  (eq.value().first * strike) + eq.value().second;
        }
    }

    // draw!
    drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsGraphWidget::paintEvent( QPaintEvent *e )
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
void OptionTradingReturnsGraphWidget::resizeEvent( QResizeEvent *e )
{
    // new graph
    drawGraph();

    _Mybase::resizeEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsGraphWidget::initialize()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsGraphWidget::createLayout()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant OptionTradingReturnsGraphWidget::modelData( int col ) const
{
    return model_->data( index_, col, Qt::UserRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionTradingReturnsGraphWidget::calcMinMaxValues( const ValuesMap& values, double& kmin, double& kmax, double& vmin, double& vmax ) const
{
    kmin = vmin = 99999999.99;
    kmax = vmax = -99999999.99;

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
void OptionTradingReturnsGraphWidget::calcIntervalValues( double min, double max, double gheight, double ints, double& interval, int& numDecimals ) const
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
void OptionTradingReturnsGraphWidget::drawGraph()
{
    // clear graph
    graph_ = QPixmap();

    // no data
    if ( returns_.isEmpty() )
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
    // y axis = return amount
    double xmin;
    double xmax;

    double ymin;
    double ymax;

    if ( !calcMinMaxValues( returns_, xmin, xmax, ymin, ymax ) )
    {
        LOG_WARN << "error calculating min/max values for graph";
        return;
    }
    else if ( xmax < xmin )
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
    int numDecimalPlacesReturns;

    calcIntervalValues( ymin, ymax, height(), 50.0, yinterval, numDecimalPlacesReturns );

    // graph constants
    xmin = xinterval * std::floor( xmin / xinterval );
    xmax = xinterval * std::ceil( xmax / xinterval );

    ymin = yinterval * std::floor( ymin / yinterval );
    ymax = yinterval * std::ceil( ymax / yinterval );

    const int marginWidth( SPACING +
        qMax( fm.boundingRect( QString::number( ymax, 'f', numDecimalPlacesReturns ) ).width() ,
              fm.boundingRect( QString::number( ymin, 'f', numDecimalPlacesReturns ) ).width() ) );

    const int marginHeight( SPACING + fm.boundingRect( "0123456789/:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" ).height() );

    // -----
    // graph
    // -----

    const QLocale l( QLocale::system() );

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

    // return value intervals
    painter.setPen( QPen( Qt::darkGray, 0 ) );

    for ( double i( ymin ); i <= ymax; i += yinterval )
    {
        const int y( gbottom - scaled( i, ymin, ymax, gbottom-gtop ) );

        painter.drawLine( gleft-2, y, gright, y );
        painter.drawText( 0, y-25, marginWidth-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( i, 'f', numDecimalPlacesReturns ) );
    }

    // strike prices
    painter.drawLine( gleft, gtop, gleft, gbottom );

    for ( double i( xmin ); i <= xmax; i += xinterval )
    {
        const int x( gleft + scaled( i, xmin, xmax, gright-gleft ) );

        painter.drawLine( x, gbottom, x, gbottom+2 );
        painter.drawText( x-4, gbottom+4, 50, 50, Qt::AlignLeft | Qt::AlignTop, QString::number( i, 'f', numDecimalPlacesStrike ) );
    }

    // returns
    int xprev( 0 );
    int yprev( 0 );

    double prev( 0.0 );

    bool ready( false );

    for ( ValuesMap::const_iterator i( returns_.constBegin() ); i != returns_.constEnd(); ++i )
    {
        const int x( gleft + scaled( i.key(), xmin, xmax, gright-gleft ) );
        const int y( gbottom - scaled( i.value(), ymin, ymax, gbottom-gtop ) );

        if ( !ready )
            ready = true;
        else
        {
            painter.setPen( QPen( (( i.value() < 0.0 ) || ( prev < 0.0 )) ? Qt::red : Qt::darkGreen, 2 ) );
            painter.drawLine( xprev, yprev, x, y );
        }

        xprev = x;
        yprev = y;

        prev = i.value();
    }

    // expected value
    const double ev( modelData( model_type::EXPECTED_VALUE ).toDouble() );

    if ( 0.0 != ev )
    {
        const int y( gbottom - scaled( ev, ymin, ymax, gbottom-gtop ) );

        painter.setPen( QPen( (ev < 0.0) ? Qt::red : Qt::darkGreen, 1, Qt::DashLine ) );

        painter.drawLine( gleft, y, gright, y );
        painter.drawText( 0, y-25, marginWidth-SPACING, 50, Qt::AlignRight | Qt::AlignVCenter, QString::number( ev, 'f', numDecimalPlacesReturns ) );
    }

    // price
    if ( 0.0 < underlyingPrice_ )
    {
        const int x( gleft + scaled( underlyingPrice_, xmin, xmax, gright-gleft ) );

        painter.setPen( QPen( palette().text().color(), 2, Qt::DashLine ) );

        painter.drawLine( x, gtop, x, gbottom );
    }

    // stamp
    painter.setPen( QPen( palette().text().color(), 0 ) );
    painter.drawText( 0, SPACING+4, gwidth, 50, Qt::AlignHCenter | Qt::AlignTop, stamp_.toString() );
/*
    // labels
    painter.setPen( QPen( penColor[0], 0 ) );
    painter.drawText( 0, SPACING+4, gwidth-SPACING, 50, Qt::AlignRight | Qt::AlignTop, tr( "CALLS" ) );

    painter.setPen( QPen( penColor[1], 0 ) );
    painter.drawText( 0, SPACING+4+marginHeight, gwidth-SPACING, 50, Qt::AlignRight | Qt::AlignTop, tr( "PUTS" ) );
*/
    painter.end();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int OptionTradingReturnsGraphWidget::scaled( double p, double min, double max, int height )
{
    return std::round( ((p - min) / (max - min)) * height );
}

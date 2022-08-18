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

#include <QComboBox>
#include <QHBoxLayout>
#include <QPainter>
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
    translateOverlays( overlays_ );
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
void OptionTradingReturnsGraphWidget::onCurrentIndexChanged( int index )
{
    Q_UNUSED( index )

    if ( overlays_ == sender() )
        drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsGraphWidget::initialize()
{
    overlays_ = new QComboBox( this );

    connect( overlays_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );

    //
    // overlays
    //

    overlays_->addItem( QString(), "NONE" );

    // hist vol (days to expiry)
    overlays_->addItem( QString(), "HVDTE" );

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
        overlays_->addItem( QString(), "HV" + hvdepth );

    // impl vol (days to expiry)
    overlays_->addItem( QString(), "IV" );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsGraphWidget::createLayout()
{
    QHBoxLayout *boxes( new QHBoxLayout() );
    boxes->addStretch();
    boxes->addWidget( overlays_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addLayout( boxes );
    form->addStretch();
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

    QString volatilityInfoStr;

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

    // overlays
    if ( 0 < overlays_->currentIndex() )
    {
        const QString data( overlays_->currentData().toString() );

        // trading days until expiration
        const double dte( AppDatabase::instance()->numTradingDaysUntil( QDateTime::fromString( modelData( model_type::EXPIRY_DATE ).toString(), Qt::ISODate ) ) );

        LOG_TRACE << "trading days to expiry " << dte;

        double vol( 0.0 );
        double volMin( 0.0 );
        double volMax( 0.0 );

        // graph expected movement based on historical volatility
        if ( data.startsWith( "HV" ) )
        {
            const QDate now( AppDatabase::instance()->currentDateTime().date() );

            int depth( std::ceil( dte ) );

            if ( "HVDTE" == data )
                vol = modelData( model_type::HIST_VOLATILITY ).toDouble() / 100.0;
            else if ( 2 < data.length() )
            {
                depth = QStringView{ data }.mid( 2 ).toInt();
                vol = SymbolDatabases::instance()->historicalVolatility( underlying_, now, depth );
            }

            SymbolDatabases::instance()->historicalVolatilityRange( underlying_, now.addDays( -HV_RANGE_DAYS ), now, depth, volMin, volMax );
        }
        // graph expected movement based on implied volatility
        else if ( data.startsWith( "IV" ) )
        {
            OptionChainCurves data;

            SymbolDatabases::instance()->optionChainCurves( underlying_, expiryDate_, data );

            // find lowest vol
            if ( data.volatility.isEmpty() )
                LOG_WARN << "no volatility info";
            else
            {
                double implVol( data.volatility.begin().value() );

                foreach ( const double& v, data.volatility )
                    if ( v < implVol )
                        implVol = v;

                LOG_TRACE << "impl vol " << implVol;
                vol = implVol;
            }
        }

        LOG_TRACE << "volatility " << vol << " min " << volMin << " max " << volMax;

        // draw
        painter.setPen( QPen( Qt::NoPen ) );

        // estimated movement
        if ( 0.0 < vol )
        {
            const double volp( vol * sqrt( dte / AppDatabase::instance()->numTradingDays() ) );
            const double estMovement( underlyingPrice_ * volp );

            QList<int> alpha;
            alpha.append( (16 * 255) / 100 );   // 1 sigma - 34.1% probability
            alpha.append( (12 * 255) / 100 );   // 2 sigma - 13.6% probability
            alpha.append( (8 * 255) / 100 );    // 3 sigma -  2.1% probability

            for ( int sigma( 3 ); 0 < sigma; --sigma )
            {
                int xleft( gleft + scaled( underlyingPrice_ - estMovement * sigma, xmin, xmax, gright-gleft ) );
                int xright( gleft + scaled( underlyingPrice_ + estMovement * sigma, xmin, xmax, gright-gleft ) );

                xleft = qMax( xleft, gleft );
                xright = qMin( xright, gright );

                QColor c( Qt::red );
                c.setAlpha( alpha[sigma-1] );

                painter.setBrush( QBrush( c, Qt::SolidPattern ) );
                painter.drawRect( xleft, gtop, xright - xleft, gbottom - gtop );
            }

            volatilityInfoStr = QString( "%1% (+/- %2)")
                .arg( l.toString( vol * 100.0, 'f', 2 ) )
                .arg( l.toString( estMovement, 'f', 3 ) );
        }

        // minimum movement
        if ( 0.0 < volMin )
        {
            const double volp( volMin * sqrt( dte / AppDatabase::instance()->numTradingDays() ) );
            const double estMovement( underlyingPrice_ * volp );

            const int xleft( gleft + scaled( underlyingPrice_ - estMovement, xmin, xmax, gright-gleft ) );
            const int xright( gleft + scaled( underlyingPrice_ + estMovement, xmin, xmax, gright-gleft ) );

            QColor c( Qt::darkRed );
            c.setAlpha( 128 );

            painter.setBrush( QBrush( c, Qt::SolidPattern ) );
            painter.drawRect( xleft, gtop, xright - xleft, gbottom - gtop );
        }

        // maximum movement
        if ( 0.0 < volMax )
        {
            const double volp( volMax * sqrt( dte / AppDatabase::instance()->numTradingDays() ) );
            const double estMovement( underlyingPrice_ * volp );

            const int xleft( gleft + scaled( underlyingPrice_ - estMovement, xmin, xmax, gright-gleft ) );
            const int xright( gleft + scaled( underlyingPrice_ + estMovement, xmin, xmax, gright-gleft ) );

            painter.setPen( QPen( palette().text().color(), 1, Qt::DashLine ) );
            painter.drawLine( xleft, gtop, xleft, gbottom - gtop );
            painter.drawLine( xright, gtop, xright, gbottom - gtop );
        }

    } // overlay selected

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

        const QString exString( QString::number( ev, 'f', numDecimalPlacesReturns ) );

        QRect r( fm.boundingRect( exString ) );
        r.moveTo( gleft - r.width()/2, y - r.height()/2 );

        painter.setBrush( (ev < 0.0) ? Qt::red : Qt::darkGreen );
        painter.setPen( (ev < 0.0) ? Qt::red : Qt::darkGreen );
        painter.drawRect( r );

        painter.setPen( Qt::white );
        painter.drawText( r, Qt::AlignCenter, exString );
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

    // volatility info
    if ( volatilityInfoStr.length() )
    {
        painter.setPen( QPen( palette().text().color(), 0 ) );
        painter.drawText( gright-250, gbottom-50, 250, 50, Qt::AlignRight | Qt::AlignBottom, volatilityInfoStr );
    }

    painter.end();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int OptionTradingReturnsGraphWidget::scaled( double p, double min, double max, int height )
{
    return std::round( ((p - min) / (max - min)) * height );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingReturnsGraphWidget::translateOverlays( QComboBox *w )
{
    for ( int i( 0 ); i < w->count(); ++i )
    {
        const QString data( w->itemData( i ).toString() );

        QString text;

        if ( "NONE" == data )
            text = tr( "OVERLAYS" );
        else if ( "HVDTE" == data )
            text = tr( "HV(DTE)" );
        else if ( data.startsWith( "HV" ) )
            text = tr( "HV(%0)" ).arg( QStringView{ data }.mid( 2 ) );
        else if ( "IV" == data )
            text = tr( "IV" );

        w->setItemText( i, text );
    }
}


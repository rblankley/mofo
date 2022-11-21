/**
 * @file symbolimplvolwidget.cpp
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
#include "symbolimplvolwidget.h"

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
SymbolImpliedVolatilityWidget::SymbolImpliedVolatilityWidget( const QString& symbol, double price, QWidget *parent ) :
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
SymbolImpliedVolatilityWidget::~SymbolImpliedVolatilityWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolImpliedVolatilityWidget::translate()
{
    QAbstractItemModel *model( dates_->model() );

    // update root item
    model->setData( model->index( 0, 0 ), tr( "EXPIRY" ), Qt::DisplayRole );

    // update expiration items
    for ( int i( 1 ); i < model->rowCount(); ++i )
    {
        const QModelIndex index( model->index( i, 0 ) );

        const QDate date( model->data( index, Qt::UserRole ).toDate() );
        const QString text( date.toString() );

        model->setData( index, text, Qt::DisplayRole );
        model->setData( index, dateColor( text ), Qt::ForegroundRole );
    }

    // adjust view width to fit contents
    // add room for check box
    dates_->view()->setMinimumWidth(
        24 + dates_->view()->sizeHintForColumn( 0 ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolImpliedVolatilityWidget::refreshData()
{
    curves_.clear();

    // fetch most recent expiration dates with curve data
    LOG_TRACE << "fetch curve expiry dates...";

    expiryDates_.clear();
    stamp_ = SymbolDatabases::instance()->optionChainCurveExpirationDates( symbol(), expiryDates_, QDateTime(), AppDatabase::instance()->currentDateTime() );

    if (( !stamp_.isValid() ) || ( expiryDates_.isEmpty() ))
    {
        LOG_WARN << "no curve expiry dates found";
        return;
    }

    // fetch curve data
    LOG_TRACE << "fetch curves...";

    foreach ( const QDate& d, expiryDates_ )
    {
        OptionChainCurves data;

        SymbolDatabases::instance()->optionChainCurves( symbol(), d, data, stamp_, stamp_ );

        if ( data.volatility.isEmpty() )
            LOG_WARN << "no volatility curve for " << qPrintable( d.toString() );
        else
        {
            curves_[d] = data;
        }
    }

    if ( curves_.isEmpty() )
    {
        LOG_WARN << "no curves found";
        return;
    }

    // check for existing model
    QAbstractItemModel *doomed( dates_->model() );

    if ( doomed )
        delete doomed;

    // populate model
    QStandardItemModel *datesModel( new QStandardItemModel( 0, 1, this ) );
    QStandardItem *item;

    item = new QStandardItem( QString() );
    datesModel->appendRow( item );

    foreach ( const QDate& date, curves_.keys() )
    {
        item = new QStandardItem();
        item->setData( date, Qt::UserRole );
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        item->setCheckState( Qt::Checked );
        datesModel->appendRow( item );
    }

    dates_->setModel( datesModel );

    connect( datesModel, &QStandardItemModel::itemChanged, this, &_Myt::onItemChanged );

    // show expiry dates
    dates_->setVisible( true );
    translate();

    // draw!
    drawGraph();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolImpliedVolatilityWidget::paintEvent( QPaintEvent *e )
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
void SymbolImpliedVolatilityWidget::resizeEvent( QResizeEvent *e )
{
    // new graph
    drawGraph();

    _Mybase::resizeEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolImpliedVolatilityWidget::onItemChanged( QStandardItem *item )
{
    Q_UNUSED( item )

    // update graph
    drawGraph();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolImpliedVolatilityWidget::initialize()
{
    dates_ = new QComboBox( this );
    dates_->setVisible( false );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SymbolImpliedVolatilityWidget::createLayout()
{
    QHBoxLayout *boxes( new QHBoxLayout() );
    boxes->addStretch();
    boxes->addWidget( dates_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addLayout( boxes );
    form->addStretch();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SymbolImpliedVolatilityWidget::calcMinMaxValues( const ValuesMap& values, double& kmin, double& kmax, double& vmin, double& vmax ) const
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
void SymbolImpliedVolatilityWidget::calcIntervalValues( double min, double max, double gheight, double ints, double& interval, int& numDecimals ) const
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
void SymbolImpliedVolatilityWidget::drawGraph()
{
    // clear graph
    graph_ = QPixmap();

    // no data
    if ( curves_.isEmpty() )
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

    {
        const QAbstractItemModel *model( dates_->model() );
        const QModelIndexList indexes( model->match( model->index( 1, 0 ), Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly ) );

        foreach ( const QModelIndex& index, indexes )
        {
            const QDate date( model->data( index, Qt::UserRole ).toDate() );

            const QMap<double, double> *values( &curves_[date].volatility );

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

    // expiry dates
    // draw curves from furthest expiry to closest expiry
    QList<QDate> dates;

    {
        const QAbstractItemModel *model( dates_->model() );
        const QModelIndexList indexes( model->match( model->index( 1, 0 ), Qt::CheckStateRole, Qt::Checked, -1, Qt::MatchExactly ) );

        foreach ( const QModelIndex& index, indexes )
        {
            const QDate date( model->data( index, Qt::UserRole ).toDate() );

            dates.prepend( date );
        }
    }

    foreach ( const QDate& date, dates )
    {
        const QMap<double, double> *values( &curves_[date].volatility );

        const QString dstr( date.toString() );

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

            painter.setPen( QPen( dateColor( dstr ), 0, solid ? Qt::SolidLine : Qt::DotLine ) );

            if ( 0.0 < xprev )
                painter.drawLine( xprev, yprev, x, y );

            xprev = x;
            yprev = y;

            solid = true;
        }
    }

    // price
    if ( 0.0 < price_ )
    {
        const int x( gleft + scaled( price_, xmin, xmax, gright-gleft ) );

        painter.setPen( QPen( palette().text().color(), 2, Qt::DashLine ) );

        painter.drawLine( x, gtop, x, gbottom );
    }

    // stamp
    const QDateTime now( AppDatabase::instance()->currentDateTime() );

    if ( stamp_.daysTo( now ) <= 0 )
        painter.setPen( QPen( palette().text().color(), 0 ) );
    else if ( stamp_.daysTo( now ) <= 7 )
        painter.setPen( QPen( QColor( 255, 165, 0 ), 0 ) ); // orange
    else
        painter.setPen( QPen( Qt::red, 0 ) );

    painter.drawText( 0, SPACING+4, gwidth, 50, Qt::AlignHCenter | Qt::AlignTop, stamp_.toString() );

    painter.end();

    // queue paint event
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int SymbolImpliedVolatilityWidget::scaled( double p, double min, double max, int height )
{
    return std::round( ((p - min) / (max - min)) * height );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QColor SymbolImpliedVolatilityWidget::dateColor( const QString& desc )
{
    QCryptographicHash h( QCryptographicHash::Md5 );
    h.addData( desc.toLatin1() );

    const QByteArray a( h.result() );

    return QColor( (unsigned char) a[0], (unsigned char) a[1], (unsigned char) a[2] );
}

/**
 * @file optiontradingview.cpp
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

#include "common.h"
#include "optiontradingview.h"
#include "gridtableheaderview.h"
#include "hoveritemdelegate.h"

#include "db/appdb.h"
#include "db/optiontradingitemmodel.h"

#include <QAction>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QVBoxLayout>

static const QString HEADER_STATE( "optionTradingViewHeaderState" );

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionTradingView::OptionTradingView( model_type *model, QWidget *parent ) :
    _Mybase( parent ),
    model_( model ),
    prevRow_( -1 )
{
    // take ownership of model
    model->setParent( this );

    // init
    initialize();
    createLayout();
    translate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionTradingView::~OptionTradingView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::translate()
{
    GridTableHeaderView *hheader( gridHeaderView( Qt::Horizontal ) );

    if ( hheader )
    {
        for ( int column( model_type::_NUM_COLUMNS ); column--; )
            hheader->setCellLabel( 0, column, columnHeaderText( column ) );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::mouseMoveEvent( QMouseEvent *event )
{
    // detect where the mouse cursor is relative to our table
    const QModelIndex idx( indexAt( event->pos() ) );

    // check update needed
    if ( prevRow_ != idx.row() )
    {
        // clear out old hover region
        if ( 0 <= prevRow_ )
        {
            for ( int column( model_type::_NUM_COLUMNS ); column--; )
                update( model_->index( prevRow_, column ) );
        }

        // set hover region
        emit setHoverRegion( prevRow_ = idx.row(), 0,  (model_type::_NUM_COLUMNS - 1) );

        // highlight new hover region
        for ( int column( model_type::_NUM_COLUMNS ); column--; )
            update( model_->index( idx.row(), column ) );
    }

    _Mybase::mouseMoveEvent( event );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::leaveEvent( QEvent *event )
{
    // detect when the mouse cursor leaves our table
    emit clearHoverRegion();

    // clear out old hover region
    if ( 0 <= prevRow_ )
    {
        for ( int column( model_type::_NUM_COLUMNS ); column--; )
            update( model_->index( prevRow_, column ) );

        prevRow_ = -1;
    }

    _Mybase::leaveEvent( event );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::onHeaderSectionMoved( int logicalIndex, int oldVisualIndex, int newVisualIndex )
{
    const QString logical( columnHeaderText( logicalIndex ) );

    LOG_DEBUG << "moving column " << logicalIndex << " '" << qPrintable( logical ) << "' from " << oldVisualIndex << " to " << newVisualIndex;

    GridTableHeaderView *hheader( qobject_cast<GridTableHeaderView*>( sender() ) );

    if ( !hheader )
        return;

    saveHeaderState( hheader );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::onHeaderSectionPressed( const QPoint& pos, Qt::MouseButton button, int from, int to )
{
    Q_UNUSED( to )

    if ( Qt::RightButton != button )
        return;

    GridTableHeaderView *hheader( qobject_cast<GridTableHeaderView*>( sender() ) );

    if ( !hheader )
        return;

    // ---- //

    QMap<QAction*, int> columnMap;

    QMenu contextMenu;
    QAction *a;

    // hide whats underneath cursor
    a = contextMenu.addAction( tr( "&Hide" ) + " " + columnHeaderText( from ) );
    columnMap[a] = from;

    QAction *showAll( contextMenu.addAction( tr( "Show &All Columns" ) ) );

    contextMenu.addAction( tr( "&Cancel" ) );
    contextMenu.addSeparator();

    // show all columns
    for ( int i( 0 ); i < model_type::_NUM_COLUMNS; ++i )
    {
        a = contextMenu.addAction( columnHeaderText( i ) );
        a->setCheckable( true );
        a->setChecked( !isColumnHidden( i ) );

        columnMap[a] = i;
    }

    // show context menu
    a = contextMenu.exec( hheader->mapToGlobal( pos ) );

    // check result
    if ( showAll == a )
    {
        for ( int i( 0 ); i < model_type::_NUM_COLUMNS; ++i )
            setColumnHidden( i, false );
    }
    else if ( columnMap.contains( a ) )
    {
        const int column( columnMap[a] );

        bool hide( true );

        if ( a->isCheckable() )
            hide = !a->isChecked();

        setColumnHidden( column, hide );
    }
    else
    {
        return;
    }

    saveHeaderState( hheader );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::onHeaderSectionResized( int logicalIndex, int oldSize, int newSize )
{
    Q_UNUSED( logicalIndex );
    Q_UNUSED( oldSize );
    Q_UNUSED( newSize );

    GridTableHeaderView *hheader( qobject_cast<GridTableHeaderView*>( sender() ) );

    if ( !hheader )
        return;

    saveHeaderState( hheader );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::initialize()
{
    // item delegate
    itemDelegate_ = new HoverItemDelegate( this );

    connect( this, &_Myt::setHoverRegion, itemDelegate_, &HoverItemDelegate::setHoverRegion );
    connect( this, &_Myt::clearHoverRegion, itemDelegate_, &HoverItemDelegate::clearHoverRegion );

    // table view
    setModel( model_ );

    setSelectionMode( QAbstractItemView::NoSelection );
    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    setEditTriggers( QAbstractItemView::NoEditTriggers );

    setGridHeaderView( Qt::Horizontal, 1 );

    verticalHeader()->setDefaultSectionSize( DEFAULT_HEIGHT );
    verticalHeader()->hide();

    setItemDelegate( itemDelegate_ );
    setMouseTracking( true );

    // table view header
    GridTableHeaderView *hheader( gridHeaderView( Qt::Horizontal ) );

    if ( hheader )
    {
        hheader->setDefaultSectionSize( DEFAULT_WIDTH );
        hheader->setSectionsMovable( true );

        // restore state
        restoreHeaderState( hheader );

        connect( hheader, &GridTableHeaderView::sectionMoved, this, &_Myt::onHeaderSectionMoved );
        connect( hheader, &GridTableHeaderView::sectionPressed, this, &_Myt::onHeaderSectionPressed );
        connect( hheader, &GridTableHeaderView::sectionResized, this, &_Myt::onHeaderSectionResized );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::createLayout()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString OptionTradingView::columnHeaderText( int column ) const
{
    switch ( column )
    {
    case model_type::STAMP:
        return tr( "Stamp" );
    case model_type::UNDERLYING:
        return tr( "Underlying" );
    case model_type::TYPE:
        return tr( "P/C" );

    case model_type::STRATEGY:
        return tr( "Strategy" );
    case model_type::STRATEGY_DESC:
        return tr( "Strat. Desc" );

    // Option Chain Information
    case model_type::SYMBOL:
        return tr( "Symbol" );
    case model_type::DESC:
        return tr( "Description" );
    case model_type::BID_ASK_SIZE:
        return tr( "B/A Size" );
    case model_type::BID_PRICE:
        return tr( "Bid" );
    case model_type::BID_SIZE:
        return tr( "Bid Size" );
    case model_type::ASK_PRICE:
        return tr( "Ask" );
    case model_type::ASK_SIZE:
        return tr( "Ask Size" );
    case model_type::LAST_PRICE:
        return tr( "Last" );
    case model_type::LAST_SIZE:
        return tr( "Last Size" );
    case model_type::BREAK_EVEN_PRICE:
        return tr( "Break Even" );
    case model_type::INTRINSIC_VALUE:
        return tr( "Int. Value" );
    case model_type::OPEN_PRICE:
        return tr( "Open" );
    case model_type::HIGH_PRICE:
        return tr( "High" );
    case model_type::LOW_PRICE:
        return tr( "Low" );
    case model_type::CLOSE_PRICE:
        return tr( "Close" );
    case model_type::CHANGE:
        return tr( "Change" );
    case model_type::PERCENT_CHANGE:
        return tr( "% Change" );
    case model_type::TOTAL_VOLUME:
        return tr( "Volume" );
    case model_type::QUOTE_TIME:
        return tr( "Quote Time" );
    case model_type::TRADE_TIME:
        return tr( "Trade Time" );
    case model_type::MARK:
        return tr( "Mark" );
    case model_type::MARK_CHANGE:
        return tr( "Mark Chg." );
    case model_type::MARK_PERCENT_CHANGE:
        return tr( "Mark % Chg." );
    case model_type::EXCHANGE_NAME:
        return tr( "Exchange" );
    case model_type::VOLATILITY:
        return tr( "Volatility" );
    case model_type::DELTA:
        return tr( "Delta" );
    case model_type::GAMMA:
        return tr( "Gamma" );
    case model_type::THETA:
        return tr( "Theta" );
    case model_type::VEGA:
        return tr( "Vega" );
    case model_type::RHO:
        return tr( "Rho" );
    case model_type::TIME_VALUE:
        return tr( "Time Value" );
    case model_type::OPEN_INTEREST:
        return tr( "Open Int" );
    case model_type::IS_IN_THE_MONEY:
        return tr( "In The Money" );
    case model_type::THEO_OPTION_VALUE:
        return tr( "Theo. Value" );
    case model_type::THEO_VOLATILITY:
        return tr( "Theo. Vol." );
    case model_type::IS_MINI:
        return tr( "Is Mini" );
    case model_type::IS_NON_STANDARD:
        return tr( "Is Non-Std" );
    case model_type::IS_INDEX:
        return tr( "Is Index" );
    case model_type::IS_WEEKLY:
        return tr( "Is Weekly" );
    case model_type::IS_QUARTERLY:
        return tr( "Is Quarterly" );
    case model_type::EXPIRY_DATE:
        return tr( "Exp. Date" );
    case model_type::EXPIRY_TYPE:
        return tr( "Exp. Type" );
    case model_type::DAYS_TO_EXPIRY:
        return tr( "Days to Exp." );
    case model_type::LAST_TRADING_DAY:
        return tr( "Last Tr. Day" );
    case model_type::MULTIPLIER:
        return tr( "Multiplier" );
    case model_type::SETTLEMENT_TYPE:
        return tr( "Sett. Type" );
    case model_type::DELIVERABLE_NOTE:
        return tr( "Deliv. Note" );
    case model_type::STRIKE_PRICE:
        return tr( "Strike" );

    // Calculated Fields
    case model_type::HIST_VOLATILITY:
        return tr( "Hist. Vol" );

    case model_type::TIME_TO_EXPIRY:
        return tr( "Time To Exp." );
    case model_type::RISK_FREE_INTEREST_RATE:
        return tr( "Risk Free Rate" );

    case model_type::CALC_BID_PRICE_VI:
        return tr( "Calc. Bid VI" );
    case model_type::CALC_ASK_PRICE_VI:
        return tr( "Calc. Ask VI" );
    case model_type::CALC_MARK_VI:
        return tr( "Calc. Mark VI" );

    case model_type::CALC_THEO_OPTION_VALUE:
        return tr( "Calc. Theo. Value" );
    case model_type::CALC_THEO_VOLATILITY:
        return tr( "Calc. Theo. VI" );

    case model_type::CALC_DELTA:
        return tr( "Calc. Delta" );
    case model_type::CALC_GAMMA:
        return tr( "Calc. Gamma" );
    case model_type::CALC_THETA:
        return tr( "Calc. Theta" );
    case model_type::CALC_VEGA:
        return tr( "Calc. Vega" );
    case model_type::CALC_RHO:
        return tr( "Calc. Rho" );

    case model_type::BID_ASK_SPREAD:
        return tr( "B/A Spread" );
    case model_type::BID_ASK_SPREAD_PERCENT:
        return tr( "B/A Spread %" );

    case model_type::PROBABILITY_ITM:
        return tr( "Prob. ITM" );
    case model_type::PROBABILITY_OTM:
        return tr( "Prob. OTM" );

    default:
        break;
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::saveHeaderState( const QHeaderView *view )
{
    AppDatabase::instance()->setHeaderState( HEADER_STATE, view->saveState() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingView::restoreHeaderState( QHeaderView *view )
{
    const QByteArray a( AppDatabase::instance()->headerState( HEADER_STATE ) );

    if ( a.isNull() )
        return;

    view->restoreState( a );
}

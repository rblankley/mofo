/**
 * @file optionchainview.cpp
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
#include "optionchainview.h"
#include "gridtableheaderview.h"
#include "hoveritemdelegate.h"

#include "db/appdb.h"
#include "db/optionchaintablemodel.h"

#include <QAction>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QVBoxLayout>

const QString OptionChainView::STATE_GROUP_NAME( "optionChainView" );
const QString OptionChainView::STATE_NAME( "[[default]]" );

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionChainView::OptionChainView( model_type *model, QWidget *parent ) :
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
OptionChainView::~OptionChainView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString OptionChainView::title() const
{
    const bool isWeekly( model_->data0( model_type::CALL_IS_WEEKLY ).toBool() );
    const bool isQuarterly( model_->data0( model_type::CALL_IS_QUARTERLY ).toBool() );

    // scale all rows for non-standard options
    bool isNonStandard( false );

    for ( int row( 0 ); row < model_->rowCount(); ++row )
        if ( model_->data( row, model_type::CALL_IS_NON_STANDARD ).toBool() )
        {
            isNonStandard = true;
            break;
        }

    const QDate expiry( model_->expirationDate() );
    const int daysToExpiry( -expiry.daysTo( QDate::currentDate() ) );

    QString result( QString( "%0 (%1)" ).arg( expiry.toString( "dd MMM yy" ).toUpper() ).arg( 0 <= daysToExpiry ? QString::number( daysToExpiry ) : "EXP" ) );

    if ( isWeekly )
        result.append( " W" );
    else if ( isQuarterly )
        result.append( " Q" );

    else if ( isNonStandard )
        result.append( " NS" );

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::translate()
{
    GridTableHeaderView *hheader( gridHeaderView( Qt::Horizontal ) );

    if ( hheader )
    {
        hheader->setCellLabel( 0, model_type::_CALL_COLUMNS_BEGIN, tr( "CALLS" ) );
        hheader->setCellLabel( 0, model_type::_PUT_COLUMNS_BEGIN, tr( "PUTS" ) );
        hheader->setCellLabel( 0, model_type::STRIKE_PRICE, tr( "STRIKE" ) );

        for ( int column( model_type::_NUM_COLUMNS ); column--; )
            hheader->setCellLabel( 1, column, columnHeaderText( column ) );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::mouseMoveEvent( QMouseEvent *event )
{
    // detect where the mouse cursor is relative to our table
    const QModelIndex idx( indexAt( event->pos() ) );

    int from;
    int to;

    if ( model_->isColumnCallOption( (model_type::ColumnIndex) idx.column() ) )
    {
        from = model_type::_CALL_COLUMNS_BEGIN;
        to = model_type::_CALL_COLUMNS_END;
    }
    else if ( model_->isColumnPutOption( (model_type::ColumnIndex) idx.column() ) )
    {
        from = model_type::_PUT_COLUMNS_BEGIN;
        to = model_type::_PUT_COLUMNS_END;
    }
    else
    {
        from = to = idx.column();
    }

    // check update needed
    if (( prevRow_ != idx.row() ) || ( prevColFrom_ != from ))
    {
        // clear out old hover region
        if ( 0 <= prevRow_ )
        {
            do
            {
                update( model_->index( prevRow_, prevColTo_-- ) );
            } while ( prevColFrom_ <= prevColTo_ );
        }

        // set hover region
        emit setHoverRegion( prevRow_ = idx.row(), prevColFrom_ = from, prevColTo_ = to );

        // highlight new hover region
        do
        {
            update( model_->index( idx.row(), to-- ) );
        } while ( from <= to );
    }

    _Mybase::mouseMoveEvent( event );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::leaveEvent( QEvent *event )
{
    // detect when the mouse cursor leaves our table
    emit clearHoverRegion();

    // clear out old hover region
    if ( 0 <= prevRow_ )
    {
        do
        {
            update( model_->index( prevRow_, prevColTo_-- ) );
        } while ( prevColFrom_ <= prevColTo_ );

        prevRow_ = -1;
    }

    _Mybase::leaveEvent( event );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::onHeaderSectionMoved( int logicalIndex, int oldVisualIndex, int newVisualIndex )
{
    const QString logical( columnHeaderText( logicalIndex ) );

    LOG_DEBUG << "moving column " << logicalIndex << " '" << qPrintable( logical ) << "' from " << oldVisualIndex << " to " << newVisualIndex;

    GridTableHeaderView *hheader( qobject_cast<GridTableHeaderView*>( sender() ) );

    if ( !hheader )
        return;

    // only allow call columns to move within calls
    // only allow put columns to move within puts
    const bool isCall( model_->isColumnCallOption( (model_type::ColumnIndex) logicalIndex ) );
    const bool isPut( model_->isColumnPutOption( (model_type::ColumnIndex) logicalIndex ) );

    bool allowed(( isCall ) || ( isPut ));

    if (( isCall ) && ( !model_->isColumnCallOption( (model_type::ColumnIndex) newVisualIndex ) ))
        allowed = false;
    else if (( isPut ) && ( !model_->isColumnPutOption( (model_type::ColumnIndex) newVisualIndex ) ))
        allowed = false;

    if ( allowed )
    {
        saveHeaderState( hheader );
        return;
    }

    LOG_WARN << "move not allowed";

    // move back
    hheader->blockSignals( true );
    hheader->moveSection( newVisualIndex, oldVisualIndex );
    hheader->blockSignals( false );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::onHeaderSectionPressed( const QPoint& pos, Qt::MouseButton button, int from, int to )
{
    if ( Qt::RightButton != button )
        return;

    GridTableHeaderView *hheader( qobject_cast<GridTableHeaderView*>( sender() ) );

    if ( !hheader )
        return;

    // ----------------------
    // create menu of actions
    // ----------------------

    const bool isNonSpannedColumn( (from == to) );

    QHash<QAction*, int> columnMap;
    QHash<QAction*, QString> headerStateMap;

    QMenu contextMenu;
    QAction *a;

    // hide whats underneath cursor
    if ( isNonSpannedColumn )
        if (( model_->isColumnCallOption( (model_type::ColumnIndex) from ) ) || ( model_->isColumnPutOption( (model_type::ColumnIndex) from ) ))
        {
            a = contextMenu.addAction( QIcon( ":/res/hide.png" ), tr( "&Hide" ) + " \"" + columnHeaderText( from ) + "\"" );
            columnMap[a] = from;
        }

    // show all columns
    QAction *showAll( contextMenu.addAction( QIcon( ":/res/view.png" ), tr( "Sho&w All Columns" ) ) );

    // resize column to content
    QAction *resizeColumn( nullptr );

    if ( isNonSpannedColumn )
        resizeColumn = contextMenu.addAction( QIcon( ":/res/width.png" ), tr( "Resi&ze" ) + " \"" + columnHeaderText( from ) + "\" " + tr( "to Content" ) );

    // resize all column to content
    QAction *resizeAllColumns( contextMenu.addAction( QIcon(), tr( "Resize All Co&lumns to Content" ) ) );

    // save state as...
    QAction *saveStateAs( contextMenu.addAction( QIcon( ":/res/disk.png" ), tr( "Save Layou&t As..." ) ) );

    // save state
    QAction *saveState( nullptr );

    if ( currentState_.length() )
        saveState = contextMenu.addAction( QIcon( ":/res/inbox.png" ), tr( "&Save" ) + " \"" + currentState_ + "\"" );

    // restore state
    const QStringList states( AppDatabase::instance()->widgetStates( AppDatabase::HeaderView, STATE_GROUP_NAME ) );

    if ( states.size() )
    {
        QMenu *restoreState( contextMenu.addMenu( QIcon( ":/res/outbox.png" ), tr( "&Restore Layout" ) ) );

        foreach ( const QString& state, states )
        {
            a = restoreState->addAction( QIcon(), state );
            headerStateMap[a] = state;
        }
    }

    // reset state
    QAction *reset( contextMenu.addAction( QIcon(), tr( "R&eset Layout to Default" ) ) );

    // cancel
    contextMenu.addAction( QIcon( ":/res/cancel.png" ), tr( "&Cancel" ) );

    contextMenu.addSeparator();

    // show/hide column
    for ( int i( model_type::_CALL_COLUMNS_BEGIN ); i <= model_type::_CALL_COLUMNS_END; ++i )
    {
        a = contextMenu.addAction( columnHeaderText( i ) );
        a->setCheckable( true );
        a->setChecked( !isColumnHidden( i ) );

        columnMap[a] = i;
    }

    // ---------
    // show menu
    // ---------

    // show context menu
    a = contextMenu.exec( hheader->mapToGlobal( pos ) );

    // ---------------------
    // process menu response
    // ---------------------

    // show all columns
    if ( showAll == a )
    {
        LOG_TRACE << "show all columns";

        for ( int i( model_type::_CALL_COLUMNS_BEGIN ); i < model_type::_CALL_COLUMNS_END; ++i )
        {
            setColumnHidden( i, false );
            setColumnHidden( model_->mappedColumn( (model_type::ColumnIndex) i ), false );
        }
    }

    // show/hide column
    else if ( columnMap.contains( a ) )
    {
        const int callColumn( columnMap[a] );
        const int putColumn( model_->mappedColumn( (model_type::ColumnIndex) callColumn ) );

        bool hide( true );

        if ( a->isCheckable() )
            hide = !a->isChecked();

        LOG_TRACE << "set columns " << callColumn << " " << putColumn << " hidden " << hide;

        setColumnHidden( callColumn, hide );
        setColumnHidden( putColumn, hide );
    }

    // resize column to contents
    else if (( resizeColumn ) && ( resizeColumn == a ))
    {
        resizeColumnToContents( from );
    }

    // resize all columns to contents
    else if ( resizeAllColumns == a )
    {
        resizeColumnsToContents();
    }

    // save state as...
    else if ( saveStateAs == a )
    {
        bool okay;

        const QString name(
            QInputDialog::getText(
                this,
                tr( "Enter Layout Name" ),
                tr( "Please enter a name for this layout:" ),
                QLineEdit::Normal,
                QString(),
                &okay ) );

        if (( okay ) && ( name.length() ))
            saveHeaderState( hheader, (currentState_ = name) );

        return;
    }

    // save state
    else if (( saveState ) && ( saveState == a ))
    {
        saveHeaderState( hheader, currentState_ );
        return;
    }

    // restore state
    else if ( headerStateMap.contains( a ) )
    {
        restoreHeaderState( hheader, (currentState_ = headerStateMap[a]) );
    }

    // reset state
    else if ( reset == a )
    {
        resetHeaderState( hheader );
    }

    // cancel
    else
    {
        return;
    }

    saveHeaderState( hheader );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::onHeaderSectionResized( int logicalIndex, int oldSize, int newSize )
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
void OptionChainView::initialize()
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

    setGridHeaderView( Qt::Horizontal, 2 );

    verticalHeader()->setDefaultSectionSize( DEFAULT_HEIGHT );
    verticalHeader()->hide();

    setItemDelegate( itemDelegate_ );
    setMouseTracking( true );

    // hide columns
    setColumnHidden( model_type::STAMP, true );
    setColumnHidden( model_type::UNDERLYING, true );
    setColumnHidden( model_type::EXPIRY_DATE, true );

    // table view header
    GridTableHeaderView *hheader( gridHeaderView( Qt::Horizontal ) );

    if ( hheader )
    {
        hheader->setDefaultSectionSize( DEFAULT_WIDTH );
        hheader->setColumnWidth( model_type::STRIKE_PRICE, STRIKE_COLUMN_WIDTH );

        hheader->setSpan( 0, model_type::_CALL_COLUMNS_BEGIN, 0, model_type::_CALL_WIDTH );
        hheader->setSpan( 0, model_type::_PUT_COLUMNS_BEGIN, 0, model_type::_PUT_WIDTH );

        hheader->setSpan( 0, model_type::STRIKE_PRICE, 2, 0 );

        hheader->setSectionsMovable( true );

        // restore state
        restoreHeaderState( hheader );

        connect( hheader, &GridTableHeaderView::sectionMoved, this, &_Myt::onHeaderSectionMoved );
        connect( hheader, &GridTableHeaderView::sectionPressed, this, &_Myt::onHeaderSectionPressed );
        connect( hheader, &GridTableHeaderView::sectionResized, this, &_Myt::onHeaderSectionResized );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::createLayout()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString OptionChainView::columnHeaderText( int column ) const
{
    switch ( column )
    {
    case model_type::CALL_SYMBOL:
    case model_type::PUT_SYMBOL:
        return tr( "Symbol" );
    case model_type::CALL_DESC:
    case model_type::PUT_DESC:
        return tr( "Description" );
    case model_type::CALL_BID_ASK_SIZE:
    case model_type::PUT_BID_ASK_SIZE:
        return tr( "B/A Size" );
    case model_type::CALL_BID_PRICE:
    case model_type::PUT_BID_PRICE:
        return tr( "Bid" );
    case model_type::CALL_BID_SIZE:
    case model_type::PUT_BID_SIZE:
        return tr( "Bid Size" );
    case model_type::CALL_ASK_PRICE:
    case model_type::PUT_ASK_PRICE:
        return tr( "Ask" );
    case model_type::CALL_ASK_SIZE:
    case model_type::PUT_ASK_SIZE:
        return tr( "Ask Size" );
    case model_type::CALL_LAST_PRICE:
    case model_type::PUT_LAST_PRICE:
        return tr( "Last" );
    case model_type::CALL_LAST_SIZE:
    case model_type::PUT_LAST_SIZE:
        return tr( "Last Size" );
    case model_type::CALL_BREAK_EVEN_PRICE:
    case model_type::PUT_BREAK_EVEN_PRICE:
        return tr( "Break Even" );
    case model_type::CALL_INTRINSIC_VALUE:
    case model_type::PUT_INTRINSIC_VALUE:
        return tr( "Int. Value" );
    case model_type::CALL_OPEN_PRICE:
    case model_type::PUT_OPEN_PRICE:
        return tr( "Open" );
    case model_type::CALL_HIGH_PRICE:
    case model_type::PUT_HIGH_PRICE:
        return tr( "High" );
    case model_type::CALL_LOW_PRICE:
    case model_type::PUT_LOW_PRICE:
        return tr( "Low" );
    case model_type::CALL_CLOSE_PRICE:
    case model_type::PUT_CLOSE_PRICE:
        return tr( "Close" );
    case model_type::CALL_CHANGE:
    case model_type::PUT_CHANGE:
        return tr( "Change" );
    case model_type::CALL_PERCENT_CHANGE:
    case model_type::PUT_PERCENT_CHANGE:
        return tr( "% Change" );
    case model_type::CALL_TOTAL_VOLUME:
    case model_type::PUT_TOTAL_VOLUME:
        return tr( "Volume" );
    case model_type::CALL_QUOTE_TIME:
    case model_type::PUT_QUOTE_TIME:
        return tr( "Quote Time" );
    case model_type::CALL_TRADE_TIME:
    case model_type::PUT_TRADE_TIME:
        return tr( "Trade Time" );
    case model_type::CALL_MARK:
    case model_type::PUT_MARK:
        return tr( "Mark" );
    case model_type::CALL_MARK_CHANGE:
    case model_type::PUT_MARK_CHANGE:
        return tr( "Mark Chg." );
    case model_type::CALL_MARK_PERCENT_CHANGE:
    case model_type::PUT_MARK_PERCENT_CHANGE:
        return tr( "Mark % Chg." );
    case model_type::CALL_EXCHANGE_NAME:
    case model_type::PUT_EXCHANGE_NAME:
        return tr( "Exchange" );
    case model_type::CALL_VOLATILITY:
    case model_type::PUT_VOLATILITY:
        return tr( "Volatility" );
    case model_type::CALL_DELTA:
    case model_type::PUT_DELTA:
        return tr( "Delta" );
    case model_type::CALL_GAMMA:
    case model_type::PUT_GAMMA:
        return tr( "Gamma" );
    case model_type::CALL_THETA:
    case model_type::PUT_THETA:
        return tr( "Theta" );
    case model_type::CALL_VEGA:
    case model_type::PUT_VEGA:
        return tr( "Vega" );
    case model_type::CALL_RHO:
    case model_type::PUT_RHO:
        return tr( "Rho" );
    case model_type::CALL_TIME_VALUE:
    case model_type::PUT_TIME_VALUE:
        return tr( "Time Value" );
    case model_type::CALL_OPEN_INTEREST:
    case model_type::PUT_OPEN_INTEREST:
        return tr( "Open Int" );
    case model_type::CALL_IS_IN_THE_MONEY:
    case model_type::PUT_IS_IN_THE_MONEY:
        return tr( "In The Money" );
    case model_type::CALL_THEO_OPTION_VALUE:
    case model_type::PUT_THEO_OPTION_VALUE:
        return tr( "Theo. Value" );
    case model_type::CALL_THEO_VOLATILITY:
    case model_type::PUT_THEO_VOLATILITY:
        return tr( "Theo. Vol." );
    case model_type::CALL_IS_MINI:
    case model_type::PUT_IS_MINI:
        return tr( "Is Mini" );
    case model_type::CALL_IS_NON_STANDARD:
    case model_type::PUT_IS_NON_STANDARD:
        return tr( "Is Non-Std" );
    case model_type::CALL_IS_INDEX:
    case model_type::PUT_IS_INDEX:
        return tr( "Is Index" );
    case model_type::CALL_IS_WEEKLY:
    case model_type::PUT_IS_WEEKLY:
        return tr( "Is Weekly" );
    case model_type::CALL_IS_QUARTERLY:
    case model_type::PUT_IS_QUARTERLY:
        return tr( "Is Quarterly" );
    case model_type::CALL_EXPIRY_DATE:
    case model_type::PUT_EXPIRY_DATE:
        return tr( "Exp. Date" );
    case model_type::CALL_EXPIRY_TYPE:
    case model_type::PUT_EXPIRY_TYPE:
        return tr( "Exp. Type" );
    case model_type::CALL_DAYS_TO_EXPIRY:
    case model_type::PUT_DAYS_TO_EXPIRY:
        return tr( "Days to Exp." );
    case model_type::CALL_LAST_TRADING_DAY:
    case model_type::PUT_LAST_TRADING_DAY:
        return tr( "Last Tr. Day" );
    case model_type::CALL_MULTIPLIER:
    case model_type::PUT_MULTIPLIER:
        return tr( "Multiplier" );
    case model_type::CALL_SETTLEMENT_TYPE:
    case model_type::PUT_SETTLEMENT_TYPE:
        return tr( "Sett. Type" );
    case model_type::CALL_DELIVERABLE_NOTE:
    case model_type::PUT_DELIVERABLE_NOTE:
        return tr( "Deliv. Note" );
    default:
        break;
    }

    return QString();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::saveHeaderState( const QHeaderView *view, const QString& name )
{
    AppDatabase::instance()->setWidgetState( AppDatabase::HeaderView, STATE_GROUP_NAME, name, view->saveState() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::restoreHeaderState( QHeaderView *view, const QString& name )
{
    const QByteArray a( AppDatabase::instance()->widgetState( AppDatabase::HeaderView, STATE_GROUP_NAME, name ) );

    if ( a.isNull() )
        return;

    view->restoreState( a );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionChainView::resetHeaderState( QHeaderView *view )
{
    // iterate each column
    for ( int i( 0 ); i < model_type::_NUM_COLUMNS; ++i )
    {
        // show column
        setColumnHidden( i, false );

        // resize to default width
        setColumnWidth( i, DEFAULT_WIDTH );

        // move column to default location
        const int vi( view->visualIndex( i ) );

        if ( i != vi )
            view->moveSection( vi, i );
    }

    // hide columns
    setColumnHidden( model_type::STAMP, true );
    setColumnHidden( model_type::UNDERLYING, true );
    setColumnHidden( model_type::EXPIRY_DATE, true );
}

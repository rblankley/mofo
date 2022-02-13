/**
 * @file gridtableheaderview.cpp
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

#include "gridtableheaderview.h"
#include "gridtableheadermodel.h"

#include <QMouseEvent>
#include <QPainter>

///////////////////////////////////////////////////////////////////////////////////////////////////
GridTableHeaderView::GridTableHeaderView( Qt::Orientation orientation, int rows, int columns, QWidget *parent ) :
    _Mybase( orientation, parent )
{
    // create model
    setModel( new model_type( rows, columns, this ) );

    // setup default width and height
    if ( Qt::Horizontal == orientation )
    {
        setDefaultSectionSize( DEFAULT_WIDTH );

        // init section size
        int n( rows );

        sectionSize_.reserve( n );

        while ( n-- )
            sectionSize_.append( DEFAULT_HEIGHT );
    }
    else if ( Qt::Vertical == orientation )
    {
        setDefaultSectionSize( DEFAULT_HEIGHT );

        // init section size
        int n( columns );

        sectionSize_.reserve( n );

        while ( n-- )
            sectionSize_.append( DEFAULT_WIDTH );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GridTableHeaderView::~GridTableHeaderView()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setCellBackgroundColor( int row, int column, const QColor& color )
{
    const QModelIndex idx( model()->index( row, column ) );
    model()->setData( idx, color, Qt::BackgroundRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setCellForegroundColor(int row, int column, const QColor& color )
{
    const QModelIndex idx( model()->index( row, column ) );
    model()->setData( idx, color, Qt::ForegroundRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setCellLabel( int row, int column, const QString& label )
{
    const QModelIndex idx( model()->index( row, column ) );
    model()->setData( idx, label, Qt::DisplayRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setColumnWidth( int col, int width )
{
    if ( Qt::Horizontal == orientation() )
        resizeSection( col, width );
    else if (( 0 <= col ) && ( col < sectionSize_.size() ))
        sectionSize_[col] = width;

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setRowHeight( int row, int height )
{
    if ( Qt::Vertical == orientation() )
        resizeSection( row, height );
    else if (( 0 <= row ) && ( row < sectionSize_.size() ))
        sectionSize_[row] = height;

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setSpan( int row, int column, int rowSpanCount, int columnSpanCount )
{
    QVariant rowSpan;
    QVariant columnSpan;

    if ( 0 < rowSpanCount )
        rowSpan = rowSpanCount;

    if ( 0 < columnSpanCount )
        columnSpan = columnSpanCount;

    // ---- //

    model_type *m( qobject_cast<model_type*>( model() ) );

    const QModelIndex idx( m->index( row, column ) );

    m->setData( idx, rowSpan, model_type::RowSpanRole );
    m->setData( idx, columnSpan, model_type::ColumnSpanRole );

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::setSpan( int row, int column )
{
    if ( Qt::Horizontal == orientation() )
        setSpan( row, column, model()->rowCount(), 1 );
    else
        setSpan( row, column, 1, model()->columnCount() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize GridTableHeaderView::sizeHint() const
{
    int width( 0 );
    int height( 0 );

    // sum width of each visible column
    if ( Qt::Horizontal == orientation() )
    {
        for ( int col( model()->columnCount() ); col--; )
            if ( !isSectionHidden( col ) )
                width += sectionSize( col );

        height = std::accumulate( sectionSize_.begin(), sectionSize_.end(), 0 );
    }

    // sum height of each visible row
    else if ( Qt::Vertical == orientation() )
    {
        for ( int row( model()->rowCount() ); row--; )
            if ( !isSectionHidden( row ) )
                height += sectionSize( row );

        width = std::accumulate( sectionSize_.begin(), sectionSize_.end(), 0 );
    }

    return QSize( width, height );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::mouseReleaseEvent( QMouseEvent *event )
{
    _Mybase::mouseReleaseEvent( event );

    // ---- //

    const QModelIndex idx( indexAt( event->pos() ) );

    if ( !idx.isValid() )
        return;

    const QModelIndex sidx( spanIndex( idx ) );

    if ( Qt::Horizontal == orientation() )
    {
        const QVariant colSpanVar( sidx.data( model_type::ColumnSpanRole ) );
        const int colSpan( colSpanVar.isValid() ? colSpanVar.toInt() : 1 );

        emit sectionPressed( event->pos(), event->button(), sidx.column(), sidx.column() + (colSpan - 1) );
    }
    else if ( Qt::Vertical == orientation() )
    {
        const QVariant rowSpanVar( sidx.data( model_type::RowSpanRole ) );
        const int rowSpan( rowSpanVar.isValid() ? rowSpanVar.toInt() : 1 );

        emit sectionPressed( event->pos(), event->button(), sidx.row(), sidx.row() + (rowSpan - 1) );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QModelIndex GridTableHeaderView::indexAt( const QPoint& pos ) const
{
    const int logicalIdx( logicalIndexAt( pos ) );

    int delta( 0 );

    // iterate row sizes (for Qt::Horizontal) or column sizes (for Qt::Vertical)
    for ( int i( 0 ); i < sectionSize_.size(); ++i )
    {
        // accumulate size
        delta += sectionSize_[i];

        // check point within accumulated size
        if ( Qt::Horizontal == orientation() )
        {
            if ( pos.y() <= delta )
                return spanIndex( model()->index( i, logicalIdx ) );
        }
        else if ( Qt::Vertical == orientation() )
        {
            if ( pos.x() <= delta )
                return spanIndex( model()->index( logicalIdx, i ) );
        }
    }

    return QModelIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize GridTableHeaderView::sectionSizeFromContents( int logicalIndex ) const
{
    QSize result( 0, 0 );

    // for hidden section there is nothing to do
    if ( !isSectionHidden( logicalIndex ) )
    {
        const model_type *m( qobject_cast<const model_type*>( model() ) );

        // generate list of index to evaluate
        QList<QModelIndex> idxs;

        if ( Qt::Horizontal == orientation() )
        {
            for ( int row( sectionSize_.size() ); row--; )
            {
                const QModelIndex idx( spanIndex( m->index( row, logicalIndex ) ) );

                if ( !idxs.contains( idx ) )
                    idxs.append( idx );
            }
        }
        else if ( Qt::Vertical == orientation() )
        {
            for ( int col( sectionSize_.size() ); col--; )
            {
                const QModelIndex idx( spanIndex( m->index( logicalIndex, col ) ) );

                if ( !idxs.contains( idx ) )
                    idxs.append( idx );
            }
        }

        // evaluate each index
        foreach ( const QModelIndex& idx, idxs )
        {
            QVariant var;

            // determine font
            QFont fnt;

            var = m->headerData( logicalIndex, orientation(), Qt::FontRole );

            if (( var.isValid() ) && ( var.canConvert<QFont>() ))
                fnt = qvariant_cast<QFont>( var );
            else
                fnt = font();

            fnt.setBold( true );

            // use contents
            QStyleOptionHeader opt;
            initStyleOption( &opt );
            opt.section = logicalIndex;
            opt.fontMetrics = QFontMetrics( fnt );
            opt.text = idx.data().toString();

            var = m->headerData( logicalIndex, orientation(), Qt::DecorationRole );
            opt.icon = qvariant_cast<QIcon>( var );

            if ( opt.icon.isNull() )
                opt.icon = qvariant_cast<QPixmap>( var );

            if ( isSortIndicatorShown() )
                opt.sortIndicator = QStyleOptionHeader::SortDown;

            const QSize sz( style()->sizeFromContents( QStyle::CT_HeaderSection, &opt, QSize(), this ) );

            result.setHeight( qMax( result.height(), sz.height() ) );
            result.setWidth( qMax( result.width(), sz.width() ) );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QModelIndex GridTableHeaderView::spanIndex( const QModelIndex& index ) const
{
    // for hidden section there is nothing to do
    if (( Qt::Horizontal == orientation() ) && ( isSectionHidden( index.column() ) ))
        return index;
    else if (( Qt::Vertical == orientation() ) && ( isSectionHidden( index.row() ) ))
        return index;

    // generate mapping of viewport positions
    QMap<int, int> viewportPos;

    for ( int idx( count() ); idx--; )
        viewportPos[visualIndex( idx )] = idx;

    // retrieve index
    if ( Qt::Horizontal == orientation() )
        return spanIndexHorizontal( index, viewportPos );
    else if ( Qt::Vertical == orientation() )
        return spanIndexVertical( index, viewportPos );

    return QModelIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QRect GridTableHeaderView::calcRect( const QModelIndex& index ) const
{
    const QVariant colSpanVar( index.data( model_type::ColumnSpanRole ) );
    const int colSpan( colSpanVar.isValid() ? colSpanVar.toInt() : 1 );

    const QVariant rowSpanVar( index.data( model_type::RowSpanRole ) );
    const int rowSpan( rowSpanVar.isValid() ? rowSpanVar.toInt() : 1 );

    // compute rect for section
    QRect rect;

    if ( Qt::Horizontal == orientation() )
    {
        rect.setLeft( sectionViewportPosition( index.column() ) );

        // compute width of rect
        int width( 0 );

        for ( int col( index.column() ); col < (index.column()+colSpan); ++col )
            if ( !isSectionHidden( col ) )
                width += sectionSize( col );

        rect.setWidth( width );

        // compute top and height of rect
        int top( 0 );
        int height( 0 );

        for ( int row( 0 ); row < sectionSize_.size(); ++row )
        {
            if ( row < index.row() )
                top += sectionSize_[row];
            else if (( index.row() <= row ) && ( row < (index.row()+rowSpan) ))
                height += sectionSize_[row];
        }

        rect.setTop( top );
        rect.setHeight( height );
    }
    else if ( Qt::Vertical == orientation() )
    {
        rect.setTop( sectionViewportPosition( index.row() ) );

        // compute height of rect
        int height( 0 );

        for ( int row( index.row() ); row < (index.row()+rowSpan); ++row )
            if ( !isSectionHidden( row ) )
                height += sectionSize( row );

        rect.setHeight( height );

        // compute left and width of rect
        int left( 0 );
        int width( 0 );

        for ( int col( 0 ); col < sectionSize_.size(); ++col )
        {
            if ( col < index.column() )
                left += sectionSize_[col];
            else if (( index.column() <= col ) && ( col < (index.column()+colSpan) ))
                width += sectionSize_[col];
        }

        rect.setLeft( left );
        rect.setWidth( width );
    }

    return rect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void GridTableHeaderView::paintSection( QPainter *painter, const QRect& rect, int logicalIndex ) const
{
    Q_UNUSED( rect )

    // for hidden section there is nothing to do
    if ( isSectionHidden( logicalIndex ) )
        return;

    const model_type *m( qobject_cast<const model_type*>( model() ) );

    // generate list of index to paint
    QList<QModelIndex> idxs;

    if ( Qt::Horizontal == orientation() )
    {
        for ( int row( sectionSize_.size() ); row--; )
        {
            const QModelIndex idx( spanIndex( m->index( row, logicalIndex ) ) );

            if ( !idxs.contains( idx ) )
                idxs.append( idx );
        }
    }
    else if ( Qt::Vertical == orientation() )
    {
        for ( int col( sectionSize_.size() ); col--; )
        {
            const QModelIndex idx( spanIndex( m->index( logicalIndex, col ) ) );

            if ( !idxs.contains( idx ) )
                idxs.append( idx );
        }
    }

    // paint each index
    foreach ( const QModelIndex& idx, idxs )
    {
        // draw section with style
        QStyleOptionHeader opt;
        initStyleOption( &opt );
        opt.textAlignment = Qt::AlignCenter;
        opt.iconAlignment = Qt::AlignVCenter;
        opt.section = logicalIndex;
        opt.text = idx.data( Qt::DisplayRole ).toString();
        opt.rect = calcRect( idx );

        const QVariant bg( idx.data( Qt::BackgroundRole ) );
        const QVariant fg( idx.data( Qt::ForegroundRole ) );

        if ( bg.canConvert<QBrush>() )
        {
            opt.palette.setBrush( QPalette::Button, bg.value<QBrush>() );
            opt.palette.setBrush( QPalette::Window, bg.value<QBrush>() );
        }

        if ( fg.canConvert<QBrush>() )
            opt.palette.setBrush( QPalette::ButtonText, fg.value<QBrush>() );

        style()->drawControl( QStyle::CE_Header, &opt, painter, this );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QModelIndex GridTableHeaderView::spanIndexHorizontal( const QModelIndex& index, const QMap<int, int>& viewportPos ) const
{
    const model_type *m( qobject_cast<const model_type *>( model() ) );

    // iterate over rows and columns to find an index that spans into passed in index
    for ( int row( 0 ); row <= index.row(); ++row )
    {
        bool found( false );

        // distance to our row
        const int rowDist( (index.row() - row) + 1 );

        for ( int col( 0 ); ( col < count() ) && ( !found ); ++col )
            if ( viewportPos.contains( col ) )
            {
                const int logicalCol( viewportPos[col] );

                // reached our column, keep looking starting at next row
                if ( logicalCol == index.column() )
                    found = true;

                const QModelIndex idx( m->index( row, logicalCol ) );

                const QVariant rowSpanVar( idx.data( model_type::RowSpanRole ) );
                const int rowSpan( rowSpanVar.isValid() ? rowSpanVar.toInt() : 1 );

                // row does not span into our row
                if ( rowSpan < rowDist )
                    continue;

                // distance to our column
                int colDist( 1 );

                for ( int i( col ); i < count(); ++i )
                    if ( viewportPos.contains( i ) )
                    {
                        if ( index.column() == viewportPos[i] )
                            break;

                        ++colDist;
                    }

                const QVariant colSpanVar( idx.data( model_type::ColumnSpanRole ) );
                const int colSpan( colSpanVar.isValid() ? colSpanVar.toInt() : 1 );

                // column does not span into our column
                if ( colSpan < colDist )
                    continue;

                // found!!
                return idx;
            }
    }

    return index;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QModelIndex GridTableHeaderView::spanIndexVertical( const QModelIndex& index, const QMap<int, int>& viewportPos ) const
{
    const model_type *m( qobject_cast<const model_type *>( model() ) );

    // iterate over rows and columns to find an index that spans into passed in index
    for ( int col( 0 ); col <= index.column(); ++col )
    {
        bool found( false );

        // distance to our column
        const int colDist( (index.column() - col) + 1 );

        for ( int row( 0 ); ( row < count() ) && ( !found ); ++row )
            if ( viewportPos.contains( row ) )
            {
                const int logicalRow( viewportPos[row] );

                // reached our row, keep looking starting at next column
                if ( logicalRow == index.row() )
                    found = true;

                const QModelIndex idx( m->index( logicalRow, col ) );

                const QVariant colSpanVar( idx.data( model_type::ColumnSpanRole ) );
                const int colSpan( colSpanVar.isValid() ? colSpanVar.toInt() : 1 );

                // column does not span into our column
                if ( colSpan < colDist )
                    continue;

                // distance to our row
                int rowDist( 1 );

                for ( int i( row ); i < count(); ++i )
                    if ( viewportPos.contains( i ) )
                    {
                        if ( index.row() == viewportPos[i] )
                            break;

                        ++rowDist;
                    }

                const QVariant rowSpanVar( idx.data( model_type::RowSpanRole ) );
                const int rowSpan( rowSpanVar.isValid() ? rowSpanVar.toInt() : 1 );

                // row does not span into our row
                if ( rowSpan < rowDist )
                    continue;

                // found!!
                return idx;
            }
    }

    return index;
}

/**
 * @file advancedfilterlistwidget.cpp
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

#include "advancedfilterlistwidget.h"
#include "advancedfilterwidget.h"

#include <QListWidgetItem>

///////////////////////////////////////////////////////////////////////////////////////////////////
AdvancedFilterListWidget::AdvancedFilterListWidget( QWidget *parent ) :
    _Mybase( parent )
{
    // init
    initialize();
    createLayout();
    translate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AdvancedFilterListWidget::~AdvancedFilterListWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QStringList AdvancedFilterListWidget::filters() const
{
    QStringList result;

    for ( FilterItemMap::const_iterator i( w_.constBegin() ); i != w_.constEnd(); ++i )
            result.append( i.key()->filter() );

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterListWidget::setFilters( const QStringList& values )
{
    // remove existing filters
    removeAllFilterItems();

    // create new filters
    foreach ( const QString& value, values )
    {
        AdvancedFilterWidget *w( createFilterItem() );
        w->setFilter( value );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterListWidget::addFilterItem()
{
    createFilterItem();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterListWidget::translate()
{
    for ( FilterItemMap::const_iterator i( w_.constBegin() ); i != w_.constEnd(); ++i )
        i.key()->translate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterListWidget::onRemoveFilterItem()
{
    removeFilterItem( qobject_cast<AdvancedFilterWidget*>( sender() ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterListWidget::initialize()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterListWidget::createLayout()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AdvancedFilterWidget *AdvancedFilterListWidget::createFilterItem()
{
    // create widget
    AdvancedFilterWidget *w( new AdvancedFilterWidget( this ) );

    // create item
    QListWidgetItem *item( new QListWidgetItem( this ) );
    item->setFlags( Qt::NoItemFlags );
    item->setSizeHint( w->sizeHint() );

    // add
    addItem( item );
    setItemWidget( item, w );

    connect( w, &AdvancedFilterWidget::remove, this, &_Myt::onRemoveFilterItem );

    // track relationship
    w_[w] = item;

    return w;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterListWidget::removeFilterItem( AdvancedFilterWidget *f )
{
    if ( !w_.contains( f ) )
        return;

    // delete item
    delete w_[f];

    // delete widget
    f->deleteLater();

    // stop tracking relationship
    w_.remove( f );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterListWidget::removeAllFilterItems()
{
    const QList<AdvancedFilterWidget*> filters( w_.keys() );

    foreach ( AdvancedFilterWidget *f, filters )
        removeFilterItem( f );
}

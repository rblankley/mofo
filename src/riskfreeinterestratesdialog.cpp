/**
 * @file riskfreeinterestratesdialog.cpp
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
#include "riskfreeinterestratesdialog.h"
#include "riskfreeinterestrateswidget.h"

#include "db/appdb.h"

#include <QHBoxLayout>

const QString RiskFreeInterestRatesDialog::STATE_GROUP_NAME( "riskFreeInterestRates" );

static const QString GEOMETRY( "geometry" );

///////////////////////////////////////////////////////////////////////////////////////////////////
RiskFreeInterestRatesDialog::RiskFreeInterestRatesDialog( QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // restore states
    restoreState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
RiskFreeInterestRatesDialog::~RiskFreeInterestRatesDialog()
{
    // save states
    saveState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize RiskFreeInterestRatesDialog::sizeHint() const
{
    // default size
    return QSize( 1800, 900 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesDialog::translate()
{
    setWindowTitle( tr( "Interest Rates" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesDialog::initialize()
{
    rates_ = new RiskFreeInterestRatesWidget( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesDialog::createLayout()
{
    QHBoxLayout *form( new QHBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addWidget( rates_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesDialog::saveState( QDialog *w ) const
{
    if ( w )
        AppDatabase::instance()->setWidgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY, w->saveGeometry() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RiskFreeInterestRatesDialog::restoreState( QDialog *w ) const
{
    if ( w )
        w->restoreGeometry( AppDatabase::instance()->widgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY ) );
}

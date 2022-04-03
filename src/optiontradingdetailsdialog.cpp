/**
 * @file optiontradingdetailsdialog.cpp
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
#include "optionchainprobwidget.h"
#include "optiontradingdetailsdialog.h"

#include "db/appdb.h"
#include "db/optiontradingitemmodel.h"
#include "db/symboldbs.h"

#include <QHBoxLayout>
#include <QTabWidget>

const QString OptionTradingDetailsDialog::STATE_GROUP_NAME( "optionTradingDetails" );

static const QString GEOMETRY( "geometry" );

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionTradingDetailsDialog::OptionTradingDetailsDialog( int index, model_type *model, QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f ),
    model_( model ),
    index_( index )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // grab model data
    underlying_ = modelData( model_type::UNDERLYING ).toString();
    underlyingPrice_ = modelData( model_type::UNDERLYING_PRICE ).toDouble();

    symbols_ = modelData( model_type::SYMBOL ).toString().split( '-' );

    stratDesc_ = modelData( model_type::STRATEGY_DESC ).toString();
    strat_ = modelData( model_type::STRATEGY ).toInt();

    // if this model changes (i.e. background scan) then reject the dialog
    connect( model_, &model_type::modelAboutToBeReset, this, &_Myt::reject );

    // init
    initialize();
    createLayout();
    translate();

    // restore states
    restoreState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionTradingDetailsDialog::~OptionTradingDetailsDialog()
{
    // save states
    saveState( this );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize OptionTradingDetailsDialog::sizeHint() const
{
    // default size
    return QSize( 1800, 900 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingDetailsDialog::translate()
{
    QString trade( tr( "Trade" ) );

    // use symbol for SINGLE trade strategy
    if ( 1 == symbols_.size() )
        trade = symbols_.first();

    QString title;
    title += trade + " " + tr( "Details" );
    title += " - ";
    title += stratDesc_;

    setWindowTitle( title );

    tabs_->setTabText( 0, tr( "Details" ) );
    tabs_->setTabText( 1, tr( "Volatility" ) );
    tabs_->setTabText( 2, tr( "Probability" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingDetailsDialog::initialize()
{
    const QDateTime stamp( QDateTime::fromString( modelData( model_type::STAMP ).toString(), Qt::ISODateWithMs ) );
    const QDate expiryDate( QDate::fromString( modelData( model_type::EXPIRY_DATE ).toString(), Qt::ISODate ) );

    tabs_ = new QTabWidget( this );

    // details
    // TODO

    tabs_->addTab( new QWidget(), QString() );

    // impl vol
    implVol_ = new OptionChainImpliedVolatilityWidget( underlying(), underlyingPrice_, expiryDate, stamp );

    tabs_->addTab( implVol_, QString() );

    // probability
    prob_ = new OptionChainProbabilityWidget( underlying(), underlyingPrice_, expiryDate, stamp );

    if ( model_type::SINGLE == modelData( model_type::STRATEGY ).toInt() )
    {
        prob_->addLeg(
            modelData( model_type::DESC ).toString(),
            modelData( model_type::STRIKE_PRICE ).toDouble(),
            modelData( model_type::TYPE ).toString().contains( "CALL", Qt::CaseInsensitive ),
            true );
    }
    else if ( model_type::VERT_BULL_PUT == modelData( model_type::STRATEGY ).toInt() )
    {
        const QStringList legs( modelData( model_type::DESC ).toString().split( "-" ) );
        const QStringList strikes( modelData( model_type::STRIKE_PRICE ).toString().split( "/" ) );

        if (( 2 == legs.size() ) && ( 2 == strikes.size() ))
        {
            prob_->addLeg(
                "-1 " + legs[0],
                strikes[0].toDouble(),
                false,
                true );

            prob_->addLeg(
                "+1 " + legs[1],
                strikes[1].toDouble(),
                false,
                false );
        }
    }
    else if ( model_type::VERT_BEAR_CALL == modelData( model_type::STRATEGY ).toInt() )
    {
        const QStringList legs( modelData( model_type::DESC ).toString().split( "-" ) );
        const QStringList strikes( modelData( model_type::STRIKE_PRICE ).toString().split( "/" ) );

        if (( 2 == legs.size() ) && ( 2 == strikes.size() ))
        {
            prob_->addLeg(
                "-1 " + legs[0],
                strikes[0].toDouble(),
                true,
                true );

            prob_->addLeg(
                "+1 " + legs[1],
                strikes[1].toDouble(),
                true,
                false );
        }
    }

    tabs_->addTab( prob_, QString() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingDetailsDialog::createLayout()
{
    QHBoxLayout *form( new QHBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addWidget( tabs_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant OptionTradingDetailsDialog::modelData( int col ) const
{
    return model_->data( index_, col, Qt::UserRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingDetailsDialog::saveState( QDialog *w ) const
{
    if ( w )
        AppDatabase::instance()->setWidgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY, w->saveGeometry() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionTradingDetailsDialog::restoreState( QDialog *w ) const
{
    if ( w )
        w->restoreGeometry( AppDatabase::instance()->widgetState( AppDatabase::Dialog, STATE_GROUP_NAME, GEOMETRY ) );
}

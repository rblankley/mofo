/**
 * @file advancedfilterwidget.cpp
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

#include "advancedfilterwidget.h"

#include "db/fundamentalstablemodel.h"
#include "db/optionchaintablemodel.h"
#include "db/optiontradingitemmodel.h"
#include "db/quotetablemodel.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QToolButton>

static const QString QUOTE_TABLE( "Q" );
static const QString FUNDAMENTALS_TABLE( "F" );
static const QString OPTION_CHAIN_TABLE( "OC" );
static const QString OPTION_TRADING_TABLE( "OT" );

static const QString STRING_VALUE( "S" );
static const QString INT_VALUE( "I" );
static const QString DOUBLE_VALUE( "D" );

static const QString TABLE_TYPE( "T" );
static const QString VALUE_TYPE( "V" );

///////////////////////////////////////////////////////////////////////////////////////////////////
AdvancedFilterWidget::AdvancedFilterWidget( QWidget *parent ) :
    _Mybase( parent )
{
    // init
    initialize();
    createLayout();
    translate();

    // hide/show value widgets
    onCurrentIndexChanged( -1 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
AdvancedFilterWidget::~AdvancedFilterWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QString AdvancedFilterWidget::filter() const
{
    const QStringList data( table_->currentData().toString().split( ":" ) );
    const QStringList op( operand_->currentData().toString().split( ":" ) );

    QString result;
    result.append( table_->currentData().toString() );
    result.append( '|' );
    result.append( operand_->currentData().toString() );
    result.append( '|' );

    if ( TABLE_TYPE == op[1] )
        result.append( tableVal_->currentData().toString() );
    else if ( STRING_VALUE == data[2] )
        result.append( stringVal_->text() );
    else if ( INT_VALUE == data[2] )
        result.append( integerVal_->text() );
    else if ( DOUBLE_VALUE == data[2] )
        result.append( doubleVal_->text() );

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterWidget::setFilter( const QString& value )
{
    const QStringList values( value.split( "|" ) );

    if ( 3 != values.size() )
        return;

    const QStringList data( values[0].split( ":" ) );
    const QStringList op( values[1].split( ":" ) );

    table_->setCurrentIndex( table_->findData( values[0] ) );
    operand_->setCurrentIndex( operand_->findData( values[1] ) );

    // hide/show value widgets
    onCurrentIndexChanged( -1 );

    if ( TABLE_TYPE == op[1] )
        tableVal_->setCurrentIndex( tableVal_->findData( values[2] ) );
    else if ( STRING_VALUE == data[2] )
        stringVal_->setText( values[2] );
    else if ( INT_VALUE == data[2] )
        integerVal_->setValue( values[2].toInt() );
    else if ( DOUBLE_VALUE == data[2] )
        doubleVal_->setValue( values[2].toDouble() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterWidget::translate()
{
    translateTableColumns( table_ );

    operand_->setItemText( 0, "==" );
    operand_->setItemText( 1, "!=" );
    operand_->setItemText( 2, "<" );
    operand_->setItemText( 3, "<=" );
    operand_->setItemText( 4, ">" );
    operand_->setItemText( 5, ">=" );
    operand_->setItemText( 6, "== [T]" );
    operand_->setItemText( 7, "!= [T]" );
    operand_->setItemText( 8, "< [T]" );
    operand_->setItemText( 9, "<= [T]" );
    operand_->setItemText( 10, "> [T]" );
    operand_->setItemText( 11, ">= [T]" );

    translateTableColumns( tableVal_ );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterWidget::resizeEvent( QResizeEvent *e )
{
    const int w( (42 * width()) / 100 );

    tableVal_->setFixedWidth( w );
    stringVal_->setFixedWidth( w );
    integerVal_->setFixedWidth( w );
    doubleVal_->setFixedWidth( w );

    _Mybase::resizeEvent( e );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterWidget::onCurrentIndexChanged( int index )
{
    Q_UNUSED( index );

    const QStringList data( table_->currentData().toString().split( ":" ) );
    const QStringList op( operand_->currentData().toString().split( ":" ) );

    if ( VALUE_TYPE == op[1] )
    {
        tableVal_->setVisible( false );
        stringVal_->setVisible( STRING_VALUE == data[2] );
        integerVal_->setVisible( INT_VALUE == data[2] );
        doubleVal_->setVisible( DOUBLE_VALUE == data[2] );
    }
    else if ( TABLE_TYPE == op[1] )
    {
        tableVal_->setVisible( true );
        stringVal_->setVisible( false );
        integerVal_->setValue( false );
        doubleVal_->setVisible( false );

        // clear and re-populate table values that can be matched
        tableVal_->clear();

        populateTableColumns( tableVal_, data[2] );
        translateTableColumns( tableVal_ );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterWidget::initialize()
{
    table_ = new QComboBox( this );
    populateTableColumns( table_ );

    connect( table_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );

    operand_ = new QComboBox( this );
    operand_->addItem( QString(), "EQ:V" );
    operand_->addItem( QString(), "NEQ:V" );
    operand_->addItem( QString(), "LT:V" );
    operand_->addItem( QString(), "LTE:V" );
    operand_->addItem( QString(), "GT:V" );
    operand_->addItem( QString(), "GTE:V" );
    operand_->addItem( QString(), "EQ:T" );
    operand_->addItem( QString(), "NEQ:T" );
    operand_->addItem( QString(), "LT:T" );
    operand_->addItem( QString(), "LTE:T" );
    operand_->addItem( QString(), "GT:T" );
    operand_->addItem( QString(), "GTE:T" );

    connect( operand_, static_cast<void(QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, &_Myt::onCurrentIndexChanged );

    tableVal_ = new QComboBox( this );
    tableVal_->setVisible( false );

    stringVal_ = new QLineEdit( this );
    stringVal_->setVisible( false );

    integerVal_ = new QSpinBox( this );
    integerVal_->setMinimum( -999999999 );
    integerVal_->setMaximum(  999999999 );
    integerVal_->setVisible( false );

    doubleVal_ = new QDoubleSpinBox( this );
    doubleVal_->setDecimals( 4 );
    doubleVal_->setMinimum( -99999999.9999 );
    doubleVal_->setMaximum(  99999999.9999 );
    doubleVal_->setVisible( false );

    remove_ = new QToolButton( this );
    remove_->setIcon( QIcon( ":/res/cancel.png" ) );

    connect( remove_, &QToolButton::pressed, this, &_Myt::remove );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AdvancedFilterWidget::createLayout()
{
    QHBoxLayout *form( new QHBoxLayout( this ) );
    form->setContentsMargins( QMargins() );
    form->addWidget( table_, 1 );
    form->addWidget( operand_ );
    form->addWidget( tableVal_, 1 );
    form->addWidget( stringVal_, 1 );
    form->addWidget( integerVal_, 1 );
    form->addWidget( doubleVal_, 1 );
    form->addWidget( remove_, 1 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
void AdvancedFilterWidget::populateTableColumns( const QString& tableName, const T& table, const QString& type, QComboBox *w )
{
    static const QString colDataFormat( "%0:%1:%2" );

    for ( int col( 0 ); col < T::_NUM_COLUMNS; ++col )
    {
        QString colType( INT_VALUE );

        if ( table.columnIsText( col ) )
            colType = STRING_VALUE;
        else if ( 0 < table.columnNumDecimalPlaces( col ) )
            colType = DOUBLE_VALUE;

        if (( type.isEmpty() ) || ( type == colType ))
            w->addItem( QString(), colDataFormat.arg( tableName ).arg( col ).arg( colType ) );
    }
}

void AdvancedFilterWidget::populateTableColumns( QComboBox *w, const QString& type )
{
    populateTableColumns( QUOTE_TABLE, QuoteTableModel( QString() ), type, w );
    populateTableColumns( FUNDAMENTALS_TABLE, FundamentalsTableModel( QString() ), type, w );
    populateTableColumns( OPTION_CHAIN_TABLE, OptionChainTableModel( QString(), QDate() ), type, w );
    populateTableColumns( OPTION_TRADING_TABLE, OptionTradingItemModel(), type, w );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
QString AdvancedFilterWidget::columnDescription( const T& table, int col )
{
    return table.columnDescription( col );
}

void AdvancedFilterWidget::translateTableColumns( QComboBox *w )
{
    static const QString colDescriptionFormat( "%0 - %1" );

    for ( int i( 0 ); i < w->count(); ++i )
    {
        const QStringList data( w->itemData( i ).toString().split( ":" ) );

        QString colDescription;

        if ( QUOTE_TABLE == data[0] )
            colDescription = colDescriptionFormat.arg(
                tr( "Quote" ),
                columnDescription( QuoteTableModel( QString() ), data[1].toInt() ) );
        else if ( FUNDAMENTALS_TABLE == data[0] )
            colDescription = colDescriptionFormat.arg(
                tr( "Fundamentals" ),
                columnDescription( FundamentalsTableModel( QString() ), data[1].toInt() ) );
        else if ( OPTION_CHAIN_TABLE == data[0] )
            colDescription = colDescriptionFormat.arg(
                tr( "Option Chains" ),
                columnDescription( OptionChainTableModel( QString(), QDate() ), data[1].toInt() ) );
        else if ( OPTION_TRADING_TABLE == data[0] )
            colDescription = colDescriptionFormat.arg(
                tr( "Trades" ),
                columnDescription( OptionTradingItemModel(), data[1].toInt() ) );

        w->setItemText( i, colDescription );
    }
}

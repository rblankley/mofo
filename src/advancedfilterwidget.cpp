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
static const QString CHARTING( "C" );

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
    const int w( (49 * (width() - operand_->sizeHint().width() - remove_->sizeHint().width())) / 100 );

    table_->setFixedWidth( w );

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
        integerVal_->setVisible( false );
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

    if (( type.isEmpty() ) || ( DOUBLE_VALUE == type ))
    {
        // simple moving average
        w->addItem( QString(), CHARTING + ":SMA5:D" );
        w->addItem( QString(), CHARTING + ":SMA5SLOPE:D" );
        w->addItem( QString(), CHARTING + ":SMA5MIN:D" );
        w->addItem( QString(), CHARTING + ":SMA5MAX:D" );

        w->addItem( QString(), CHARTING + ":SMA10:D" );
        w->addItem( QString(), CHARTING + ":SMA10SLOPE:D" );
        w->addItem( QString(), CHARTING + ":SMA10MIN:D" );
        w->addItem( QString(), CHARTING + ":SMA10MAX:D" );

        w->addItem( QString(), CHARTING + ":SMA15:D" );
        w->addItem( QString(), CHARTING + ":SMA15SLOPE:D" );
        w->addItem( QString(), CHARTING + ":SMA15MIN:D" );
        w->addItem( QString(), CHARTING + ":SMA15MAX:D" );

        w->addItem( QString(), CHARTING + ":SMA20:D" );
        w->addItem( QString(), CHARTING + ":SMA20SLOPE:D" );
        w->addItem( QString(), CHARTING + ":SMA20MIN:D" );
        w->addItem( QString(), CHARTING + ":SMA20MAX:D" );

        w->addItem( QString(), CHARTING + ":SMA30:D" );
        w->addItem( QString(), CHARTING + ":SMA30SLOPE:D" );
        w->addItem( QString(), CHARTING + ":SMA30MIN:D" );
        w->addItem( QString(), CHARTING + ":SMA30MAX:D" );

        w->addItem( QString(), CHARTING + ":SMA50:D" );
        w->addItem( QString(), CHARTING + ":SMA50SLOPE:D" );
        w->addItem( QString(), CHARTING + ":SMA50MIN:D" );
        w->addItem( QString(), CHARTING + ":SMA50MAX:D" );

        w->addItem( QString(), CHARTING + ":SMA100:D" );
        w->addItem( QString(), CHARTING + ":SMA100SLOPE:D" );
        w->addItem( QString(), CHARTING + ":SMA100MIN:D" );
        w->addItem( QString(), CHARTING + ":SMA100MAX:D" );

        w->addItem( QString(), CHARTING + ":SMA200:D" );
        w->addItem( QString(), CHARTING + ":SMA200SLOPE:D" );
        w->addItem( QString(), CHARTING + ":SMA200MIN:D" );
        w->addItem( QString(), CHARTING + ":SMA200MAX:D" );

        // exponential moving average
        w->addItem( QString(), CHARTING + ":EMA5:D" );
        w->addItem( QString(), CHARTING + ":EMA5SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA5MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA5MAX:D" );

        w->addItem( QString(), CHARTING + ":EMA10:D" );
        w->addItem( QString(), CHARTING + ":EMA10SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA10MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA10MAX:D" );

        w->addItem( QString(), CHARTING + ":EMA12:D" );
        w->addItem( QString(), CHARTING + ":EMA12SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA12MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA12MAX:D" );

        w->addItem( QString(), CHARTING + ":EMA15:D" );
        w->addItem( QString(), CHARTING + ":EMA15SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA15MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA15MAX:D" );

        w->addItem( QString(), CHARTING + ":EMA20:D" );
        w->addItem( QString(), CHARTING + ":EMA20SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA20MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA20MAX:D" );

        w->addItem( QString(), CHARTING + ":EMA26:D" );
        w->addItem( QString(), CHARTING + ":EMA26SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA26MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA26MAX:D" );

        w->addItem( QString(), CHARTING + ":EMA30:D" );
        w->addItem( QString(), CHARTING + ":EMA30SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA30MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA30MAX:D" );

        w->addItem( QString(), CHARTING + ":EMA50:D" );
        w->addItem( QString(), CHARTING + ":EMA50SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA50MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA50MAX:D" );

        w->addItem( QString(), CHARTING + ":EMA100:D" );
        w->addItem( QString(), CHARTING + ":EMA100SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA100MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA100MAX:D" );

        w->addItem( QString(), CHARTING + ":EMA200:D" );
        w->addItem( QString(), CHARTING + ":EMA200SLOPE:D" );
        w->addItem( QString(), CHARTING + ":EMA200MIN:D" );
        w->addItem( QString(), CHARTING + ":EMA200MAX:D" );

        // macd
        w->addItem( QString(), CHARTING + ":MACD:D" );
        w->addItem( QString(), CHARTING + ":MACDSLOPE:D" );
        w->addItem( QString(), CHARTING + ":MACDSIG:D" );
        w->addItem( QString(), CHARTING + ":MACDSIGSLOPE:D" );
        w->addItem( QString(), CHARTING + ":MACDH:D" );
        w->addItem( QString(), CHARTING + ":MACDHSLOPE:D" );
    }

    if (( type.isEmpty() ) || ( INT_VALUE == type ))
    {
        w->addItem( QString(), CHARTING + ":MACDBUYFLAG:I" );
        w->addItem( QString(), CHARTING + ":MACDSELLFLAG:I" );
    }

    if (( type.isEmpty() ) || ( DOUBLE_VALUE == type ))
    {
        // relative strength index
        w->addItem( QString(), CHARTING + ":RSI2:D" );
        w->addItem( QString(), CHARTING + ":RSI2SLOPE:D" );
        w->addItem( QString(), CHARTING + ":RSI2MIN:D" );
        w->addItem( QString(), CHARTING + ":RSI2MAX:D" );

        w->addItem( QString(), CHARTING + ":RSI3:D" );
        w->addItem( QString(), CHARTING + ":RSI3SLOPE:D" );
        w->addItem( QString(), CHARTING + ":RSI3MIN:D" );
        w->addItem( QString(), CHARTING + ":RSI3MAX:D" );

        w->addItem( QString(), CHARTING + ":RSI4:D" );
        w->addItem( QString(), CHARTING + ":RSI4SLOPE:D" );
        w->addItem( QString(), CHARTING + ":RSI4MIN:D" );
        w->addItem( QString(), CHARTING + ":RSI4MAX:D" );

        w->addItem( QString(), CHARTING + ":RSI5:D" );
        w->addItem( QString(), CHARTING + ":RSI5SLOPE:D" );
        w->addItem( QString(), CHARTING + ":RSI5MIN:D" );
        w->addItem( QString(), CHARTING + ":RSI5MAX:D" );

        w->addItem( QString(), CHARTING + ":RSI6:D" );
        w->addItem( QString(), CHARTING + ":RSI6SLOPE:D" );
        w->addItem( QString(), CHARTING + ":RSI6MIN:D" );
        w->addItem( QString(), CHARTING + ":RSI6MAX:D" );

        w->addItem( QString(), CHARTING + ":RSI10:D" );
        w->addItem( QString(), CHARTING + ":RSI10SLOPE:D" );
        w->addItem( QString(), CHARTING + ":RSI10MIN:D" );
        w->addItem( QString(), CHARTING + ":RSI10MAX:D" );

        w->addItem( QString(), CHARTING + ":RSI14:D" );
        w->addItem( QString(), CHARTING + ":RSI14SLOPE:D" );
        w->addItem( QString(), CHARTING + ":RSI14MIN:D" );
        w->addItem( QString(), CHARTING + ":RSI14MAX:D" );

        w->addItem( QString(), CHARTING + ":RSI20:D" );
        w->addItem( QString(), CHARTING + ":RSI20SLOPE:D" );
        w->addItem( QString(), CHARTING + ":RSI20MIN:D" );
        w->addItem( QString(), CHARTING + ":RSI20MAX:D" );

        w->addItem( QString(), CHARTING + ":RSI50:D" );
        w->addItem( QString(), CHARTING + ":RSI50SLOPE:D" );
        w->addItem( QString(), CHARTING + ":RSI50MIN:D" );
        w->addItem( QString(), CHARTING + ":RSI50MAX:D" );

        // historical volatility
        w->addItem( QString(), CHARTING + ":HV5:D" );
        w->addItem( QString(), CHARTING + ":HV5SLOPE:D" );
        w->addItem( QString(), CHARTING + ":HV5MIN:D" );
        w->addItem( QString(), CHARTING + ":HV5MAX:D" );

        w->addItem( QString(), CHARTING + ":HV10:D" );
        w->addItem( QString(), CHARTING + ":HV10SLOPE:D" );
        w->addItem( QString(), CHARTING + ":HV10MIN:D" );
        w->addItem( QString(), CHARTING + ":HV10MAX:D" );

        w->addItem( QString(), CHARTING + ":HV20:D" );
        w->addItem( QString(), CHARTING + ":HV20SLOPE:D" );
        w->addItem( QString(), CHARTING + ":HV20MIN:D" );
        w->addItem( QString(), CHARTING + ":HV20MAX:D" );

        w->addItem( QString(), CHARTING + ":HV30:D" );
        w->addItem( QString(), CHARTING + ":HV30SLOPE:D" );
        w->addItem( QString(), CHARTING + ":HV30MIN:D" );
        w->addItem( QString(), CHARTING + ":HV30MAX:D" );

        w->addItem( QString(), CHARTING + ":HV60:D" );
        w->addItem( QString(), CHARTING + ":HV60SLOPE:D" );
        w->addItem( QString(), CHARTING + ":HV60MIN:D" );
        w->addItem( QString(), CHARTING + ":HV60MAX:D" );

        w->addItem( QString(), CHARTING + ":HV90:D" );
        w->addItem( QString(), CHARTING + ":HV90SLOPE:D" );
        w->addItem( QString(), CHARTING + ":HV90MIN:D" );
        w->addItem( QString(), CHARTING + ":HV90MAX:D" );

        w->addItem( QString(), CHARTING + ":HV120:D" );
        w->addItem( QString(), CHARTING + ":HV120SLOPE:D" );
        w->addItem( QString(), CHARTING + ":HV120MIN:D" );
        w->addItem( QString(), CHARTING + ":HV120MAX:D" );

        w->addItem( QString(), CHARTING + ":HV240:D" );
        w->addItem( QString(), CHARTING + ":HV240SLOPE:D" );
        w->addItem( QString(), CHARTING + ":HV240MIN:D" );
        w->addItem( QString(), CHARTING + ":HV240MAX:D" );

        w->addItem( QString(), CHARTING + ":HV480:D" );
        w->addItem( QString(), CHARTING + ":HV480SLOPE:D" );
        w->addItem( QString(), CHARTING + ":HV480MIN:D" );
        w->addItem( QString(), CHARTING + ":HV480MAX:D" );

        w->addItem( QString(), CHARTING + ":HVDTE:D" );
        w->addItem( QString(), CHARTING + ":HVDTESLOPE:D" );
        w->addItem( QString(), CHARTING + ":HVDTEMIN:D" );
        w->addItem( QString(), CHARTING + ":HVDTEMAX:D" );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
QString AdvancedFilterWidget::columnDescription( const T& table, int col )
{
    return table.columnDescription( col );
}

QString AdvancedFilterWidget::chartingValueDescription( const QString& cdata )
{
    QString data( cdata );
    QString suffix;

    if ( cdata.endsWith( "SLOPE" ) )
    {
        data = cdata.chopped( 5 );
        suffix = tr( "- Slope" );
    }
    else if ( cdata.endsWith( "MIN" ) )
    {
        data = cdata.chopped( 3 );
        suffix = tr( "- Minimum Value" );
    }
    else if ( cdata.endsWith( "MAX" ) )
    {
        data = cdata.chopped( 3 );
        suffix = tr( "- Maximum Value" );
    }

    // simple moving average
    if ( data.startsWith( "SMA" ) )
        return tr( "Simple Moving Average - %1 Days %2" ).arg( QStringView{ data }.mid( 3 ) ).arg( suffix );

    // exponential moving average
    else if ( data.startsWith( "EMA" ) )
        return tr( "Exponential Moving Average - %1 Days %2" ).arg( QStringView{ data }.mid( 3 ) ).arg( suffix );

    // relative strength index
    else if ( data.startsWith( "RSI" ) )
        return tr( "Relative Strength Index - %1 Days %2" ).arg( QStringView{ data }.mid( 3 ) ).arg( suffix );

    // historical volatility
    else if ( data.startsWith( "HVDTE" ) )
        return tr( "Historical Volatility - Trading Days Until Expiration %1" ).arg( suffix );
    else if ( data.startsWith( "HV" ) )
        return tr( "Historical Volatility - %1 Days %2" ).arg( QStringView{ data }.mid( 2 ) ).arg( suffix );

    // macd
    else if ( data.startsWith( "MACD" ) )
    {
        const QString title( tr( "Moving Average Convergence/Divergence (MACD)" ) );

        if ( "MACD" == data )
            return QString( "%1 %2" ).arg( title, suffix );
        else if ( "MACDSIG" == data )
            return tr( "%1 - Signal Line Value %2" ).arg( title, suffix );
        else if ( "MACDH" == data )
            return tr( "%1 - Histogram Value %2" ).arg( title, suffix );
        else if ( "MACDBUYFLAG" == data )
            return tr( "%1 - Buy Flag" ).arg( title );
        else if ( "MACDSELLFLAG" == data )
            return tr( "%1 - Sell Flag" ).arg( title );
    }

    return QString();
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
        else if ( CHARTING == data[0] )
            colDescription = colDescriptionFormat.arg(
                tr( "Charting" ),
                chartingValueDescription( data[1] ) );

        w->setItemText( i, colDescription );
    }
}

/**
 * @file filtereditordialog.cpp
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
#include "filtereditordialog.h"

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
FilterEditorDialog::FilterEditorDialog( const QString& name, const QByteArray& value, QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f ),
    name_( name )
{
    // remove the question mark button
    setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );

    // init
    initialize();
    createLayout();
    translate();

    // parse filter
    f_.restoreState( value );

    // set filters
    minInvestAmount_->setValue( f_.minInvestAmount() );
    maxInvestAmount_->setValue( f_.maxInvestAmount() );

    maxLossAmount_->setValue( f_.maxLossAmount() );

    minReturnOnInvestment_->setValue( f_.minReturnOnInvestment() );
    maxReturnOnInvestment_->setValue( f_.maxReturnOnInvestment() );

    minReturnOnInvestmentTime_->setValue( f_.minReturnOnInvestmentTime() );
    maxReturnOnInvestmentTime_->setValue( f_.maxReturnOnInvestmentTime() );

    maxSpreadPercent_->setValue( f_.maxSpreadPercent() );

    minVolatility_->setValue( f_.minVolatility() );
    maxVolatility_->setValue( f_.maxVolatility() );

    itmCalls_->setChecked( OptionProfitCalculatorFilter::ITM_CALLS & f_.optionTypeFilter() );
    otmCalls_->setChecked( OptionProfitCalculatorFilter::OTM_CALLS & f_.optionTypeFilter() );
    itmPuts_->setChecked( OptionProfitCalculatorFilter::ITM_PUTS & f_.optionTypeFilter() );
    otmPuts_->setChecked( OptionProfitCalculatorFilter::OTM_PUTS & f_.optionTypeFilter() );

    single_->setChecked( OptionProfitCalculatorFilter::SINGLE & f_.optionTradingStrategyFilter() );
    vertical_->setChecked( OptionProfitCalculatorFilter::VERTICAL & f_.optionTradingStrategyFilter() );
    calendar_->setChecked( OptionProfitCalculatorFilter::CALENDAR & f_.optionTradingStrategyFilter() );
    strangle_->setChecked( OptionProfitCalculatorFilter::STRANGLE & f_.optionTradingStrategyFilter() );
    straddle_->setChecked( OptionProfitCalculatorFilter::STRADDLE & f_.optionTradingStrategyFilter() );
    butterfly_->setChecked( OptionProfitCalculatorFilter::BUTTERFLY & f_.optionTradingStrategyFilter() );
    condor_->setChecked( OptionProfitCalculatorFilter::CONDOR & f_.optionTradingStrategyFilter() );
    diagonal_->setChecked( OptionProfitCalculatorFilter::DIAGONAL & f_.optionTradingStrategyFilter() );
    collar_->setChecked( OptionProfitCalculatorFilter::COLLAR & f_.optionTradingStrategyFilter() );

    histLessThanImpl_->setChecked( OptionProfitCalculatorFilter::HV_LTE_VI & f_.volatilityFilter() );
    histGreaterThanImpl_->setChecked( OptionProfitCalculatorFilter::HV_GT_VI & f_.volatilityFilter() );

    verticalDepth_->setValue( f_.verticalDepth() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QByteArray FilterEditorDialog::filterValue() const
{
    return f_.saveState();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QSize FilterEditorDialog::sizeHint() const
{
    return QSize( 800, 800 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterEditorDialog::translate()
{
    setWindowTitle( tr( "Filter Editor" ) + " - " + name_ );

    minInvestAmountLabel_->setText( tr( "Minimum Investment Amount" ) );
    maxInvestAmountLabel_->setText( tr( "Maximum Investment Amount" ) );

    maxLossAmountLabel_->setText( tr( "Maximum Loss Allowed" ) );

    minReturnOnInvestmentLabel_->setText( tr( "Minimum ROI (%)" ) );
    maxReturnOnInvestmentLabel_->setText( tr( "Maximum ROI (%)" ) );

    minReturnOnInvestmentTimeLabel_->setText( tr( "Minimum ROI / Time (%)" ) );
    maxReturnOnInvestmentTimeLabel_->setText( tr( "Maximum ROI / Time (%)" ) );

    maxSpreadPercentLabel_->setText( tr( "Maximum Bid/Ask Spread (%)" ) );

    minVolatilityLabel_->setText( tr( "Minimum Implied Volatility" ) );
    maxVolatilityLabel_->setText( tr( "Maximum Implied Volatility" ) );

    optionTypes_->setTitle( tr( "Option Type" ) );

    itmCalls_->setText( tr( "ITM Calls" ) );
    otmCalls_->setText( tr( "OTM Calls" ) );
    itmPuts_->setText( tr( "ITM Puts" ) );
    otmPuts_->setText( tr( "OTM Puts" ) );

    optionTradingStrats_->setTitle( tr( "Option Trading Strategy" ) );

    single_->setText( tr( "Single" ) );
    vertical_->setText( tr( "Vertical" ) );
    calendar_->setText( tr( "Calendar" ) );
    strangle_->setText( tr( "Strangle" ) );
    straddle_->setText( tr( "Straddle" ) );
    butterfly_->setText( tr( "Butterfly" ) );
    condor_->setText( tr( "Iron Condor" ) );
    diagonal_->setText( tr( "Diagonal" ) );
    collar_->setText( tr( "Collar" ) );

    volatility_->setTitle( tr( "Volatility" ) );

    histLessThanImpl_->setText( tr( "Hist. Vol. <= Implied Vol." ) );
    histGreaterThanImpl_->setText( tr( "Hist. Vol. > Implied Vol." ) );

    verticalDepthLabel_->setText( tr( "Vertical Depth" ) );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterEditorDialog::onButtonClicked()
{
    // okay
    if ( okay_ == sender() )
    {
        // set filters
        f_.setMinInvestAmount( minInvestAmount_->value() );
        f_.setMaxInvestAmount( maxInvestAmount_->value() );

        f_.setMaxLossAmount( maxLossAmount_->value() );

        f_.setMinReturnOnInvestment( minReturnOnInvestment_->value() );
        f_.setMaxReturnOnInvestment( maxReturnOnInvestment_->value() );

        f_.setMinReturnOnInvestmentTime( minReturnOnInvestmentTime_->value() );
        f_.setMaxReturnOnInvestmentTime( maxReturnOnInvestmentTime_->value() );

        f_.setMaxSpreadPercent( maxSpreadPercent_->value() );

        f_.setMinVolatility( minVolatility_->value() );
        f_.setMaxVolatility( maxVolatility_->value() );

        int optionTypeFilter( 0 );
        optionTypeFilter |= ( itmCalls_->isChecked() ? OptionProfitCalculatorFilter::ITM_CALLS : 0 );
        optionTypeFilter |= ( otmCalls_->isChecked() ? OptionProfitCalculatorFilter::OTM_CALLS : 0 );
        optionTypeFilter |= ( itmPuts_->isChecked() ? OptionProfitCalculatorFilter::ITM_PUTS : 0 );
        optionTypeFilter |= ( otmPuts_->isChecked() ? OptionProfitCalculatorFilter::OTM_PUTS : 0 );

        f_.setOptionTypeFilter( (OptionProfitCalculatorFilter::OptionTypeFilter) optionTypeFilter );

        int optionTradingStrategyFilter( 0 );
        optionTradingStrategyFilter |= ( single_->isChecked() ? OptionProfitCalculatorFilter::SINGLE : 0 );
        optionTradingStrategyFilter |= ( vertical_->isChecked() ? OptionProfitCalculatorFilter::VERTICAL : 0 );
        optionTradingStrategyFilter |= ( calendar_->isChecked() ? OptionProfitCalculatorFilter::CALENDAR : 0 );
        optionTradingStrategyFilter |= ( strangle_->isChecked() ? OptionProfitCalculatorFilter::STRANGLE : 0 );
        optionTradingStrategyFilter |= ( straddle_->isChecked() ? OptionProfitCalculatorFilter::STRADDLE : 0 );
        optionTradingStrategyFilter |= ( butterfly_->isChecked() ? OptionProfitCalculatorFilter::BUTTERFLY : 0 );
        optionTradingStrategyFilter |= ( condor_->isChecked() ? OptionProfitCalculatorFilter::CONDOR : 0 );
        optionTradingStrategyFilter |= ( diagonal_->isChecked() ? OptionProfitCalculatorFilter::DIAGONAL : 0 );
        optionTradingStrategyFilter |= ( collar_->isChecked() ? OptionProfitCalculatorFilter::COLLAR : 0 );

        f_.setOptionTradingStrategyFilter( (OptionProfitCalculatorFilter::OptionTradingStrategyFilter) optionTradingStrategyFilter );

        int volatilityFilter( 0 );
        volatilityFilter |= ( histLessThanImpl_->isChecked() ? OptionProfitCalculatorFilter::HV_LTE_VI : 0 );
        volatilityFilter |= ( histGreaterThanImpl_->isChecked() ? OptionProfitCalculatorFilter::HV_GT_VI : 0 );

        f_.setVolatilityFilter( (OptionProfitCalculatorFilter::VolatilityFilter) volatilityFilter );

        f_.setVerticalDepth( verticalDepth_->value() );

        accept();
    }

    // cancel
    else if ( cancel_ == sender() )
    {
        reject();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterEditorDialog::initialize()
{
    minInvestAmountLabel_ = new QLabel( this );

    minInvestAmount_ = new QDoubleSpinBox( this );
    minInvestAmount_->setDecimals( 2 );
    minInvestAmount_->setMinimum( 0.0 );

    maxInvestAmountLabel_ = new QLabel( this );

    maxInvestAmount_ = new QDoubleSpinBox( this );
    maxInvestAmount_->setDecimals( 2 );
    maxInvestAmount_->setMinimum( 0.0 );

    maxLossAmountLabel_ = new QLabel( this );

    maxLossAmount_ = new QDoubleSpinBox( this );
    maxLossAmount_->setDecimals( 2 );
    maxLossAmount_->setMinimum( 0.0 );

    minReturnOnInvestmentLabel_ = new QLabel( this );

    minReturnOnInvestment_ = new QDoubleSpinBox( this );
    minReturnOnInvestment_->setDecimals( 2 );
    minReturnOnInvestment_->setMinimum( 0.0 );

    maxReturnOnInvestmentLabel_ = new QLabel( this );

    maxReturnOnInvestment_ = new QDoubleSpinBox( this );
    maxReturnOnInvestment_->setDecimals( 2 );
    maxReturnOnInvestment_->setMinimum( 0.0 );

    minReturnOnInvestmentTimeLabel_ = new QLabel( this );

    minReturnOnInvestmentTime_ = new QDoubleSpinBox( this );
    minReturnOnInvestmentTime_->setDecimals( 2 );
    minReturnOnInvestmentTime_->setMinimum( 0.0 );

    maxReturnOnInvestmentTimeLabel_ = new QLabel( this );

    maxReturnOnInvestmentTime_ = new QDoubleSpinBox( this );
    maxReturnOnInvestmentTime_->setDecimals( 2 );
    maxReturnOnInvestmentTime_->setMinimum( 0.0 );

    maxSpreadPercentLabel_ = new QLabel( this );

    maxSpreadPercent_ = new QDoubleSpinBox( this );
    maxSpreadPercent_->setDecimals( 2 );
    maxSpreadPercent_->setMinimum( 0.0 );

    minVolatilityLabel_ = new QLabel( this );

    minVolatility_ = new QDoubleSpinBox( this );
    minVolatility_->setDecimals( 2 );
    minVolatility_->setMinimum( 0.0 );

    maxVolatilityLabel_ = new QLabel( this );

    maxVolatility_ = new QDoubleSpinBox( this );
    maxVolatility_->setDecimals( 2 );
    maxVolatility_->setMinimum( 0.0 );

    optionTypes_ = new QGroupBox( this );

    itmCalls_ = new QCheckBox( optionTypes_ );

    otmCalls_ = new QCheckBox( optionTypes_ );

    itmPuts_ = new QCheckBox( optionTypes_ );

    otmPuts_ = new QCheckBox( optionTypes_ );

    optionTradingStrats_ = new QGroupBox( this );

    single_ = new QCheckBox( optionTradingStrats_ );

    vertical_ = new QCheckBox( optionTradingStrats_ );

    calendar_ = new QCheckBox( optionTradingStrats_ );

    strangle_ = new QCheckBox( optionTradingStrats_ );

    straddle_ = new QCheckBox( optionTradingStrats_ );

    butterfly_ = new QCheckBox( optionTradingStrats_ );

    condor_ = new QCheckBox( optionTradingStrats_ );

    diagonal_ = new QCheckBox( optionTradingStrats_ );

    collar_ = new QCheckBox( optionTradingStrats_ );

    volatility_ = new QGroupBox( this );

    histLessThanImpl_ = new QCheckBox( volatility_ );

    histGreaterThanImpl_ = new QCheckBox( volatility_ );

    verticalDepthLabel_ = new QLabel( this );

    verticalDepth_ = new QSpinBox( this );
    verticalDepth_->setMinimum( 1 );

    // okay
    okay_ = new QPushButton( this );

    connect( okay_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // cancel
    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::onButtonClicked );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterEditorDialog::createLayout()
{
    QVBoxLayout *optionTypes( new QVBoxLayout( optionTypes_ ) );
    optionTypes->addWidget( itmCalls_ );
    optionTypes->addWidget( otmCalls_ );
    optionTypes->addWidget( itmPuts_ );
    optionTypes->addWidget( otmPuts_ );
    optionTypes->addStretch();

    QVBoxLayout *optionTradingStrats( new QVBoxLayout( optionTradingStrats_ ) );
    optionTradingStrats->addWidget( single_ );
    optionTradingStrats->addWidget( vertical_ );
    optionTradingStrats->addWidget( calendar_ );
    optionTradingStrats->addWidget( strangle_ );
    optionTradingStrats->addWidget( straddle_ );
    optionTradingStrats->addWidget( butterfly_ );
    optionTradingStrats->addWidget( condor_ );
    optionTradingStrats->addWidget( diagonal_ );
    optionTradingStrats->addWidget( collar_ );
    optionTradingStrats->addStretch();

    QVBoxLayout *volatility( new QVBoxLayout( volatility_ ) );
    volatility->addWidget( histLessThanImpl_ );
    volatility->addWidget( histGreaterThanImpl_ );
    volatility->addStretch();

    // ---- //

    QFormLayout *filters( new QFormLayout() );
    filters->addRow( minInvestAmountLabel_, minInvestAmount_ );
    filters->addRow( maxInvestAmountLabel_, maxInvestAmount_ );
    filters->addItem( new QSpacerItem( 16, 16 ) );
    filters->addRow( maxLossAmountLabel_, maxLossAmount_ );
    filters->addItem( new QSpacerItem( 16, 16 ) );
    filters->addRow( minReturnOnInvestmentLabel_, minReturnOnInvestment_ );
    filters->addRow( maxReturnOnInvestmentLabel_, maxReturnOnInvestment_ );
    filters->addItem( new QSpacerItem( 16, 16 ) );
    filters->addRow( minReturnOnInvestmentTimeLabel_, minReturnOnInvestmentTime_ );
    filters->addRow( maxReturnOnInvestmentTimeLabel_, maxReturnOnInvestmentTime_ );
    filters->addItem( new QSpacerItem( 16, 16 ) );
    filters->addRow( maxSpreadPercentLabel_, maxSpreadPercent_ );
    filters->addItem( new QSpacerItem( 16, 16 ) );
    filters->addRow( minVolatilityLabel_, minVolatility_ );
    filters->addRow( maxVolatilityLabel_, maxVolatility_ );
    filters->addItem( new QSpacerItem( 16, 16 ) );
    filters->addRow( verticalDepthLabel_, verticalDepth_ );
    filters->addItem( new QSpacerItem( 16, 16 ) );

    QHBoxLayout *groups( new QHBoxLayout() );
    groups->addWidget( optionTypes_ );
    groups->addWidget( optionTradingStrats_ );
    groups->addWidget( volatility_ );

    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( filters );
    form->addLayout( groups );
    form->addStretch();
    form->addLayout( buttons );
}

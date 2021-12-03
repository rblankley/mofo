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
#include <QTabWidget>
#include <QVBoxLayout>

///////////////////////////////////////////////////////////////////////////////////////////////////
FilterEditorDialog::FilterEditorDialog( const QString& name, const QByteArray& value, QWidget *parent, Qt::WindowFlags f ) :
    _Mybase( parent, f ),
    name_( name ),
    sized_( false )
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
    minUnderlyingPrice_->setValue( f_.minUnderlyingPrice() );
    maxUnderlyingPrice_->setValue( f_.maxUnderlyingPrice() );

    minInvestAmount_->setValue( f_.minInvestAmount() );
    maxInvestAmount_->setValue( f_.maxInvestAmount() );

    maxLossAmount_->setValue( f_.maxLossAmount() );
    minGainAmount_->setValue( f_.minGainAmount() );

    minBidSize_->setValue( f_.minBidSize() );
    minAskSize_->setValue( f_.minAskSize() );

    maxSpreadPercent_->setValue( f_.maxSpreadPercent() );

    minDaysToExpiry_->setValue( f_.minDaysToExpiry() );
    maxDaysToExpiry_->setValue( f_.maxDaysToExpiry() );

    minImplVolatility_->setValue( f_.minVolatility() );
    maxImplVolatility_->setValue( f_.maxVolatility() );

    minDivAmount_->setValue( f_.minDividendAmount() );
    maxDivAmount_->setValue( f_.maxDividendAmount() );

    minDivYield_->setValue( f_.minDividendYield() );
    maxDivYield_->setValue( f_.maxDividendYield() );

    minProbITM_->setValue( f_.minProbITM() );
    maxProbITM_->setValue( f_.maxProbITM() );

    minProbOTM_->setValue( f_.minProbOTM() );
    maxProbOTM_->setValue( f_.maxProbOTM() );

    minProbProfit_->setValue( f_.minProbProfit() );
    maxProbProfit_->setValue( f_.maxProbProfit() );

    minReturnOnRisk_->setValue( f_.minReturnOnRisk() );
    maxReturnOnRisk_->setValue( f_.maxReturnOnRisk() );

    minReturnOnRiskTime_->setValue( f_.minReturnOnRiskTime() );
    maxReturnOnRiskTime_->setValue( f_.maxReturnOnRiskTime() );

    minReturnOnInvestment_->setValue( f_.minReturnOnInvestment() );
    maxReturnOnInvestment_->setValue( f_.maxReturnOnInvestment() );

    minReturnOnInvestmentTime_->setValue( f_.minReturnOnInvestmentTime() );
    maxReturnOnInvestmentTime_->setValue( f_.maxReturnOnInvestmentTime() );

    minExpectedValue_->setValue( f_.minExpectedValue() );
    maxExpectedValue_->setValue( f_.maxExpectedValue() );

    minExpectedValueReturnOnInvestment_->setValue( f_.minExpectedValueReturnOnInvestment() );
    maxExpectedValueReturnOnInvestment_->setValue( f_.maxExpectedValueReturnOnInvestment() );

    minExpectedValueReturnOnInvestmentTime_->setValue( f_.minExpectedValueReturnOnInvestmentTime() );
    maxExpectedValueReturnOnInvestmentTime_->setValue( f_.maxExpectedValueReturnOnInvestmentTime() );

    verticalDepth_->setValue( f_.verticalDepth() );

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

    theoPriceLessThanMarket_->setChecked( OptionProfitCalculatorFilter::THEO_LTE_MARKET & f_.priceFilter() );
    theoPriceGreaterThanMarket_->setChecked( OptionProfitCalculatorFilter::THEO_GT_MARKET & f_.priceFilter() );

    histLessThanImpl_->setChecked( OptionProfitCalculatorFilter::HV_LTE_VI & f_.volatilityFilter() );
    histGreaterThanImpl_->setChecked( OptionProfitCalculatorFilter::HV_GT_VI & f_.volatilityFilter() );
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

    minColumnLabel_->setText( tr( "Minimum" ) );
    maxColumnLabel_->setText( tr( "Maximum" ) );

    underlyingPriceLabel_->setText( tr( "Underlying (Spot) Price ($)" ) );
    minUnderlyingPrice_->setToolTip( tr( "Underlying with mark prices below this amount will be filtered out." ) );
    maxUnderlyingPrice_->setToolTip( tr( "Underlying with mark prices above this amount will be filtered out." ) );

    verticalDepthLabel_->setText( tr( "Vertical Depth" ) );
    verticalDepth_->setToolTip( tr( "Determines how many option steps will be evaluated when calculating verticals. For example, a value of 3 will evaluate options 1, 2, and 3 steps away from a strike." ) );

    tabs_->setTabText( 0, tr( "Option Data" ) );

    minColumnLabel0_->setText( tr( "Minimum" ) );
    maxColumnLabel0_->setText( tr( "Maximum" ) );

    investAmountLabel_->setText( tr( "Investment Amount ($)" ) );
    minInvestAmount_->setToolTip( tr( "Net investment amount (i.e. margin amount minus premium) below this amount will be filtered out." ) );
    maxInvestAmount_->setToolTip( tr( "Net investment amount (i.e. margin amount minus premium) above this amount will be filtered out." ) );

    lossAmountLabel_->setText( tr( "Max. Loss Allowed ($)" ) );
    maxLossAmount_->setToolTip( tr( "Trades with a maximum loss above this amount will be filtered out." ) );

    gainAmountLabel_->setText( tr( "Min. Gain Desired ($)" ) );
    minGainAmount_->setToolTip( tr( "Trades with a minimum gain below this amount will be filtered out. Set to $0.01 to only see trades that can net you money." ) );

    bidSizeLabel_->setText( tr( "Min. Bid Size" ) );
    minBidSize_->setToolTip( tr( "Options with bid sizes below this value will be filtered out. Set to 1 to only see actively traded options." ) );

    askSizeLabel_->setText( tr( "Min. Ask Size" ) );
    minAskSize_->setToolTip( tr( "Options with ask sizes below this value will be filtered out. Set to 1 to only see actively traded options." ) );

    spreadPercentLabel_->setText( tr( "Max. Bid/Ask Spread Ratio (%)" ) );
    maxSpreadPercent_->setToolTip( tr( "Trades with a spread percent above this value will be filtered out. Actively traded options have smaller spreads." ) );

    daysToExpiryLabel_->setText( tr( "Days To Expiration (DTE)" ) );
    minDaysToExpiry_->setToolTip( tr( "Options with days to expiration below this amount will be filtered out.") );
    maxDaysToExpiry_->setToolTip( tr( "Options with days to expiration above this amount will be filtered out.") );

    implVolatilityLabel_->setText( tr( "Implied Volatility" ) );
    minImplVolatility_->setToolTip( tr( "Trades with VI below this value will be filtered out." ) );
    maxImplVolatility_->setToolTip( tr( "Trades with VI above this value will be filtered out." ) );

    divAmountLabel_->setText( tr( "Est. Dividend Amount ($)" ) );
    minDivAmount_->setToolTip( tr( "Options with expected dividend amounts (dividends paid before expiration) below this amount will be filtered out. Dividends are estimated from prior payout history." ) );
    maxDivAmount_->setToolTip( tr( "Options with expected dividend amounts (dividends paid before expiration) above this amount will be filtered out. Dividends are estimated from prior payout history." ) );

    divYieldLabel_->setText( tr( "Est. Dividend Yield (%)" ) );
    minDivYield_->setToolTip( tr( "Options with expected dividend yields (dividends paid before expiration) below this value will be filtered out. Dividends are estimated from prior payout history." ) );
    maxDivYield_->setToolTip( tr( "Options with expected dividend yields (dividends paid before expiration) above this value will be filtered out. Dividends are estimated from prior payout history." ) );

    tabs_->setTabText( 1, tr( "Calculations" ) );

    minColumnLabel1_->setText( tr( "Minimum" ) );
    maxColumnLabel1_->setText( tr( "Maximum" ) );

    probITMLabel_->setText( tr( "Probability ITM (%)" ) );
    minProbITM_->setToolTip( tr( "Options with a probability of being in the money below this value will be filtered out. ITM probability is estimated from put and call deltas." ) );
    maxProbITM_->setToolTip( tr( "Options with a probability of being in the money above this value will be filtered out. ITM probability is estimated from put and call deltas." ) );

    probOTMLabel_->setText( tr( "Probability OTM (%)" ) );
    minProbOTM_->setToolTip( tr( "Options with a probability of being out of money below this value will be filtered out. OTM probability is estimated from put and call deltas." ) );
    maxProbOTM_->setToolTip( tr( "Options with a probability of being out of money above this value will be filtered out. OTM probability is estimated from put and call deltas." ) );

    probProfitLabel_->setText( tr( "Probability of Profit (%)" ) );
    minProbProfit_->setToolTip( tr( "Options with a probability of profit below this value will be filtered out. Probability of profit is estimated from break even and ITM or OTM probability." ) );
    maxProbProfit_->setToolTip( tr( "Options with a probability of profit above this value will be filtered out. Probability of profit is estimated from break even and ITM or OTM probability." ) );

    returnOnRiskLabel_->setText( tr( "Return on Risk (%)" ) );
    minReturnOnRisk_->setToolTip( tr( "Options with a return on risk below this value will be filtered out." ) );
    maxReturnOnRisk_->setToolTip( tr( "Options with a return on risk above this value will be filtered out." ) );

    returnOnRiskTimeLabel_->setText( tr( "Return on Risk / Time (%)" ) );
    minReturnOnRiskTime_->setToolTip( tr( "Options with a return on risk divided by time below this value will be filtered out." ) );
    maxReturnOnRiskTime_->setToolTip( tr( "Options with a return on risk divided by time above this value will be filtered out." ) );

    returnOnInvestmentLabel_->setText( tr( "Return on Investment (%)" ) );
    minReturnOnInvestment_->setToolTip( tr( "Options with a return on investment below this value will be filtered out." ) );
    maxReturnOnInvestment_->setToolTip( tr( "Options with a return on investment above this value will be filtered out." ) );

    returnOnInvestmentTimeLabel_->setText( tr( "Return on Investment / Time (%)" ) );
    minReturnOnInvestmentTime_->setToolTip( tr( "Options with a return on investment divided by time below this value will be filtered out." ) );
    maxReturnOnInvestmentTime_->setToolTip( tr( "Options with a return on investment divided by time above this value will be filtered out." ) );

    expectedValueLabel_->setText( tr( "Expected Value ($)" ) );
    minExpectedValue_->setToolTip( tr( "Options expecting to an amount below this value will be filtered out. Expected value is calcuated from max gain and option chain ITM and OTM probabilities." ) );
    maxExpectedValue_->setToolTip( tr( "Options expecting to an amount above this value will be filtered out. Expected value is calcuated from max gain and option chain ITM and OTM probabilities." ) );

    expectedValueReturnOnInvestmentLabel_->setText( tr( "Expected Value ROI (%)" ) );
    minExpectedValueReturnOnInvestment_->setToolTip( tr( "Option expected return on investment below this value will be filtered out. Expected value is calcuated from max gain and option chain ITM and OTM probabilities." ) );
    maxExpectedValueReturnOnInvestment_->setToolTip( tr( "Option expected return on investment above this value will be filtered out. Expected value is calcuated from max gain and option chain ITM and OTM probabilities." ) );

    expectedValueReturnOnInvestmentTimeLabel_->setText( tr( "Expected Value ROI / Time (%)" ) );
    minExpectedValueReturnOnInvestmentTime_->setToolTip( tr( "Option expected return on investment over time below this value will be filtered out. Expected value is calcuated from max gain and option chain ITM and OTM probabilities." ) );
    maxExpectedValueReturnOnInvestmentTime_->setToolTip( tr( "Option expected return on investment over time above this value will be filtered out. Expected value is calcuated from max gain and option chain ITM and OTM probabilities." ) );

    optionTypes_->setTitle( tr( "Option Type" ) );

    itmCalls_->setText( tr( "ITM Calls" ) );
    otmCalls_->setText( tr( "OTM Calls" ) );
    itmPuts_->setText( tr( "ITM Puts" ) );
    otmPuts_->setText( tr( "OTM Puts" ) );

    optionTradingStrats_->setTitle( tr( "Option Trading Strategy" ) );

    single_->setText( tr( "Single (CSP and CC)" ) );
    vertical_->setText( tr( "Vertical (Credit Spread)" ) );
    calendar_->setText( tr( "Calendar" ) );
    strangle_->setText( tr( "Strangle" ) );
    straddle_->setText( tr( "Straddle" ) );
    butterfly_->setText( tr( "Butterfly" ) );
    condor_->setText( tr( "Iron Condor" ) );
    diagonal_->setText( tr( "Diagonal" ) );
    collar_->setText( tr( "Collar" ) );

    pricing_->setTitle( tr( "Pricing" ) );

    theoPriceLessThanMarket_->setText( tr( "Theo. Price <= Market" ) );
    theoPriceGreaterThanMarket_->setText( tr( "Theo. Price > Market" ) );

    volatility_->setTitle( tr( "Volatility" ) );

    histLessThanImpl_->setText( tr( "Hist. Vol. <= Implied Vol." ) );
    histGreaterThanImpl_->setText( tr( "Hist. Vol. > Implied Vol." ) );

    okay_->setText( tr( "Okay" ) );
    cancel_->setText( tr( "Cancel" ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterEditorDialog::resizeEvent( QResizeEvent *event )
{
    // set label sizes
    if ( !sized_ )
    {
        const int w( (width() * 3) / 10 );

        // top label in each form layout
        underlyingPriceLabel_->setMinimumWidth( w );
        investAmountLabel_->setMinimumWidth( w );
        probITMLabel_->setMinimumWidth( w );

        sized_ = true;
    }

    // resize!
    _Mybase::resizeEvent( event );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterEditorDialog::onDoubleSpinBoxValueChanged( double value )
{
    QDoubleSpinBox *w( qobject_cast<QDoubleSpinBox*>( sender() ) );

    if ( !w )
        return;

    QPalette p( w->palette() );
    p.setColor( QPalette::Text, (value < 0.0) ? Qt::red : palette().color( QPalette::Text ) );

    w->setPalette( p );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterEditorDialog::onButtonClicked()
{
    // okay
    if ( okay_ == sender() )
    {
        // set filters
        f_.setMinUnderlyingPrice( minUnderlyingPrice_->value() );
        f_.setMaxUnderlyingPrice( maxUnderlyingPrice_->value() );

        f_.setMinInvestAmount( minInvestAmount_->value() );
        f_.setMaxInvestAmount( maxInvestAmount_->value() );

        f_.setMaxLossAmount( maxLossAmount_->value() );
        f_.setMinGainAmount( minGainAmount_->value() );

        f_.setMinBidSize( minBidSize_->value() );
        f_.setMinAskSize( minAskSize_->value() );

        f_.setMaxSpreadPercent( maxSpreadPercent_->value() );

        f_.setMinDaysToExpiry( minDaysToExpiry_->value() );
        f_.setMaxDaysToExpiry( maxDaysToExpiry_->value() );

        f_.setMinVolatility( minImplVolatility_->value() );
        f_.setMaxVolatility( maxImplVolatility_->value() );

        f_.setMinDividendAmount( minDivAmount_->value() );
        f_.setMaxDividendAmount( maxDivAmount_->value() );

        f_.setMinDividendYield( minDivYield_->value() );
        f_.setMaxDividendYield( maxDivYield_->value() );

        f_.setMinProbITM( minProbITM_->value() );
        f_.setMaxProbITM( maxProbITM_->value() );

        f_.setMinProbOTM( minProbOTM_->value() );
        f_.setMaxProbOTM( maxProbOTM_->value() );

        f_.setMinProbProfit( minProbProfit_->value() );
        f_.setMaxProbProfit( maxProbProfit_->value() );

        f_.setMinReturnOnRisk( minReturnOnRisk_->value() );
        f_.setMaxReturnOnRisk( maxReturnOnRisk_->value() );

        f_.setMinReturnOnRiskTime( minReturnOnRiskTime_->value() );
        f_.setMaxReturnOnRiskTime( maxReturnOnRiskTime_->value() );

        f_.setMinReturnOnInvestment( minReturnOnInvestment_->value() );
        f_.setMaxReturnOnInvestment( maxReturnOnInvestment_->value() );

        f_.setMinReturnOnInvestmentTime( minReturnOnInvestmentTime_->value() );
        f_.setMaxReturnOnInvestmentTime( maxReturnOnInvestmentTime_->value() );

        f_.setMinExpectedValue( minExpectedValue_->value() );
        f_.setMaxExpectedValue( maxExpectedValue_->value() );

        f_.setMinExpectedValueReturnOnInvestment( minExpectedValueReturnOnInvestment_->value() );
        f_.setMaxExpectedValueReturnOnInvestment( maxExpectedValueReturnOnInvestment_->value() );

        f_.setMinExpectedValueReturnOnInvestmentTime( minExpectedValueReturnOnInvestmentTime_->value() );
        f_.setMaxExpectedValueReturnOnInvestmentTime( maxExpectedValueReturnOnInvestmentTime_->value() );

        f_.setVerticalDepth( verticalDepth_->value() );

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

        int priceFilter( 0 );
        priceFilter |= ( theoPriceLessThanMarket_->isChecked() ? OptionProfitCalculatorFilter::THEO_LTE_MARKET : 0 );
        priceFilter |= ( theoPriceGreaterThanMarket_->isChecked() ? OptionProfitCalculatorFilter::THEO_GT_MARKET : 0 );

        f_.setPriceFilter( (OptionProfitCalculatorFilter::PriceFilter) priceFilter );

        int volatilityFilter( 0 );
        volatilityFilter |= ( histLessThanImpl_->isChecked() ? OptionProfitCalculatorFilter::HV_LTE_VI : 0 );
        volatilityFilter |= ( histGreaterThanImpl_->isChecked() ? OptionProfitCalculatorFilter::HV_GT_VI : 0 );

        f_.setVolatilityFilter( (OptionProfitCalculatorFilter::VolatilityFilter) volatilityFilter );

        accept();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterEditorDialog::initialize()
{
    tab0_ = new QWidget( this );

    minColumnLabel0_ = new QLabel( tab0_ );

    maxColumnLabel0_ = new QLabel( tab0_ );

    investAmountLabel_ = new QLabel( tab0_ );

    minInvestAmount_ = new QDoubleSpinBox( tab0_ );
    minInvestAmount_->setDecimals( 2 );
    minInvestAmount_->setMinimum( 0.0 );
    minInvestAmount_->setMaximum( 99999999.99 );

    maxInvestAmount_ = new QDoubleSpinBox( tab0_ );
    maxInvestAmount_->setDecimals( 2 );
    maxInvestAmount_->setMinimum( 0.0 );
    maxInvestAmount_->setMaximum( 99999999.99 );

    lossAmountLabel_ = new QLabel( tab0_ );

    maxLossAmount_ = new QDoubleSpinBox( tab0_ );
    maxLossAmount_->setDecimals( 2 );
    maxLossAmount_->setMinimum( 0.0 );
    maxLossAmount_->setMaximum( 99999999.99 );

    gainAmountLabel_ = new QLabel( tab0_ );

    minGainAmount_ = new QDoubleSpinBox( tab0_ );
    minGainAmount_->setDecimals( 2 );
    minGainAmount_->setMinimum( 0.0 );
    minGainAmount_->setMaximum( 99999999.99 );

    bidSizeLabel_ = new QLabel( tab0_ );

    minBidSize_ = new QSpinBox( tab0_ );
    minBidSize_->setMinimum( 0 );
    minBidSize_->setMaximum( 99999999 );

    askSizeLabel_ = new QLabel( tab0_ );

    minAskSize_ = new QSpinBox( tab0_ );
    minAskSize_->setMinimum( 0 );
    minAskSize_->setMaximum( 99999999 );

    spreadPercentLabel_ = new QLabel( tab0_ );

    maxSpreadPercent_ = new QDoubleSpinBox( tab0_ );
    maxSpreadPercent_->setDecimals( 2 );
    maxSpreadPercent_->setMinimum( 0.0 );
    maxSpreadPercent_->setMaximum( 100.0 );

    daysToExpiryLabel_ = new QLabel( tab0_ );

    minDaysToExpiry_ = new QSpinBox( tab0_ );
    minDaysToExpiry_->setMinimum( 0 );

    maxDaysToExpiry_ = new QSpinBox( tab0_ );
    maxDaysToExpiry_->setMinimum( 0 );
    maxDaysToExpiry_->setMaximum( 99999999 );

    implVolatilityLabel_ = new QLabel( tab0_ );

    minImplVolatility_ = new QDoubleSpinBox( tab0_ );
    minImplVolatility_->setDecimals( 2 );
    minImplVolatility_->setMinimum( 0.0 );
    minImplVolatility_->setMaximum( 99999.99 );

    maxImplVolatility_ = new QDoubleSpinBox( tab0_ );
    maxImplVolatility_->setDecimals( 2 );
    maxImplVolatility_->setMinimum( 0.0 );
    maxImplVolatility_->setMaximum( 99999.99 );

    divAmountLabel_ = new QLabel( tab0_ );

    minDivAmount_ = new QDoubleSpinBox( tab0_ );
    minDivAmount_->setDecimals( 2 );
    minDivAmount_->setMinimum( 0.0 );
    minDivAmount_->setMaximum( 99999.99 );

    maxDivAmount_ = new QDoubleSpinBox( tab0_ );
    maxDivAmount_->setDecimals( 2 );
    maxDivAmount_->setMinimum( 0.0 );
    maxDivAmount_->setMaximum( 99999.99 );

    divYieldLabel_ = new QLabel( tab0_ );

    minDivYield_ = new QDoubleSpinBox( tab0_ );
    minDivYield_->setDecimals( 2 );
    minDivYield_->setMinimum( 0.0 );
    minDivYield_->setMaximum( 99999.99 );

    maxDivYield_ = new QDoubleSpinBox( tab0_ );
    maxDivYield_->setDecimals( 2 );
    maxDivYield_->setMinimum( 0.0 );
    maxDivYield_->setMaximum( 99999.99 );

    // ---- //

    tab1_ = new QWidget( this );

    minColumnLabel1_ = new QLabel( tab1_ );

    maxColumnLabel1_ = new QLabel( tab1_ );

    probITMLabel_ = new QLabel( tab1_ );

    minProbITM_ = new QDoubleSpinBox( tab1_ );
    minProbITM_->setDecimals( 2 );
    minProbITM_->setMinimum( 0.0 );
    minProbITM_->setMaximum( 100.0 );

    maxProbITM_ = new QDoubleSpinBox( tab1_ );
    maxProbITM_->setDecimals( 2 );
    maxProbITM_->setMinimum( 0.0 );
    maxProbITM_->setMaximum( 100.0 );

    probOTMLabel_ = new QLabel( tab1_ );

    minProbOTM_ = new QDoubleSpinBox( tab1_ );
    minProbOTM_->setDecimals( 2 );
    minProbOTM_->setMinimum( 0.0 );
    minProbOTM_->setMaximum( 100.0 );

    maxProbOTM_ = new QDoubleSpinBox( tab1_ );
    maxProbOTM_->setDecimals( 2 );
    maxProbOTM_->setMinimum( 0.0 );
    maxProbOTM_->setMaximum( 100.0 );

    probProfitLabel_ = new QLabel( tab1_ );

    minProbProfit_ = new QDoubleSpinBox( tab1_ );
    minProbProfit_->setDecimals( 2 );
    minProbProfit_->setMinimum( 0.0 );
    minProbProfit_->setMaximum( 100.0 );

    maxProbProfit_ = new QDoubleSpinBox( tab1_ );
    maxProbProfit_->setDecimals( 2 );
    maxProbProfit_->setMinimum( 0.0 );
    maxProbProfit_->setMaximum( 100.0 );

    returnOnRiskLabel_ = new QLabel( tab1_ );

    minReturnOnRisk_ = new QDoubleSpinBox( tab1_ );
    minReturnOnRisk_->setDecimals( 2 );
    minReturnOnRisk_->setMinimum( -99999999.99 );
    minReturnOnRisk_->setMaximum( 99999999.99 );

    connect( minReturnOnRisk_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    maxReturnOnRisk_ = new QDoubleSpinBox( tab1_ );
    maxReturnOnRisk_->setDecimals( 2 );
    maxReturnOnRisk_->setMinimum( -99999999.99 );
    maxReturnOnRisk_->setMaximum( 99999999.99 );

    connect( maxReturnOnRisk_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    returnOnRiskTimeLabel_ = new QLabel( tab1_ );

    minReturnOnRiskTime_ = new QDoubleSpinBox( tab1_ );
    minReturnOnRiskTime_->setDecimals( 2 );
    minReturnOnRiskTime_->setMinimum( -99999999.99 );
    minReturnOnRiskTime_->setMaximum( 99999999.99 );

    connect( minReturnOnRiskTime_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    maxReturnOnRiskTime_ = new QDoubleSpinBox( tab1_ );
    maxReturnOnRiskTime_->setDecimals( 2 );
    maxReturnOnRiskTime_->setMinimum( -99999999.99 );
    maxReturnOnRiskTime_->setMaximum( 99999999.99 );

    connect( maxReturnOnRiskTime_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    returnOnInvestmentLabel_ = new QLabel( tab1_ );

    minReturnOnInvestment_ = new QDoubleSpinBox( tab1_ );
    minReturnOnInvestment_->setDecimals( 2 );
    minReturnOnInvestment_->setMinimum( -99999999.99 );
    minReturnOnInvestment_->setMaximum( 99999999.99 );

    connect( minReturnOnInvestment_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    maxReturnOnInvestment_ = new QDoubleSpinBox( tab1_ );
    maxReturnOnInvestment_->setDecimals( 2 );
    maxReturnOnInvestment_->setMinimum( -99999999.99 );
    maxReturnOnInvestment_->setMaximum( 99999999.99 );

    connect( maxReturnOnInvestment_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    returnOnInvestmentTimeLabel_ = new QLabel( tab1_ );

    minReturnOnInvestmentTime_ = new QDoubleSpinBox( tab1_ );
    minReturnOnInvestmentTime_->setDecimals( 2 );
    minReturnOnInvestmentTime_->setMinimum( -99999999.99 );
    minReturnOnInvestmentTime_->setMaximum( 99999999.99 );

    connect( minReturnOnInvestmentTime_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    maxReturnOnInvestmentTime_ = new QDoubleSpinBox( tab1_ );
    maxReturnOnInvestmentTime_->setDecimals( 2 );
    maxReturnOnInvestmentTime_->setMinimum( -99999999.99 );
    maxReturnOnInvestmentTime_->setMaximum( 99999999.99 );

    connect( maxReturnOnInvestmentTime_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    expectedValueLabel_ = new QLabel( tab1_ );

    minExpectedValue_ = new QDoubleSpinBox( tab1_ );
    minExpectedValue_->setDecimals( 2 );
    minExpectedValue_->setMinimum( -99999999.99 );
    minExpectedValue_->setMaximum( 99999999.99 );

    connect( minExpectedValue_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    maxExpectedValue_ = new QDoubleSpinBox( tab1_ );
    maxExpectedValue_->setDecimals( 2 );
    maxExpectedValue_->setMinimum( -99999999.99 );
    maxExpectedValue_->setMaximum( 99999999.99 );

    connect( maxExpectedValue_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    expectedValueReturnOnInvestmentLabel_ = new QLabel( tab1_ );

    minExpectedValueReturnOnInvestment_ = new QDoubleSpinBox( tab1_ );
    minExpectedValueReturnOnInvestment_->setDecimals( 2 );
    minExpectedValueReturnOnInvestment_->setMinimum( -99999999.99 );
    minExpectedValueReturnOnInvestment_->setMaximum( 99999999.99 );

    connect( minExpectedValueReturnOnInvestment_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    maxExpectedValueReturnOnInvestment_ = new QDoubleSpinBox( tab1_ );
    maxExpectedValueReturnOnInvestment_->setDecimals( 2 );
    maxExpectedValueReturnOnInvestment_->setMinimum( -99999999.99 );
    maxExpectedValueReturnOnInvestment_->setMaximum( 99999999.99 );

    connect( maxExpectedValueReturnOnInvestment_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    expectedValueReturnOnInvestmentTimeLabel_ = new QLabel( tab1_ );

    minExpectedValueReturnOnInvestmentTime_ = new QDoubleSpinBox( tab1_ );
    minExpectedValueReturnOnInvestmentTime_->setDecimals( 2 );
    minExpectedValueReturnOnInvestmentTime_->setMinimum( -99999999.99 );
    minExpectedValueReturnOnInvestmentTime_->setMaximum( 99999999.99 );

    connect( minExpectedValueReturnOnInvestmentTime_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    maxExpectedValueReturnOnInvestmentTime_ = new QDoubleSpinBox( tab1_ );
    maxExpectedValueReturnOnInvestmentTime_->setDecimals( 2 );
    maxExpectedValueReturnOnInvestmentTime_->setMinimum( -99999999.99 );
    maxExpectedValueReturnOnInvestmentTime_->setMaximum( 99999999.99 );

    connect( maxExpectedValueReturnOnInvestmentTime_, static_cast<void(QDoubleSpinBox::*)(double)>( &QDoubleSpinBox::valueChanged ), this, &_Myt::onDoubleSpinBoxValueChanged );

    // ---- //

    minColumnLabel_ = new QLabel( tab0_ );

    maxColumnLabel_ = new QLabel( tab0_ );

    underlyingPriceLabel_ = new QLabel( this );

    minUnderlyingPrice_ = new QDoubleSpinBox( this );
    minUnderlyingPrice_->setDecimals( 2 );
    minUnderlyingPrice_->setMinimum( 0.0 );
    minUnderlyingPrice_->setMaximum( 99999999.99 );

    maxUnderlyingPrice_ = new QDoubleSpinBox( this );
    maxUnderlyingPrice_->setDecimals( 2 );
    maxUnderlyingPrice_->setMinimum( 0.0 );
    maxUnderlyingPrice_->setMaximum( 99999999.99 );

    verticalDepthLabel_ = new QLabel( this );

    verticalDepth_ = new QSpinBox( this );
    verticalDepth_->setMinimum( 1 );

    tabs_ = new QTabWidget( this );
    tabs_->addTab( tab0_, QString() );
    tabs_->addTab( tab1_, QString() );

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

    pricing_ = new QGroupBox( this );

    theoPriceLessThanMarket_ = new QCheckBox( pricing_ );

    theoPriceGreaterThanMarket_ = new QCheckBox( pricing_ );

    volatility_ = new QGroupBox( this );

    histLessThanImpl_ = new QCheckBox( volatility_ );

    histGreaterThanImpl_ = new QCheckBox( volatility_ );

    // okay
    okay_ = new QPushButton( this );
    okay_->setDefault( true );

    connect( okay_, &QPushButton::clicked, this, &_Myt::onButtonClicked );

    // cancel
    cancel_ = new QPushButton( this );

    connect( cancel_, &QPushButton::clicked, this, &_Myt::reject );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void FilterEditorDialog::createLayout()
{
    QHBoxLayout *header0( new QHBoxLayout() );
    header0->addWidget( minColumnLabel0_ );
    header0->addWidget( maxColumnLabel0_ );

    QHBoxLayout *investAmount( new QHBoxLayout() );
    investAmount->addWidget( minInvestAmount_ );
    investAmount->addWidget( maxInvestAmount_ );

    QHBoxLayout *daysToExpiry( new QHBoxLayout() );
    daysToExpiry->addWidget( minDaysToExpiry_ );
    daysToExpiry->addWidget( maxDaysToExpiry_ );

    QHBoxLayout *implVolatility( new QHBoxLayout() );
    implVolatility->addWidget( minImplVolatility_ );
    implVolatility->addWidget( maxImplVolatility_ );

    QHBoxLayout *divAmount( new QHBoxLayout() );
    divAmount->addWidget( minDivAmount_ );
    divAmount->addWidget( maxDivAmount_ );

    QHBoxLayout *divYield( new QHBoxLayout() );
    divYield->addWidget( minDivYield_ );
    divYield->addWidget( maxDivYield_ );

    QFormLayout *filters0( new QFormLayout( tab0_ ) );
    filters0->addRow( new QLabel(), header0 );
    filters0->addRow( investAmountLabel_, investAmount );
    filters0->addRow( lossAmountLabel_, maxLossAmount_ );
    filters0->addRow( gainAmountLabel_, minGainAmount_ );
    filters0->addItem( new QSpacerItem( 16, 16 ) );
    filters0->addRow( bidSizeLabel_, minBidSize_ );
    filters0->addRow( askSizeLabel_, minAskSize_ );
    filters0->addRow( spreadPercentLabel_, maxSpreadPercent_ );
    filters0->addItem( new QSpacerItem( 16, 16 ) );
    filters0->addRow( daysToExpiryLabel_, daysToExpiry );
    filters0->addRow( implVolatilityLabel_, implVolatility );
    filters0->addItem( new QSpacerItem( 16, 16 ) );
    filters0->addRow( divAmountLabel_, divAmount );
    filters0->addRow( divYieldLabel_, divYield );

    // ---- //

    QHBoxLayout *header1( new QHBoxLayout() );
    header1->addWidget( minColumnLabel1_ );
    header1->addWidget( maxColumnLabel1_ );

    QHBoxLayout *probITM( new QHBoxLayout() );
    probITM->addWidget( minProbITM_ );
    probITM->addWidget( maxProbITM_ );

    QHBoxLayout *probOTM( new QHBoxLayout() );
    probOTM->addWidget( minProbOTM_ );
    probOTM->addWidget( maxProbOTM_ );

    QHBoxLayout *probProfit( new QHBoxLayout() );
    probProfit->addWidget( minProbProfit_ );
    probProfit->addWidget( maxProbProfit_ );

    QHBoxLayout *returnOnRisk( new QHBoxLayout() );
    returnOnRisk->addWidget( minReturnOnRisk_ );
    returnOnRisk->addWidget( maxReturnOnRisk_ );

    QHBoxLayout *returnOnRiskTime( new QHBoxLayout() );
    returnOnRiskTime->addWidget( minReturnOnRiskTime_ );
    returnOnRiskTime->addWidget( maxReturnOnRiskTime_ );

    QHBoxLayout *returnOnInvestment( new QHBoxLayout() );
    returnOnInvestment->addWidget( minReturnOnInvestment_ );
    returnOnInvestment->addWidget( maxReturnOnInvestment_ );

    QHBoxLayout *returnOnInvestmentTime( new QHBoxLayout() );
    returnOnInvestmentTime->addWidget( minReturnOnInvestmentTime_ );
    returnOnInvestmentTime->addWidget( maxReturnOnInvestmentTime_ );

    QHBoxLayout *expectedValue( new QHBoxLayout() );
    expectedValue->addWidget( minExpectedValue_ );
    expectedValue->addWidget( maxExpectedValue_ );

    QHBoxLayout *expectedValueReturnOnInvestment( new QHBoxLayout() );
    expectedValueReturnOnInvestment->addWidget( minExpectedValueReturnOnInvestment_ );
    expectedValueReturnOnInvestment->addWidget( maxExpectedValueReturnOnInvestment_ );

    QHBoxLayout *expectedValueReturnOnInvestmentTime( new QHBoxLayout() );
    expectedValueReturnOnInvestmentTime->addWidget( minExpectedValueReturnOnInvestmentTime_ );
    expectedValueReturnOnInvestmentTime->addWidget( maxExpectedValueReturnOnInvestmentTime_ );

    QFormLayout *filters1( new QFormLayout( tab1_ ) );
    filters1->addRow( new QLabel(), header1 );
    filters1->addRow( probITMLabel_, probITM );
    filters1->addRow( probOTMLabel_, probOTM );
    filters1->addRow( probProfitLabel_, probProfit );
    filters1->addItem( new QSpacerItem( 16, 16 ) );
    filters1->addRow( returnOnRiskLabel_, returnOnRisk );
    filters1->addRow( returnOnRiskTimeLabel_, returnOnRiskTime );
    filters1->addItem( new QSpacerItem( 16, 16 ) );
    filters1->addRow( returnOnInvestmentLabel_, returnOnInvestment );
    filters1->addRow( returnOnInvestmentTimeLabel_, returnOnInvestmentTime );
    filters1->addItem( new QSpacerItem( 16, 16 ) );
    filters1->addRow( expectedValueLabel_, expectedValue );
    filters1->addRow( expectedValueReturnOnInvestmentLabel_, expectedValueReturnOnInvestment );
    filters1->addRow( expectedValueReturnOnInvestmentTimeLabel_, expectedValueReturnOnInvestmentTime );

    // ---- //

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

    QVBoxLayout *pricing( new QVBoxLayout( pricing_ ) );
    pricing->addWidget( theoPriceLessThanMarket_ );
    pricing->addWidget( theoPriceGreaterThanMarket_ );
    pricing->addStretch();

    QVBoxLayout *volatility( new QVBoxLayout( volatility_ ) );
    volatility->addWidget( histLessThanImpl_ );
    volatility->addWidget( histGreaterThanImpl_ );
    volatility->addStretch();

    // ---- //

    QHBoxLayout *header( new QHBoxLayout() );
    header->addWidget( minColumnLabel_ );
    header->addWidget( maxColumnLabel_ );

    QHBoxLayout *underlyingPrice( new QHBoxLayout() );
    underlyingPrice->addWidget( minUnderlyingPrice_ );
    underlyingPrice->addWidget( maxUnderlyingPrice_ );

    QFormLayout *options( new QFormLayout() );
    options->addRow( new QLabel(), header );
    options->addRow( underlyingPriceLabel_, underlyingPrice );
    options->addRow( verticalDepthLabel_, verticalDepth_ );

    QHBoxLayout *groups( new QHBoxLayout() );
    groups->addWidget( optionTypes_ );
    groups->addWidget( optionTradingStrats_ );
    groups->addWidget( pricing_ );
    groups->addWidget( volatility_ );

    QHBoxLayout *buttons( new QHBoxLayout() );
    buttons->addStretch();
    buttons->addWidget( cancel_ );
    buttons->addWidget( okay_ );

    QVBoxLayout *form( new QVBoxLayout( this ) );
    form->addLayout( options );
    form->addItem( new QSpacerItem( 8, 8 ) );
    form->addWidget( tabs_ );
    form->addItem( new QSpacerItem( 8, 8 ) );
    form->addLayout( groups );
    form->addStretch();
    form->addLayout( buttons );
}

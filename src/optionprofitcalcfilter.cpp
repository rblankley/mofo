/**
 * @file optionprofitcalcfilter.cpp
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

#include "optionprofitcalcfilter.h"

#include <common.h>

#include <db/appdb.h>
#include <db/fundamentalstablemodel.h>
#include <db/optionchaintablemodel.h>
#include <db/optiontradingitemmodel.h>
#include <db/quotetablemodel.h>

#include <cmath>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

static const QString JSON_MIN_INVEST_AMOUNT( "minInvestAmount" );
static const QString JSON_MAX_INVEST_AMOUNT( "maxInvestAmount" );

static const QString JSON_MIN_UNDERLYING_PRICE( "minUnderlyingPrice" );
static const QString JSON_MAX_UNDERLYING_PRICE( "maxUnderlyingPrice" );

static const QString JSON_MAX_LOSS_AMOUNT( "maxLossAmount" );
static const QString JSON_MIN_GAIN_AMOUNT( "minGainAmount" );

static const QString JSON_MIN_BID_SIZE( "minBidSize" );
static const QString JSON_MIN_ASK_SIZE( "minAskSize" );

static const QString JSON_MIN_PROB_ITM( "minProbITM" );
static const QString JSON_MAX_PROB_ITM( "maxProbITM" );

static const QString JSON_MIN_PROB_OTM( "minProbOTM" );
static const QString JSON_MAX_PROB_OTM( "maxProbOTM" );

static const QString JSON_MIN_PROB_PROFIT( "minProbProfit" );
static const QString JSON_MAX_PROB_PROFIT( "maxProbProfit" );

static const QString JSON_MIN_DTE( "minDaysToExpiry" );
static const QString JSON_MAX_DTE( "maxDaysToExpiry" );

static const QString JSON_MIN_DIV_AMOUNT( "minDividendAmount" );
static const QString JSON_MAX_DIV_AMOUNT( "maxDividendAmount" );

static const QString JSON_MIN_DIV_YIELD( "minDividendYield" );
static const QString JSON_MAX_DIV_YIELD( "maxDividendYield" );

static const QString JSON_MIN_ROR( "minReturnOnRisk" );
static const QString JSON_MAX_ROR( "maxReturnOnRisk" );

static const QString JSON_MIN_ROR_TIME( "minReturnOnRiskTime" );
static const QString JSON_MAX_ROR_TIME( "maxReturnOnRiskTime" );

static const QString JSON_MIN_ROI( "minReturnOnInvestment" );
static const QString JSON_MAX_ROI( "maxReturnOnInvestment" );

static const QString JSON_MIN_ROI_TIME( "minReturnOnInvestmentTime" );
static const QString JSON_MAX_ROI_TIME( "maxReturnOnInvestmentTime" );

static const QString JSON_MIN_EV( "minExpectedValue" );
static const QString JSON_MAX_EV( "maxExpectedValue" );

static const QString JSON_MIN_EV_ROI( "minExpectedValueReturnOnInvestment" );
static const QString JSON_MAX_EV_ROI( "maxExpectedValueReturnOnInvestment" );

static const QString JSON_MIN_EV_ROI_TIME( "minExpectedValueReturnOnInvestmentTime" );
static const QString JSON_MAX_EV_ROI_TIME( "maxExpectedValueReturnOnInvestmentTime" );

static const QString JSON_MAX_SPREAD_PERCENT( "maxSpreadPercent" );

static const QString JSON_MIN_VI( "minVolatility" );
static const QString JSON_MAX_VI( "maxVolatility" );

static const QString JSON_OPTION_TYPES( "optionTypes" );
static const QString JSON_OPTION_TRADING_STRATS( "optionTradingStrats" );

static const QString JSON_PRICE( "price" );

static const QString JSON_VOLATILITY( "volatility" );

static const QString JSON_ADVANCED_FILTERS( "advancedFilters" );

static const QString JSON_VERT_DEPTH( "vertDepth" );


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
OptionProfitCalculatorFilter::OptionProfitCalculatorFilter() :
    minInvestAmount_( 0.0 ),
    maxInvestAmount_( 0.0 ),
    minUnderlyingPrice_( 0.0 ),
    maxUnderlyingPrice_( 0.0 ),
    maxLossAmount_( 0.0 ),
    minGainAmount_( 0.0 ),
    minBidSize_( 0 ),
    minAskSize_( 0 ),
    minProbITM_( 0.0 ),
    maxProbITM_( 0.0 ),
    minProbOTM_( 0.0 ),
    maxProbOTM_( 0.0 ),
    minProbProfit_( 0.0 ),
    maxProbProfit_( 0.0 ),
    minDaysToExpiry_( 0 ),
    maxDaysToExpiry_( 0 ),
    minDividendAmount_( 0.0 ),
    maxDividendAmount_( 0.0 ),
    minDividendYield_( 0.0 ),
    maxDividendYield_( 0.0 ),
    minReturnOnRisk_( 0.0 ),
    maxReturnOnRisk_( 0.0 ),
    minReturnOnRiskTime_( 0.0 ),
    maxReturnOnRiskTime_( 0.0 ),
    minReturnOnInvestment_( 0.0 ),
    maxReturnOnInvestment_( 0.0 ),
    minReturnOnInvestmentTime_( 0.0 ),
    maxReturnOnInvestmentTime_( 0.0 ),
    minExpectedValue_( 0.0 ),
    maxExpectedValue_( 0.0 ),
    minExpectedValueReturnOnInvestment_( 0.0 ),
    maxExpectedValueReturnOnInvestment_( 0.0 ),
    minExpectedValueReturnOnInvestmentTime_( 0.0 ),
    maxExpectedValueReturnOnInvestmentTime_( 0.0 ),
    maxSpreadPercent_( 0.0 ),
    minVolatility_( 0.0 ),
    maxVolatility_( 0.0 ),
    optionTypes_( ALL_OPTION_TYPES ),
    optionTradingStrats_( ALL_STRATEGIES ),
    price_( ALL_PRICES ),
    volatility_( ALL_VOLATILITY ),
    vertDepth_( DEFAULT_VERT_DEPTH ),
    q_( nullptr ),
    f_( nullptr ),
    oc_( nullptr ),
    ocr_( 0 ),
    t_( nullptr )
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculatorFilter::OptionProfitCalculatorFilter( const QByteArray& state ) :
    OptionProfitCalculatorFilter()
{
    restoreState( state );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculatorFilter::~OptionProfitCalculatorFilter()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionProfitCalculatorFilter::check( const QuoteTableModel *quote, const FundamentalsTableModel *fundamentals ) const
{
    // save values for future comparison
    q_ = quote;
    f_ = fundamentals;

    if ( !checkAdvancedFilters() )
    {
        LOG_TRACE << "failed advanced filters";
        return false;
    }

    // ---- //

    const double markPrice( quote->data0( QuoteTableModel::MARK ).toDouble() );

    const double divAmount( fundamentals->data0( QuoteTableModel::DIV_AMOUNT ).toDouble() );
    const double divYield( fundamentals->data0( QuoteTableModel::DIV_YIELD ).toDouble() );

    // check underlying (spot price)
    if (( 0.0 < minUnderlyingPrice() ) && ( markPrice < minUnderlyingPrice() ))
    {
        LOG_DEBUG << "spot price too low";
        return false;
    }
    else if (( 0.0 < maxUnderlyingPrice() ) && ( maxUnderlyingPrice() < markPrice ))
    {
        LOG_DEBUG << "spot price too high";
        return false;
    }

    // check dividend amount
    else if (( 0.0 < minDividendAmount() ) && ( divAmount < minDividendAmount() ))
    {
        LOG_DEBUG << "dividend amount too low";
        return false;
    }
    else if (( 0.0 < maxDividendAmount() ) && ( maxDividendAmount() < divAmount ))
    {
        LOG_DEBUG << "dividend amount too high";
        return false;
    }

    // check dividend yield
    else if (( 0.0 < minDividendYield() ) && ( divYield < minDividendYield() ))
    {
        LOG_DEBUG << "dividend yield too low";
        return false;
    }
    else if (( 0.0 < maxDividendYield() ) && ( maxDividendYield() < divYield ))
    {
        LOG_DEBUG << "dividend yield too high";
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionProfitCalculatorFilter::check( const OptionChainTableModel *chains, int row, bool isCall ) const
{
    // save values for future comparison
    oc_ = chains;
    ocr_ = row;

    if ( !checkAdvancedFilters() )
    {
        LOG_TRACE << "failed advanced filters";
        return false;
    }

    // ---- //

    const QDateTime now( AppDatabase::instance()->currentDateTime() );

    const double daysToExpiry( now.date().daysTo( chains->expirationDate() ) );

    // check filter for days to expiry
    if (( minDaysToExpiry() ) && ( daysToExpiry < minDaysToExpiry() ))
    {
        LOG_DEBUG << "DTE too low";
        return false;
    }
    else if (( maxDaysToExpiry() ) && ( maxDaysToExpiry() < daysToExpiry ))
    {
        LOG_DEBUG << "DTE to high";
        return false;
    }

    // ---- //

    bool itm;
    bool otm;

    int bidSize;
    int askSize;

    double spread;
    double spreadPercent;

    if ( isCall )
    {
        const bool isInTheMoney( chains->tableData( row, OptionChainTableModel::CALL_IS_IN_THE_MONEY ).toBool() );

        itm = (( ITM_CALLS & optionTypeFilter() ) && ( isInTheMoney ));
        otm = (( OTM_CALLS & optionTypeFilter() ) && ( !isInTheMoney ));

        bidSize = chains->tableData( row, OptionChainTableModel::CALL_BID_SIZE ).toInt();
        askSize = chains->tableData( row, OptionChainTableModel::CALL_ASK_SIZE ).toInt();

        spread = chains->tableData( row, OptionChainTableModel::CALL_ASK_PRICE ).toDouble() - chains->tableData( row, OptionChainTableModel::CALL_BID_PRICE ).toDouble();
        spreadPercent = spread / chains->tableData( row, OptionChainTableModel::CALL_ASK_PRICE ).toDouble();
    }
    else
    {
        const bool isInTheMoney( chains->tableData( row, OptionChainTableModel::PUT_IS_IN_THE_MONEY ).toBool() );

        itm = (( ITM_PUTS & optionTypeFilter() ) && ( isInTheMoney ));
        otm = (( OTM_PUTS & optionTypeFilter() ) && ( !isInTheMoney ));

        bidSize = chains->tableData( row, OptionChainTableModel::PUT_BID_SIZE ).toInt();
        askSize = chains->tableData( row, OptionChainTableModel::PUT_ASK_SIZE ).toInt();

        spread = chains->tableData( row, OptionChainTableModel::PUT_ASK_PRICE ).toDouble() - chains->tableData( row, OptionChainTableModel::PUT_BID_PRICE ).toDouble();
        spreadPercent = spread / chains->tableData( row, OptionChainTableModel::PUT_ASK_PRICE ).toDouble();
    }

    // check selected
    if (( !itm ) && ( !otm ))
    {
        LOG_TRACE << "not selected for processing";
        return false;
    }

    // check bid/ask size
    else if (( 0 < minBidSize() ) && ( bidSize < minBidSize() ))
    {
        LOG_DEBUG << "below min bid size";
        return false;
    }
    else if (( 0 < minAskSize() ) && ( askSize < minAskSize() ))
    {
        LOG_DEBUG << "below min ask size";
        return false;
    }

    // check spread percent
    else if (( 0.0 < maxSpreadPercent() ) &&
        ( std::isnormal( spread ) ) && ( MIN_SPREAD_AMOUNT < spread ) &&
        ( std::isnormal( spreadPercent ) ) && ( maxSpreadPercent() < spreadPercent ))
    {
        LOG_DEBUG << "above max spread percent";
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionProfitCalculatorFilter::check( const OptionTradingItemModel::ColumnValueMap& trade ) const
{
    // save values for future comparison
    t_ = &trade;

    if ( !checkAdvancedFilters() )
    {
        t_ = nullptr;

        LOG_TRACE << "failed advanced filters";
        return false;
    }

    t_ = nullptr;

    // ---- //

    // negative investments have zero risk!
    // either the VI is really high or option data is out of date
    const bool freeMoney( trade[OptionTradingItemModel::INVESTMENT_AMOUNT].toDouble() <= 0.0 );

    // investment amount in range
    if (( 0.0 < minInvestAmount() ) && ( trade[OptionTradingItemModel::INVESTMENT_AMOUNT].toDouble() < minInvestAmount() ) && ( !freeMoney ))
    {
        LOG_DEBUG << "below min investment amount";
        return false;
    }
    else if (( 0.0 < maxInvestAmount() ) && ( maxInvestAmount() < trade[OptionTradingItemModel::INVESTMENT_AMOUNT].toDouble() ))
    {
        LOG_DEBUG << "above max investment amount";
        return false;
    }

    // check max loss and min gain
    else if (( 0.0 < maxLossAmount() ) && ( maxLossAmount() < trade[OptionTradingItemModel::MAX_LOSS].toDouble() ) && ( !freeMoney ))
    {
        LOG_DEBUG << "above max loss amount";
        return false;
    }
    else if (( 0.0 < minGainAmount() ) && ( trade[OptionTradingItemModel::MAX_GAIN].toDouble() < minGainAmount() ))
    {
        LOG_DEBUG << "below min gain amount";
        return false;
    }

    // probability ITM
    else if (( 0.0 < minProbITM() ) && ( trade[OptionTradingItemModel::PROBABILITY_ITM].toDouble() < minProbITM() ))
    {
        LOG_DEBUG << "below min itm prob";
        return false;
    }
    else if (( 0.0 < maxProbITM() ) && ( maxProbITM() < trade[OptionTradingItemModel::PROBABILITY_ITM].toDouble() ))
    {
        LOG_DEBUG << "above max itm prob";
        return false;
    }

    // probability OTM
    else if (( 0.0 < minProbOTM() ) && ( trade[OptionTradingItemModel::PROBABILITY_OTM].toDouble() < minProbOTM() ))
    {
        LOG_DEBUG << "below min otm prob";
        return false;
    }
    else if (( 0.0 < maxProbOTM() ) && ( maxProbOTM() < trade[OptionTradingItemModel::PROBABILITY_OTM].toDouble() ))
    {
        LOG_DEBUG << "above max otm prob";
        return false;
    }

    // probability of profit
    else if (( 0.0 < minProbProfit() ) && ( trade[OptionTradingItemModel::PROBABILITY_PROFIT].toDouble() < minProbProfit() ))
    {
        LOG_DEBUG << "below min prob of profit";
        return false;
    }
    else if (( 0.0 < maxProbProfit() ) && ( maxProbProfit() < trade[OptionTradingItemModel::PROBABILITY_PROFIT].toDouble() ))
    {
        LOG_DEBUG << "above max prob of profit";
        return false;
    }

    // ROR
    else if (( 0.0 != minReturnOnRisk() ) && ( trade[OptionTradingItemModel::ROR].toDouble() < minReturnOnRisk() ) && ( !freeMoney ))
    {
        LOG_DEBUG << "below min return on risk";
        return false;
    }
    else if (( 0.0 != maxReturnOnRisk() ) && ( maxReturnOnRisk() < trade[OptionTradingItemModel::ROR].toDouble() ))
    {
        LOG_DEBUG << "above max return on risk";
        return false;
    }

    // ROR / Time
    else if (( 0.0 != minReturnOnRiskTime() ) && ( trade[OptionTradingItemModel::ROR_TIME].toDouble() < minReturnOnRiskTime() ) && ( !freeMoney ))
    {
        LOG_DEBUG << "below min return on risk / time";
        return false;
    }
    else if (( 0.0 != maxReturnOnRiskTime() ) && ( maxReturnOnRiskTime() < trade[OptionTradingItemModel::ROR_TIME].toDouble() ))
    {
        LOG_DEBUG << "above max return on risk / time";
        return false;
    }

    // ROI
    else if (( 0.0 != minReturnOnInvestment() ) && ( trade[OptionTradingItemModel::ROI].toDouble() < minReturnOnInvestment() ) && ( !freeMoney ))
    {
        LOG_DEBUG << "below min return on investment";
        return false;
    }
    else if (( 0.0 != maxReturnOnInvestment() ) && ( maxReturnOnInvestment() < trade[OptionTradingItemModel::ROI].toDouble() ))
    {
        LOG_DEBUG << "above max return on investment";
        return false;
    }

    // ROI / Time
    else if (( 0.0 != minReturnOnInvestmentTime() ) && ( trade[OptionTradingItemModel::ROI_TIME].toDouble() < minReturnOnInvestmentTime() ) && ( !freeMoney ))
    {
        LOG_DEBUG << "below min return on investment / time";
        return false;
    }
    else if (( 0.0 != maxReturnOnInvestmentTime() ) && ( maxReturnOnInvestmentTime() < trade[OptionTradingItemModel::ROI_TIME].toDouble() ))
    {
        LOG_DEBUG << "above max return on investment / time";
        return false;
    }

    // EV
    else if (( 0.0 != minExpectedValue() ) && ( trade[OptionTradingItemModel::EXPECTED_VALUE].toDouble() < minExpectedValue() ) && ( !freeMoney ))
    {
        LOG_DEBUG << "below min expected value";
        return false;
    }
    else if (( 0.0 != maxExpectedValue() ) && ( maxExpectedValue() < trade[OptionTradingItemModel::EXPECTED_VALUE].toDouble() ))
    {
        LOG_DEBUG << "above max expected value";
        return false;
    }

    // EV-ROI
    else if (( 0.0 != minExpectedValueReturnOnInvestment() ) && ( trade[OptionTradingItemModel::EXPECTED_VALUE_ROI].toDouble() < minExpectedValueReturnOnInvestment() ) && ( !freeMoney ))
    {
        LOG_DEBUG << "below min expected value / time";
        return false;
    }
    else if (( 0.0 != maxExpectedValueReturnOnInvestment() ) && ( maxExpectedValueReturnOnInvestment() < trade[OptionTradingItemModel::EXPECTED_VALUE_ROI].toDouble() ))
    {
        LOG_DEBUG << "above max expected value / time";
        return false;
    }

    // EV-ROI / Time
    else if (( 0.0 != minExpectedValueReturnOnInvestmentTime() ) && ( trade[OptionTradingItemModel::EXPECTED_VALUE_ROI_TIME].toDouble() < minExpectedValueReturnOnInvestmentTime() ) && ( !freeMoney ))
    {
        LOG_DEBUG << "below min EV-ROI / time";
        return false;
    }
    else if (( 0.0 != maxExpectedValueReturnOnInvestmentTime() ) && ( maxExpectedValueReturnOnInvestmentTime() < trade[OptionTradingItemModel::EXPECTED_VALUE_ROI_TIME].toDouble() ))
    {
        LOG_DEBUG << "above max EV-ROI / time";
        return false;
    }

    // volatility
    else if (( 0.0 < minVolatility() ) && ( trade[OptionTradingItemModel::CALC_THEO_VOLATILITY].toDouble() < minVolatility() ))
    {
        LOG_DEBUG << "below min volatility";
        return false;
    }
    else if (( 0.0 < maxVolatility() ) && ( maxVolatility() < trade[OptionTradingItemModel::CALC_THEO_VOLATILITY].toDouble() ))
    {
        LOG_DEBUG << "above max volatility";
        return false;
    }

    // theo price versus market price
    const bool theoPriceLowerThanMarket( THEO_LTE_MARKET & priceFilter() );
    const bool theoPriceHigherThanMarketd( THEO_GT_MARKET & priceFilter() );

    if (( -0.005 <= trade[OptionTradingItemModel::INVESTMENT_OPTION_PRICE_VS_THEO].toDouble() ) && ( !theoPriceLowerThanMarket ))
    {
        LOG_DEBUG << "theo price below market";
        return false;
    }
    else if (( trade[OptionTradingItemModel::INVESTMENT_OPTION_PRICE_VS_THEO].toDouble() < -0.005 ) && ( !theoPriceHigherThanMarketd ))
    {
        LOG_DEBUG << "theo price above market";
        return false;
    }

    // hist volatility versus implied volatility
    const bool histLowerThanImplied( HV_LTE_VI & volatilityFilter() );
    const bool histHigherThanImplied( HV_GT_VI & volatilityFilter() );

    if (( trade[OptionTradingItemModel::HIST_VOLATILITY].toDouble() <= trade[OptionTradingItemModel::CALC_THEO_VOLATILITY].toDouble() ) && ( !histLowerThanImplied ))
    {
        LOG_DEBUG << "hist vol lower than implied vol";
        return false;
    }
    else if (( trade[OptionTradingItemModel::CALC_THEO_VOLATILITY].toDouble() < trade[OptionTradingItemModel::HIST_VOLATILITY].toDouble() ) && ( !histHigherThanImplied ))
    {
        LOG_DEBUG << "hist vol higher than implied vol";
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QByteArray OptionProfitCalculatorFilter::saveState() const
{
    QJsonArray filters;

    foreach ( const QString& af, advancedFilters_ )
        filters.append( af );

    QJsonObject obj;
    obj[JSON_MIN_INVEST_AMOUNT] = minInvestAmount_;
    obj[JSON_MAX_INVEST_AMOUNT] = maxInvestAmount_;
    obj[JSON_MIN_UNDERLYING_PRICE] = minUnderlyingPrice_;
    obj[JSON_MAX_UNDERLYING_PRICE] = maxUnderlyingPrice_;
    obj[JSON_MAX_LOSS_AMOUNT] = maxLossAmount_;
    obj[JSON_MIN_GAIN_AMOUNT] = minGainAmount_;
    obj[JSON_MIN_BID_SIZE] = minBidSize_;
    obj[JSON_MIN_ASK_SIZE] = minAskSize_;
    obj[JSON_MIN_PROB_ITM] = minProbITM_;
    obj[JSON_MAX_PROB_ITM] = maxProbITM_;
    obj[JSON_MIN_PROB_OTM] = minProbOTM_;
    obj[JSON_MAX_PROB_OTM] = maxProbOTM_;
    obj[JSON_MIN_PROB_PROFIT] = minProbProfit_;
    obj[JSON_MAX_PROB_PROFIT] = maxProbProfit_;
    obj[JSON_MIN_DTE] = minDaysToExpiry_;
    obj[JSON_MAX_DTE] = maxDaysToExpiry_;
    obj[JSON_MIN_DIV_AMOUNT] = minDividendAmount_;
    obj[JSON_MAX_DIV_AMOUNT] = maxDividendAmount_;
    obj[JSON_MIN_DIV_YIELD] = minDividendYield_;
    obj[JSON_MAX_DIV_YIELD] = maxDividendYield_;
    obj[JSON_MIN_ROR] = minReturnOnRisk_;
    obj[JSON_MAX_ROR] = maxReturnOnRisk_;
    obj[JSON_MIN_ROR_TIME] = minReturnOnRiskTime_;
    obj[JSON_MAX_ROR_TIME] = maxReturnOnRiskTime_;
    obj[JSON_MIN_ROI] = minReturnOnInvestment_;
    obj[JSON_MAX_ROI] = maxReturnOnInvestment_;
    obj[JSON_MIN_ROI_TIME] = minReturnOnInvestmentTime_;
    obj[JSON_MAX_ROI_TIME] = maxReturnOnInvestmentTime_;
    obj[JSON_MIN_EV] = minExpectedValue_;
    obj[JSON_MAX_EV] = maxExpectedValue_;
    obj[JSON_MIN_EV_ROI] = minExpectedValueReturnOnInvestment_;
    obj[JSON_MAX_EV_ROI] = maxExpectedValueReturnOnInvestment_;
    obj[JSON_MIN_EV_ROI_TIME] = minExpectedValueReturnOnInvestmentTime_;
    obj[JSON_MAX_EV_ROI_TIME] = maxExpectedValueReturnOnInvestmentTime_;
    obj[JSON_MAX_SPREAD_PERCENT] = maxSpreadPercent_;
    obj[JSON_MIN_VI] = minVolatility_;
    obj[JSON_MAX_VI] = maxVolatility_;
    obj[JSON_OPTION_TYPES] = optionTypes_;
    obj[JSON_OPTION_TRADING_STRATS] = optionTradingStrats_;
    obj[JSON_PRICE] = price_;
    obj[JSON_VOLATILITY] = volatility_;
    obj[JSON_ADVANCED_FILTERS] = filters;
    obj[JSON_VERT_DEPTH] = vertDepth_;

    const QJsonDocument doc( obj );

    return doc.toJson( QJsonDocument::Compact );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionProfitCalculatorFilter::restoreState( const QByteArray& state )
{
    QJsonParseError e;

    const QJsonDocument doc( QJsonDocument::fromJson( state, &e ) );

    // check for error parsing state
    if ( QJsonParseError::NoError != e.error )
    {
        LOG_WARN << "error parsing state " << e.error << " " << qPrintable( e.errorString() ) << " offset " << e.offset;
        return;
    }
    else if ( !doc.isObject() )
    {
        LOG_WARN << "document not an object";
        return;
    }

    // restore member variables
    const QJsonObject obj( doc.object() );

    if ( obj.contains( JSON_MIN_INVEST_AMOUNT ) )
        minInvestAmount_ = obj[JSON_MIN_INVEST_AMOUNT].toDouble();
    if ( obj.contains( JSON_MAX_INVEST_AMOUNT ) )
        maxInvestAmount_ = obj[JSON_MAX_INVEST_AMOUNT].toDouble();

    if ( obj.contains( JSON_MIN_UNDERLYING_PRICE ) )
        minUnderlyingPrice_ = obj[JSON_MIN_UNDERLYING_PRICE].toDouble();
    if ( obj.contains( JSON_MAX_UNDERLYING_PRICE ) )
        maxUnderlyingPrice_ = obj[JSON_MAX_UNDERLYING_PRICE].toDouble();

    if ( obj.contains( JSON_MAX_LOSS_AMOUNT ) )
        maxLossAmount_ = obj[JSON_MAX_LOSS_AMOUNT].toDouble();
    if ( obj.contains( JSON_MIN_GAIN_AMOUNT ) )
        minGainAmount_ = obj[JSON_MIN_GAIN_AMOUNT].toDouble();

    if ( obj.contains( JSON_MIN_BID_SIZE ) )
        minBidSize_ = obj[JSON_MIN_BID_SIZE].toInt();
    if ( obj.contains( JSON_MIN_ASK_SIZE ) )
        minAskSize_ = obj[JSON_MIN_ASK_SIZE].toInt();

    if ( obj.contains( JSON_MIN_PROB_ITM ) )
        minProbITM_ = obj[JSON_MIN_PROB_ITM].toDouble();
    if ( obj.contains( JSON_MAX_PROB_ITM ) )
        maxProbITM_ = obj[JSON_MAX_PROB_ITM].toDouble();

    if ( obj.contains( JSON_MIN_PROB_OTM ) )
        minProbOTM_ = obj[JSON_MIN_PROB_OTM].toDouble();
    if ( obj.contains( JSON_MAX_PROB_OTM ) )
        maxProbOTM_ = obj[JSON_MAX_PROB_OTM].toDouble();

    if ( obj.contains( JSON_MIN_PROB_PROFIT ) )
        minProbProfit_ = obj[JSON_MIN_PROB_PROFIT].toDouble();
    if ( obj.contains( JSON_MAX_PROB_PROFIT ) )
        maxProbProfit_ = obj[JSON_MAX_PROB_PROFIT].toDouble();

    if ( obj.contains( JSON_MIN_DTE ) )
        minDaysToExpiry_ = obj[JSON_MIN_DTE].toInt();
    if ( obj.contains( JSON_MAX_DTE ) )
        maxDaysToExpiry_ = obj[JSON_MAX_DTE].toInt();

    if ( obj.contains( JSON_MIN_DIV_AMOUNT ) )
        minDividendAmount_ = obj[JSON_MIN_DIV_AMOUNT].toDouble();
    if ( obj.contains( JSON_MAX_DIV_AMOUNT ) )
        maxDividendAmount_ = obj[JSON_MAX_DIV_AMOUNT].toDouble();

    if ( obj.contains( JSON_MIN_DIV_YIELD ) )
        minDividendYield_ = obj[JSON_MIN_DIV_YIELD].toDouble();
    if ( obj.contains( JSON_MAX_DIV_YIELD ) )
        maxDividendYield_ = obj[JSON_MAX_DIV_YIELD].toDouble();

    if ( obj.contains( JSON_MIN_ROR ) )
        minReturnOnRisk_ = obj[JSON_MIN_ROR].toDouble();
    if ( obj.contains( JSON_MAX_ROR ) )
        maxReturnOnRisk_ = obj[JSON_MAX_ROR].toDouble();

    if ( obj.contains( JSON_MIN_ROR_TIME ) )
        minReturnOnRiskTime_ = obj[JSON_MIN_ROR_TIME].toDouble();
    if ( obj.contains( JSON_MAX_ROR_TIME ) )
        maxReturnOnRiskTime_ = obj[JSON_MAX_ROR_TIME].toDouble();

    if ( obj.contains( JSON_MIN_ROI ) )
        minReturnOnInvestment_ = obj[JSON_MIN_ROI].toDouble();
    if ( obj.contains( JSON_MAX_ROI ) )
        maxReturnOnInvestment_ = obj[JSON_MAX_ROI].toDouble();

    if ( obj.contains( JSON_MIN_ROI_TIME ) )
        minReturnOnInvestmentTime_ = obj[JSON_MIN_ROI_TIME].toDouble();
    if ( obj.contains( JSON_MAX_ROI_TIME ) )
        maxReturnOnInvestmentTime_ = obj[JSON_MAX_ROI_TIME].toDouble();

    if ( obj.contains( JSON_MIN_EV ) )
        minExpectedValue_ = obj[JSON_MIN_EV].toDouble();
    if ( obj.contains( JSON_MAX_EV ) )
        maxExpectedValue_ = obj[JSON_MAX_EV].toDouble();

    if ( obj.contains( JSON_MIN_EV_ROI ) )
        minExpectedValueReturnOnInvestment_ = obj[JSON_MIN_EV_ROI].toDouble();
    if ( obj.contains( JSON_MAX_EV_ROI ) )
        maxExpectedValueReturnOnInvestment_ = obj[JSON_MAX_EV_ROI].toDouble();

    if ( obj.contains( JSON_MIN_EV_ROI_TIME ) )
        minExpectedValueReturnOnInvestmentTime_ = obj[JSON_MIN_EV_ROI_TIME].toDouble();
    if ( obj.contains( JSON_MAX_EV_ROI_TIME ) )
        maxExpectedValueReturnOnInvestmentTime_ = obj[JSON_MAX_EV_ROI_TIME].toDouble();

    if ( obj.contains( JSON_MAX_SPREAD_PERCENT ) )
        maxSpreadPercent_ = obj[JSON_MAX_SPREAD_PERCENT].toDouble();

    if ( obj.contains( JSON_MIN_VI ) )
        minVolatility_ = obj[JSON_MIN_VI].toDouble();
    if ( obj.contains( JSON_MAX_VI ) )
        maxVolatility_ = obj[JSON_MAX_VI].toDouble();

    if ( obj.contains( JSON_OPTION_TYPES ) )
        optionTypes_ = (OptionTypeFilter) obj[JSON_OPTION_TYPES].toInt();
    if ( obj.contains( JSON_OPTION_TRADING_STRATS ) )
        optionTradingStrats_ = (OptionTradingStrategyFilter) obj[JSON_OPTION_TRADING_STRATS].toInt();

    if ( obj.contains( JSON_PRICE ) )
        price_ = (PriceFilter) obj[JSON_PRICE].toInt();

    if ( obj.contains( JSON_VOLATILITY ) )
        volatility_ = (VolatilityFilter) obj[JSON_VOLATILITY].toInt();

    if ( obj.contains( JSON_ADVANCED_FILTERS ) )
    {
        const QJsonArray filters( obj[JSON_ADVANCED_FILTERS].toArray() );

        foreach ( const QJsonValue& afVal, filters )
            advancedFilters_.append( afVal.toString() );
    }

    if ( obj.contains( JSON_VERT_DEPTH ) )
        vertDepth_ = obj[JSON_VERT_DEPTH].toInt();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionProfitCalculatorFilter::checkAdvancedFilters() const
{
    // check what tables we have available
    QStringList tables;

    if ( q_ )
    {
        tables.append( QUOTE_TABLE );
        tables.append( CHARTING );
    }

    if ( f_ )
        tables.append( FUNDAMENTALS_TABLE );

    if ( oc_ )
        tables.append( OPTION_CHAIN_TABLE );

    if ( t_ )
        tables.append( OPTION_TRADING_TABLE );

    // check each filter
    foreach ( const QString& f, advancedFilters_ )
    {
        const QStringList filter( f.split( "|" ) );

        if ( 3 != filter.size() )
            continue;

        // retrieve table
        const QStringList t0( filter[0].split( ":" ) );

        // check we are interested in this table
        if ( !tables.contains( t0[0] ) )
            continue;

        const QVariant v0( tableData( t0[0], t0[1] ) );

        // retrieve operand
        const QStringList op( filter[1].split( ":" ) );

        QVariant v1;

        if ( TABLE_TYPE == op[1] )
        {
            const QStringList t1( filter[2].split( ":" ) );

            // check we are interested in this table
            if ( !tables.contains( t1[0] ) )
                continue;

            v1 = tableData( t1[0], t1[1] );
        }
        else if ( STRING_VALUE == t0[2] )
            v1 = filter[2];
        else if ( INT_VALUE == t0[2] )
            v1 = filter[2].toInt();
        else if ( DOUBLE_VALUE == t0[2] )
            v1 = filter[2].toDouble();

        // validate
#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
        const QPartialOrdering result( QVariant::compare( v0, v1 ) );

        if (( QPartialOrdering::Equivalent == result ) && (( "LT" == op[0] ) || ( "GT" == op[0] ) || ( "NEQ" == op[0] )))
        {
            LOG_DEBUG << "advanced filter (eq) failed " << qPrintable( f ) << " " << qPrintable( v0.toString() ) << " " << qPrintable( v1.toString() );
            return false;
        }
        else if (( QPartialOrdering::Less == result ) && (( "GT" == op[0] ) || ( "GTE" == op[0] ) || ( "EQ" == op[0] )))
        {
            LOG_DEBUG << "advanced filter (l) failed " << qPrintable( f ) << " " << qPrintable( v0.toString() ) << " " << qPrintable( v1.toString() );
            return false;
        }
        else if (( QPartialOrdering::Greater == result ) && (( "LT" == op[0] ) || ( "LTE" == op[0] ) || ( "EQ" == op[0] )))
        {
            LOG_DEBUG << "advanced filter (g) failed " << qPrintable( f ) << " " << qPrintable( v0.toString() ) << " " << qPrintable( v1.toString() );
            return false;
        }
        else if ( QPartialOrdering::Unordered == result )
        {
            LOG_WARN << "advanced filter mismatched types " << qPrintable( f );
        }
#else
        if (( "EQ" == op[0] ) && ( v0 != v1 ))
        {
            LOG_DEBUG << "advanced filter (eq) failed " << qPrintable( f ) << " " << qPrintable( v0.toString() ) << " " << qPrintable( v1.toString() );
            return false;
        }
        else if (( "NEQ" == op[0] ) && ( v0 == v1 ))
        {
            LOG_DEBUG << "advanced filter (neq) failed " << qPrintable( f ) << " " << qPrintable( v0.toString() ) << " " << qPrintable( v1.toString() );
            return false;
        }
        else if (( "LT" == op[0] ) && ( v0 >= v1 ))
        {
            LOG_DEBUG << "advanced filter (lt) failed " << qPrintable( f ) << " " << qPrintable( v0.toString() ) << " " << qPrintable( v1.toString() );
            return false;
        }
        else if (( "LTE" == op[0] ) && ( v0 > v1 ))
        {
            LOG_DEBUG << "advanced filter (lte) failed " << qPrintable( f ) << " " << qPrintable( v0.toString() ) << " " << qPrintable( v1.toString() );
            return false;
        }
        else if (( "GT" == op[0] ) && ( v0 <= v1 ))
        {
            LOG_DEBUG << "advanced filter (gt) failed " << qPrintable( f ) << " " << qPrintable( v0.toString() ) << " " << qPrintable( v1.toString() );
            return false;
        }
        else if (( "GTE" == op[0] ) && ( v0 < v1 ))
        {
            LOG_DEBUG << "advanced filter (gte) failed " << qPrintable( f ) << " " << qPrintable( v0.toString() ) << " " << qPrintable( v1.toString() );
            return false;
        }
#endif
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QVariant OptionProfitCalculatorFilter::tableData( const QString& t, const QString& col ) const
{
    if (( QUOTE_TABLE == t ) && ( q_ ))
        return q_->data0( col.toInt() );
    else if (( FUNDAMENTALS_TABLE == t ) && ( f_ ))
        return f_->data0( col.toInt() );
    else if (( OPTION_CHAIN_TABLE == t ) && ( oc_ ))
        return oc_->data( ocr_, col.toInt() );
    else if (( OPTION_TRADING_TABLE == t ) && ( t_ ))
        return (*t_)[col.toInt()];
    else if (( CHARTING == t ) && ( q_ ))
    {
        const QDateTime now( AppDatabase::instance()->currentDateTime() );

        const QString symbol( q_->data0( QuoteTableModel::SYMBOL ).toString() );
        const QDate start( now.date().addDays( -5 ) );
        const QDate end( now.date() );

        // exponential moving average (from MACD)
        if (( "EMA12" == col ) || ( "EMA26" == col ))
        {
            QList<MovingAveragesConvergenceDivergence> values;

            AppDatabase::instance()->movingAveragesConvergenceDivergence( symbol, start, end, values );

            if ( values.size() )
            {
#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
                return values.last().ema[ QStringView{ col }.mid( 3 ).toInt() ];
#else
                return values.last().ema[ col.midRef( 3 ).toInt() ];
#endif
            }
        }
        // simple moving average
        // exponential moving average
        else if (( "SMA" == col.left( 3 ) ) || ( "EMA" == col.left( 3 ) ))
        {
            QList<MovingAverages> values;

            AppDatabase::instance()->movingAverages( symbol, start, end, values );

            if ( values.size() )
            {
                if ( "SMA" == col.left( 3 ) )
                {
#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
                    return values.last().sma[ QStringView{ col }.mid( 3 ).toInt() ];
#else
                    return values.last().sma[ col.midRef( 3 ).toInt() ];
#endif
                }
                else if ( "EMA" == col.left( 3 ) )
                {
#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
                    return values.last().ema[ QStringView{ col }.mid( 3 ).toInt() ];
#else
                    return values.last().ema[ col.midRef( 3 ).toInt() ];
#endif
                }
            }
        }
        // relative strength index
        else if ( "RSI" == col.left( 3 ) )
        {
            QList<RelativeStrengthIndexes> values;

            AppDatabase::instance()->relativeStrengthIndex( symbol, start, end, values );

            if ( values.size() )
            {
#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
                return values.last().values[ QStringView{ col }.mid( 3 ).toInt() ];
#else
                return values.last().values[ col.midRef( 3 ).toInt() ];
#endif
            }
        }
        // historical volatility
        else if ( "HV" == col.left( 2 ) )
        {
            QList<HistoricalVolatilities> values;

            AppDatabase::instance()->historicalVolatilities( symbol, start, end, values );

            if ( values.size() )
            {
#if QT_VERSION_CHECK( 6, 0, 0 ) <= QT_VERSION
                return values.last().volatilities[ QStringView{ col }.mid( 2 ).toInt() ];
#else
                return values.last().volatilities[ col.midRef( 2 ).toInt() ];
#endif
            }
        }
        // macd
        else if ( "MACD" == col.left( 4 ) )
        {
            QList<MovingAveragesConvergenceDivergence> values;

            AppDatabase::instance()->movingAveragesConvergenceDivergence( symbol, start, end, values );

            if (( "MACDBUYFLAG" == col ) && ( 2 <= values.size() ))
            {
                const double pval( values[values.size()-2].histogram );
                const double val( values[values.size()-1].histogram );

                return ((( pval < 0.0 ) && ( 0.0 <= val )) ? 1 : 0);
            }
            else if (( "MACDSELLFLAG" == col ) && ( 2 <= values.size() ))
            {
                const double pval( values[values.size()-2].histogram );
                const double val( values[values.size()-1].histogram );

                return ((( 0.0 <= pval ) && ( val < 0.0 )) ? 1 : 0);
            }
            else if ( values.size() )
            {
                if ( "MACD" == col )
                    return values.last().macd;
                else if ( "MACDSIG" == col )
                    return values.last().signal;
                else if ( "MACDH" == col )
                    return values.last().histogram;
            }
        }
    }

    return QVariant();
}

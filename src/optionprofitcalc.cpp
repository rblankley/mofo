/**
 * @file optionprofitcalc.cpp
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
#include "optionprofitcalc.h"

#include "calc/binomialcalc.h"
#include "calc/blackscholescalc.h"
#include "calc/epbinomialcalc.h"
#include "calc/montecarlocalc.h"
#include "calc/trinomialcalc.h"

#include "db/appdb.h"
#include "db/optionchaintablemodel.h"

#include <cmath>

#include <QObject>

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculator::OptionProfitCalculator( double underlying, const table_model_type *chains, item_model_type *results ) :
    valid_( true ),
    underlying_( underlying ),
    totalDivAmount_( 0.0 ),
    totalDivYield_( 0.0 ),
    chains_( chains ),
    results_( results ),
    costBasis_( 0.0 ),
    equityTradeCost_( 0.0 ),
    optionTradeCost_( 0.0 )
{
    const QDateTime now( AppDatabase::instance()->currentDateTime() );

    // validate underlying price
    if ( underlying_ <= 0.0 )
        valid_ = false;

    daysToExpiry_ = calcDaysToExpiry();

    // ignore expired options
    if ( daysToExpiry_ < 0 )
        valid_ = false;

    double timeToExpiryYears = daysToExpiry_;
    timeToExpiryYears /= AppDatabase::instance()->numDays();

    // calculate dividends
    QDate divDate;
    double divFreq;

    const double divAmount = AppDatabase::instance()->dividendAmount( chains_->symbol(), divDate, divFreq );
    const double divYield = AppDatabase::instance()->dividendYield( chains_->symbol() );

    if (( divDate.isValid() ) && ( 0.0 < divFreq ) && ( 0.0 < divYield ))
    {
        double timeToDivYears = now.date().daysTo( divDate );
        timeToDivYears /= AppDatabase::instance()->numDays();

        // make dividend payment in the future
        if ( timeToDivYears < 0.0 )
            timeToDivYears += divFreq;

        if ( 0.0 <= timeToDivYears )
        {
            // make list of dividend payment dates and yields
            while ( timeToDivYears < timeToExpiryYears )
            {
                const double yield( divYield * divFreq );

                divTimes_.push_back( timeToDivYears );
                div_.push_back( yield );

                // accumulate dividend
                totalDivAmount_ += (divAmount * divFreq);
                totalDivYield_ += yield;

                // next dividend
                timeToDivYears += divFreq;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculator::~OptionProfitCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculator *OptionProfitCalculator::create( double underlying, const table_model_type *chains, item_model_type *results )
{
    const QString method( AppDatabase::instance()->optionCalcMethod() );

    if ( "BINOM" == method )
        return new BinomialCalculator( underlying, chains, results );
    else if ( "BINOM_EQPROB" == method )
        return new EqualProbBinomialCalculator( underlying, chains, results );
    else if ( "BLACKSCHOLES" == method )
        return new BlackScholesCalculator( underlying, chains, results );
/*
    // do not use
    // not consistent enough to calculate implied volatility reliably
    else if ( "MONTECARLO" == method )
        return new MonteCarloCalculator( underlying, chains, results );
*/
    else if ( "TRINOM" == method )
        return new TrinomialCalculator( underlying, chains, results );

    LOG_WARN << "unhandled option calc method " << qPrintable( method );

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionProfitCalculator::destroy( _Myt *calc )
{
    if ( calc )
        delete calc;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
double OptionProfitCalculator::calcDaysToExpiry() const
{
    const QDateTime now( AppDatabase::instance()->currentDateTime() );

    return now.date().daysTo( chains_->expirationDate() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionProfitCalculator::isFilteredOut( int row, bool isCall ) const
{
    if ( isNonStandard( row ) )
        return true;

    if ( isCall )
    {
        const bool itm( OptionProfitCalculatorFilter::ITM_CALLS & f_.optionTypeFilter() );
        const bool otm( OptionProfitCalculatorFilter::OTM_CALLS & f_.optionTypeFilter() );

        if ((( chains_->tableData( row, table_model_type::CALL_IS_IN_THE_MONEY ).toBool() ) && ( !itm )) ||
            (( !chains_->tableData( row, table_model_type::CALL_IS_IN_THE_MONEY ).toBool() ) && ( !otm )))
            return true;
    }
    else
    {
        const bool itm( OptionProfitCalculatorFilter::ITM_PUTS & f_.optionTypeFilter() );
        const bool otm( OptionProfitCalculatorFilter::OTM_PUTS & f_.optionTypeFilter() );

        if ((( chains_->tableData( row, table_model_type::PUT_IS_IN_THE_MONEY ).toBool() ) && ( !itm )) ||
            (( !chains_->tableData( row, table_model_type::PUT_IS_IN_THE_MONEY ).toBool() ) && ( !otm )))
            return true;
    }

    if (( f_.minDaysToExpiry() ) && ( daysToExpiry_ < f_.minDaysToExpiry() ))
        return true;
    else if (( f_.maxDaysToExpiry() ) && ( f_.maxDaysToExpiry() < daysToExpiry_ ))
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool OptionProfitCalculator::isNonStandard( int row ) const
{
    return (( chains_->tableData( row, table_model_type::CALL_IS_NON_STANDARD ).toBool() ) ||
            ( chains_->tableData( row, table_model_type::PUT_IS_NON_STANDARD ).toBool() ));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionProfitCalculator::addRowToItemModel( const item_model_type::ColumnValueMap& result ) const
{
    // negative investments have zero risk!
    // either the VI is really high or option data is out of date
    const bool freeMoney( result[item_model_type::INVESTMENT_AMOUNT].toDouble() <= 0.0 );

    // investment amount in range
    if (( 0.0 < f_.minInvestAmount() ) && ( result[item_model_type::INVESTMENT_AMOUNT].toDouble() < f_.minInvestAmount() ) && ( !freeMoney ))
        return;
    if (( 0.0 < f_.maxInvestAmount() ) && ( f_.maxInvestAmount() < result[item_model_type::INVESTMENT_AMOUNT].toDouble() ))
        return;

    // check max loss and min gain
    if (( 0.0 < f_.maxLossAmount() ) && ( f_.maxLossAmount() < result[item_model_type::MAX_LOSS].toDouble() ) && ( !freeMoney ))
        return;
    if (( 0.0 < f_.minGainAmount() ) && ( result[item_model_type::MAX_GAIN].toDouble() < f_.minGainAmount() ))
        return;

    // check bid sizes
    if (( 0 < f_.minBidSize() ) && ( result[item_model_type::BID_SIZE].toInt() < f_.minBidSize() ))
        return;
    if (( 0 < f_.minAskSize() ) && ( result[item_model_type::ASK_SIZE].toInt() < f_.minAskSize() ))
        return;

    // probability of profit
    if (( 0.0 < f_.minProbProfit() ) && ( result[item_model_type::PROBABILITY_PROFIT].toDouble() < f_.minProbProfit() ))
        return;
    else if (( 0.0 < f_.maxProbProfit() ) && ( f_.maxProbProfit() < result[item_model_type::PROBABILITY_PROFIT].toDouble() ))
        return;

    // DTE
    if (( f_.minDaysToExpiry() ) && ( daysToExpiry_ < f_.minDaysToExpiry() ))
        return;
    else if (( f_.maxDaysToExpiry() ) && ( f_.maxDaysToExpiry() < daysToExpiry_ ))
        return;

    // ROI
    if (( 0.0 < f_.minReturnOnInvestment() ) && ( result[item_model_type::ROI].toDouble() < f_.minReturnOnInvestment() ) && ( !freeMoney ))
        return;
    if (( 0.0 < f_.maxReturnOnInvestment() ) && ( f_.maxReturnOnInvestment() < result[item_model_type::ROI].toDouble() ))
        return;

    // ROI / Time
    if (( 0.0 < f_.minReturnOnInvestmentTime() ) && ( result[item_model_type::ROI_TIME].toDouble() < f_.minReturnOnInvestmentTime() ) && ( !freeMoney ))
        return;
    if (( 0.0 < f_.maxReturnOnInvestmentTime() ) && ( f_.maxReturnOnInvestmentTime() < result[item_model_type::ROI_TIME].toDouble() ))
        return;

    if ( 0.0 < f_.maxSpreadPercent() )
    {
        // spread percent is populated
        if ( result[item_model_type::BID_ASK_SPREAD_PERCENT].isNull() )
            return;

        const double spreadPercent( result[item_model_type::BID_ASK_SPREAD_PERCENT].toDouble() );

        // spread percent is valid
        if (( std::isinf( spreadPercent ) ) || ( std::isnan( spreadPercent ) ))
            return;

        if ( f_.maxSpreadPercent() < spreadPercent )
            return;
    }

    if (( 0.0 < f_.minVolatility() ) && ( result[item_model_type::CALC_THEO_VOLATILITY].toDouble() < f_.minVolatility() ))
        return;
    if (( 0.0 < f_.maxVolatility() ) && ( f_.maxVolatility() < result[item_model_type::CALC_THEO_VOLATILITY].toDouble() ))
        return;

    const bool itm( (OptionProfitCalculatorFilter::ITM_CALLS | OptionProfitCalculatorFilter::ITM_PUTS) & f_.optionTypeFilter() );
    const bool otm( (OptionProfitCalculatorFilter::OTM_CALLS | OptionProfitCalculatorFilter::OTM_PUTS) & f_.optionTypeFilter() );

    if (( result[item_model_type::IS_IN_THE_MONEY].toBool() ) && ( !itm ))
        return;
    else if (( !result[item_model_type::IS_IN_THE_MONEY].toBool() ) && ( !otm ))
        return;

    const bool histLowerThanImplied( OptionProfitCalculatorFilter::HV_LTE_VI & f_.volatilityFilter() );
    const bool histHigherThanImplied( OptionProfitCalculatorFilter::HV_GT_VI & f_.volatilityFilter() );

    if (( result[item_model_type::HIST_VOLATILITY].toDouble() <= result[item_model_type::CALC_THEO_VOLATILITY].toDouble() ) && ( !histLowerThanImplied ))
        return;
    else if (( result[item_model_type::CALC_THEO_VOLATILITY].toDouble() < result[item_model_type::HIST_VOLATILITY].toDouble() ) && ( !histHigherThanImplied ))
        return;

    // add
    results_->addRow( result );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionProfitCalculator::populateResultModelSingle( int row, bool isCall, item_model_type::ColumnValueMap& result ) const
{
    result[item_model_type::STAMP] = AppDatabase::instance()->currentDateTime();
    result[item_model_type::UNDERLYING] = chains_->symbol();

    if ( isCall )
    {
        result[item_model_type::TYPE] = QObject::tr( "Call" );

        // Option Chain Information
        result[item_model_type::SYMBOL] = chains_->tableData( row, table_model_type::CALL_SYMBOL );
        result[item_model_type::DESC] = chains_->tableData( row, table_model_type::CALL_DESC );
        result[item_model_type::BID_ASK_SIZE] = chains_->tableData( row, table_model_type::CALL_BID_ASK_SIZE );
        result[item_model_type::BID_PRICE] = chains_->tableData( row, table_model_type::CALL_BID_PRICE );
        result[item_model_type::BID_SIZE] = chains_->tableData( row, table_model_type::CALL_BID_SIZE );
        result[item_model_type::ASK_PRICE] = chains_->tableData( row, table_model_type::CALL_ASK_PRICE );
        result[item_model_type::ASK_SIZE] = chains_->tableData( row, table_model_type::CALL_ASK_SIZE );
        result[item_model_type::LAST_PRICE] = chains_->tableData( row, table_model_type::CALL_LAST_PRICE );
        result[item_model_type::LAST_SIZE] = chains_->tableData( row, table_model_type::CALL_LAST_SIZE );
        result[item_model_type::BREAK_EVEN_PRICE] = chains_->tableData( row, table_model_type::CALL_BREAK_EVEN_PRICE );
        result[item_model_type::INTRINSIC_VALUE] = chains_->tableData( row, table_model_type::CALL_INTRINSIC_VALUE );
        result[item_model_type::OPEN_PRICE] = chains_->tableData( row, table_model_type::CALL_OPEN_PRICE );
        result[item_model_type::HIGH_PRICE] = chains_->tableData( row, table_model_type::CALL_HIGH_PRICE );
        result[item_model_type::LOW_PRICE] = chains_->tableData( row, table_model_type::CALL_LOW_PRICE );
        result[item_model_type::CLOSE_PRICE] = chains_->tableData( row, table_model_type::CALL_CLOSE_PRICE );
        result[item_model_type::CHANGE] = chains_->tableData( row, table_model_type::CALL_CHANGE );
        result[item_model_type::PERCENT_CHANGE] = chains_->tableData( row, table_model_type::CALL_PERCENT_CHANGE );
        result[item_model_type::TOTAL_VOLUME] = chains_->tableData( row, table_model_type::CALL_TOTAL_VOLUME );
        result[item_model_type::QUOTE_TIME] = chains_->tableData( row, table_model_type::CALL_QUOTE_TIME );
        result[item_model_type::TRADE_TIME] = chains_->tableData( row, table_model_type::CALL_TRADE_TIME );
        result[item_model_type::MARK] = chains_->tableData( row, table_model_type::CALL_MARK );
        result[item_model_type::MARK_CHANGE] = chains_->tableData( row, table_model_type::CALL_MARK_CHANGE );
        result[item_model_type::MARK_PERCENT_CHANGE] = chains_->tableData( row, table_model_type::CALL_MARK_PERCENT_CHANGE );
        result[item_model_type::EXCHANGE_NAME] = chains_->tableData( row, table_model_type::CALL_EXCHANGE_NAME );
        result[item_model_type::VOLATILITY] = chains_->tableData( row, table_model_type::CALL_VOLATILITY );
        result[item_model_type::DELTA] = chains_->tableData( row, table_model_type::CALL_DELTA );
        result[item_model_type::GAMMA] = chains_->tableData( row, table_model_type::CALL_GAMMA );
        result[item_model_type::THETA] = chains_->tableData( row, table_model_type::CALL_THETA );
        result[item_model_type::VEGA] = chains_->tableData( row, table_model_type::CALL_VEGA );
        result[item_model_type::RHO] = chains_->tableData( row, table_model_type::CALL_RHO );
        result[item_model_type::TIME_VALUE] = chains_->tableData( row, table_model_type::CALL_TIME_VALUE );
        result[item_model_type::OPEN_INTEREST] = chains_->tableData( row, table_model_type::CALL_OPEN_INTEREST );
        result[item_model_type::IS_IN_THE_MONEY] = chains_->tableData( row, table_model_type::CALL_IS_IN_THE_MONEY );
        result[item_model_type::IS_OUT_OF_THE_MONEY] = !result[item_model_type::IS_IN_THE_MONEY].toBool();
        result[item_model_type::THEO_OPTION_VALUE] = chains_->tableData( row, table_model_type::CALL_THEO_OPTION_VALUE );
        result[item_model_type::THEO_VOLATILITY] = chains_->tableData( row, table_model_type::CALL_THEO_VOLATILITY );
        result[item_model_type::IS_MINI] = chains_->tableData( row, table_model_type::CALL_IS_MINI );
        result[item_model_type::IS_NON_STANDARD] = chains_->tableData( row, table_model_type::CALL_IS_NON_STANDARD );
        result[item_model_type::IS_INDEX] = chains_->tableData( row, table_model_type::CALL_IS_INDEX );
        result[item_model_type::IS_WEEKLY] = chains_->tableData( row, table_model_type::CALL_IS_WEEKLY );
        result[item_model_type::IS_QUARTERLY] = chains_->tableData( row, table_model_type::CALL_IS_QUARTERLY );
        result[item_model_type::EXPIRY_DATE] = chains_->tableData( row, table_model_type::CALL_EXPIRY_DATE );
        result[item_model_type::EXPIRY_TYPE] = chains_->tableData( row, table_model_type::CALL_EXPIRY_TYPE );
        result[item_model_type::DAYS_TO_EXPIRY] = chains_->tableData( row, table_model_type::CALL_DAYS_TO_EXPIRY );
        result[item_model_type::LAST_TRADING_DAY] = chains_->tableData( row, table_model_type::CALL_LAST_TRADING_DAY );
        result[item_model_type::MULTIPLIER] = chains_->tableData( row, table_model_type::CALL_MULTIPLIER );
        result[item_model_type::SETTLEMENT_TYPE] = chains_->tableData( row, table_model_type::CALL_SETTLEMENT_TYPE );
        result[item_model_type::DELIVERABLE_NOTE] = chains_->tableData( row, table_model_type::CALL_DELIVERABLE_NOTE );
    }
    else
    {
        result[item_model_type::TYPE] = QObject::tr( "Put" );

        // Option Chain Information
        result[item_model_type::SYMBOL] = chains_->tableData( row, table_model_type::PUT_SYMBOL );
        result[item_model_type::DESC] = chains_->tableData( row, table_model_type::PUT_DESC );
        result[item_model_type::BID_ASK_SIZE] = chains_->tableData( row, table_model_type::PUT_BID_ASK_SIZE );
        result[item_model_type::BID_PRICE] = chains_->tableData( row, table_model_type::PUT_BID_PRICE );
        result[item_model_type::BID_SIZE] = chains_->tableData( row, table_model_type::PUT_BID_SIZE );
        result[item_model_type::ASK_PRICE] = chains_->tableData( row, table_model_type::PUT_ASK_PRICE );
        result[item_model_type::ASK_SIZE] = chains_->tableData( row, table_model_type::PUT_ASK_SIZE );
        result[item_model_type::LAST_PRICE] = chains_->tableData( row, table_model_type::PUT_LAST_PRICE );
        result[item_model_type::LAST_SIZE] = chains_->tableData( row, table_model_type::PUT_LAST_SIZE );
        result[item_model_type::BREAK_EVEN_PRICE] = chains_->tableData( row, table_model_type::PUT_BREAK_EVEN_PRICE );
        result[item_model_type::INTRINSIC_VALUE] = chains_->tableData( row, table_model_type::PUT_INTRINSIC_VALUE );
        result[item_model_type::OPEN_PRICE] = chains_->tableData( row, table_model_type::PUT_OPEN_PRICE );
        result[item_model_type::HIGH_PRICE] = chains_->tableData( row, table_model_type::PUT_HIGH_PRICE );
        result[item_model_type::LOW_PRICE] = chains_->tableData( row, table_model_type::PUT_LOW_PRICE );
        result[item_model_type::CLOSE_PRICE] = chains_->tableData( row, table_model_type::PUT_CLOSE_PRICE );
        result[item_model_type::CHANGE] = chains_->tableData( row, table_model_type::PUT_CHANGE );
        result[item_model_type::PERCENT_CHANGE] = chains_->tableData( row, table_model_type::PUT_PERCENT_CHANGE );
        result[item_model_type::TOTAL_VOLUME] = chains_->tableData( row, table_model_type::PUT_TOTAL_VOLUME );
        result[item_model_type::QUOTE_TIME] = chains_->tableData( row, table_model_type::PUT_QUOTE_TIME );
        result[item_model_type::TRADE_TIME] = chains_->tableData( row, table_model_type::PUT_TRADE_TIME );
        result[item_model_type::MARK] = chains_->tableData( row, table_model_type::PUT_MARK );
        result[item_model_type::MARK_CHANGE] = chains_->tableData( row, table_model_type::PUT_MARK_CHANGE );
        result[item_model_type::MARK_PERCENT_CHANGE] = chains_->tableData( row, table_model_type::PUT_MARK_PERCENT_CHANGE );
        result[item_model_type::EXCHANGE_NAME] = chains_->tableData( row, table_model_type::PUT_EXCHANGE_NAME );
        result[item_model_type::VOLATILITY] = chains_->tableData( row, table_model_type::PUT_VOLATILITY );
        result[item_model_type::DELTA] = chains_->tableData( row, table_model_type::PUT_DELTA );
        result[item_model_type::GAMMA] = chains_->tableData( row, table_model_type::PUT_GAMMA );
        result[item_model_type::THETA] = chains_->tableData( row, table_model_type::PUT_THETA );
        result[item_model_type::VEGA] = chains_->tableData( row, table_model_type::PUT_VEGA );
        result[item_model_type::RHO] = chains_->tableData( row, table_model_type::PUT_RHO );
        result[item_model_type::TIME_VALUE] = chains_->tableData( row, table_model_type::PUT_TIME_VALUE );
        result[item_model_type::OPEN_INTEREST] = chains_->tableData( row, table_model_type::PUT_OPEN_INTEREST );
        result[item_model_type::IS_IN_THE_MONEY] = chains_->tableData( row, table_model_type::PUT_IS_IN_THE_MONEY );
        result[item_model_type::IS_OUT_OF_THE_MONEY] = !result[item_model_type::IS_IN_THE_MONEY].toBool();
        result[item_model_type::THEO_OPTION_VALUE] = chains_->tableData( row, table_model_type::PUT_THEO_OPTION_VALUE );
        result[item_model_type::THEO_VOLATILITY] = chains_->tableData( row, table_model_type::PUT_THEO_VOLATILITY );
        result[item_model_type::IS_MINI] = chains_->tableData( row, table_model_type::PUT_IS_MINI );
        result[item_model_type::IS_NON_STANDARD] = chains_->tableData( row, table_model_type::PUT_IS_NON_STANDARD );
        result[item_model_type::IS_INDEX] = chains_->tableData( row, table_model_type::PUT_IS_INDEX );
        result[item_model_type::IS_WEEKLY] = chains_->tableData( row, table_model_type::PUT_IS_WEEKLY );
        result[item_model_type::IS_QUARTERLY] = chains_->tableData( row, table_model_type::PUT_IS_QUARTERLY );
        result[item_model_type::EXPIRY_DATE] = chains_->tableData( row, table_model_type::PUT_EXPIRY_DATE );
        result[item_model_type::EXPIRY_TYPE] = chains_->tableData( row, table_model_type::PUT_EXPIRY_TYPE );
        result[item_model_type::DAYS_TO_EXPIRY] = chains_->tableData( row, table_model_type::PUT_DAYS_TO_EXPIRY );
        result[item_model_type::LAST_TRADING_DAY] = chains_->tableData( row, table_model_type::PUT_LAST_TRADING_DAY );
        result[item_model_type::MULTIPLIER] = chains_->tableData( row, table_model_type::PUT_MULTIPLIER );
        result[item_model_type::SETTLEMENT_TYPE] = chains_->tableData( row, table_model_type::PUT_SETTLEMENT_TYPE );
        result[item_model_type::DELIVERABLE_NOTE] = chains_->tableData( row, table_model_type::PUT_DELIVERABLE_NOTE );
    }

    result[item_model_type::STRIKE_PRICE] = chains_->tableData( row, table_model_type::STRIKE_PRICE );

    // determine historical volatility
    const QDateTime quoteTime( result[item_model_type::QUOTE_TIME].toDateTime() );

    int tradingDaysToExpiry( daysToExpiry_ );
    tradingDaysToExpiry *= AppDatabase::instance()->numTradingDays();
    tradingDaysToExpiry /= AppDatabase::instance()->numDays();

    result[item_model_type::HIST_VOLATILITY] = 100.0 * AppDatabase::instance()->historicalVolatility( chains_->symbol(), quoteTime, tradingDaysToExpiry );

    // expected dividend
    result[item_model_type::DIV_AMOUNT] = totalDivAmount_;
    result[item_model_type::DIV_YIELD] = 100.0 * totalDivYield_;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionProfitCalculator::populateResultModelVertical( int rowLong, int rowShort, bool isCall, item_model_type::ColumnValueMap& result ) const
{
    result[item_model_type::STAMP] = AppDatabase::instance()->currentDateTime();
    result[item_model_type::UNDERLYING] = chains_->symbol();

    if ( isCall )
    {
        result[item_model_type::TYPE] = QObject::tr( "Call Spread" );

        // Option Chain Information
        result[item_model_type::SYMBOL] = chains_->tableData( rowShort, table_model_type::CALL_SYMBOL ).toString() + "-" + chains_->tableData( rowLong, table_model_type::CALL_SYMBOL ).toString();
        result[item_model_type::DESC] = chains_->tableData( rowShort, table_model_type::CALL_DESC ).toString() + "-" + chains_->tableData( rowLong, table_model_type::CALL_DESC ).toString();

        result[item_model_type::BID_PRICE] = chains_->tableData( rowShort, table_model_type::CALL_BID_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_ASK_PRICE ).toDouble();
        result[item_model_type::BID_SIZE] = qMin( chains_->tableData( rowShort, table_model_type::CALL_BID_SIZE ).toInt(), chains_->tableData( rowLong, table_model_type::CALL_BID_SIZE ).toInt() );
        result[item_model_type::ASK_PRICE] = chains_->tableData( rowShort, table_model_type::CALL_ASK_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_BID_PRICE ).toDouble();
        result[item_model_type::ASK_SIZE] = qMin( chains_->tableData( rowShort, table_model_type::CALL_ASK_SIZE ).toInt(), chains_->tableData( rowLong, table_model_type::CALL_ASK_SIZE ).toInt() );
        result[item_model_type::BID_ASK_SIZE] = result[item_model_type::BID_SIZE].toString() + " x " + result[item_model_type::ASK_SIZE].toString();

        result[item_model_type::LAST_PRICE] = chains_->tableData( rowShort, table_model_type::CALL_LAST_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_LAST_PRICE ).toDouble();
        result[item_model_type::LAST_SIZE] = qMin( chains_->tableData( rowShort, table_model_type::CALL_LAST_SIZE ).toInt(), chains_->tableData( rowLong, table_model_type::CALL_LAST_SIZE ).toInt() );

        double breakEvenPrice = chains_->tableData( rowShort, table_model_type::CALL_MULTIPLIER ).toDouble() * (chains_->tableData( rowShort, table_model_type::CALL_MARK ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_MARK ).toDouble());
        breakEvenPrice -= 2.0 * AppDatabase::instance()->optionTradeCost();
        breakEvenPrice /= chains_->tableData( rowShort, table_model_type::CALL_MULTIPLIER ).toDouble();
        breakEvenPrice = chains_->tableData( rowShort, table_model_type::STRIKE_PRICE ).toDouble() + breakEvenPrice;

        result[item_model_type::BREAK_EVEN_PRICE] = breakEvenPrice;
        result[item_model_type::INTRINSIC_VALUE] = underlying_ - breakEvenPrice;

        result[item_model_type::OPEN_PRICE] = chains_->tableData( rowShort, table_model_type::CALL_OPEN_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_OPEN_PRICE ).toDouble();
        result[item_model_type::HIGH_PRICE] = chains_->tableData( rowShort, table_model_type::CALL_HIGH_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_LOW_PRICE ).toDouble();
        result[item_model_type::LOW_PRICE] = chains_->tableData( rowShort, table_model_type::CALL_LOW_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_HIGH_PRICE ).toDouble();
        result[item_model_type::CLOSE_PRICE] = chains_->tableData( rowShort, table_model_type::CALL_CLOSE_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_CLOSE_PRICE ).toDouble();
        result[item_model_type::CHANGE] = chains_->tableData( rowShort, table_model_type::CALL_CHANGE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_CHANGE ).toDouble();
        result[item_model_type::PERCENT_CHANGE] = chains_->tableData( rowShort, table_model_type::CALL_PERCENT_CHANGE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_PERCENT_CHANGE ).toDouble();

        result[item_model_type::TOTAL_VOLUME] = qMin( chains_->tableData( rowShort, table_model_type::CALL_TOTAL_VOLUME ).toInt(), chains_->tableData( rowLong, table_model_type::CALL_TOTAL_VOLUME ).toInt() );
        result[item_model_type::QUOTE_TIME] = qMin( chains_->tableData( rowShort, table_model_type::CALL_QUOTE_TIME ).toDateTime(), chains_->tableData( rowLong, table_model_type::CALL_QUOTE_TIME ).toDateTime() );
        result[item_model_type::TRADE_TIME] = qMin( chains_->tableData( rowShort, table_model_type::CALL_TRADE_TIME ).toDateTime(), chains_->tableData( rowLong, table_model_type::CALL_TRADE_TIME ).toDateTime() );

        result[item_model_type::MARK] = chains_->tableData( rowShort, table_model_type::CALL_MARK ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_MARK ).toDouble();
        result[item_model_type::MARK_CHANGE] = chains_->tableData( rowShort, table_model_type::CALL_MARK_CHANGE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_MARK_CHANGE ).toDouble();
        result[item_model_type::MARK_PERCENT_CHANGE] = chains_->tableData( rowShort, table_model_type::CALL_MARK_PERCENT_CHANGE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_MARK_PERCENT_CHANGE ).toDouble();
        result[item_model_type::EXCHANGE_NAME] = chains_->tableData( rowShort, table_model_type::CALL_EXCHANGE_NAME );

        // net volatility
        // https://en.wikipedia.org/wiki/Net_volatility#:~:text=Net%20volatility%20refers%20to%20the,implied%20volatility%20of%20the%20spread.
        double volNet = chains_->tableData( rowLong, table_model_type::CALL_VEGA ).toDouble() * chains_->tableData( rowLong, table_model_type::CALL_VOLATILITY ).toDouble();
        volNet -= chains_->tableData( rowShort, table_model_type::CALL_VEGA ).toDouble() * chains_->tableData( rowShort, table_model_type::CALL_VOLATILITY ).toDouble();
        volNet /= chains_->tableData( rowLong, table_model_type::CALL_VEGA ).toDouble() - chains_->tableData( rowShort, table_model_type::CALL_VEGA ).toDouble();

        result[item_model_type::VOLATILITY] = volNet;
        result[item_model_type::DELTA] = chains_->tableData( rowLong, table_model_type::CALL_DELTA ).toDouble() - chains_->tableData( rowShort, table_model_type::CALL_DELTA ).toDouble();
        result[item_model_type::GAMMA] = chains_->tableData( rowLong, table_model_type::CALL_GAMMA ).toDouble() - chains_->tableData( rowShort, table_model_type::CALL_GAMMA ).toDouble();
        result[item_model_type::THETA] = chains_->tableData( rowLong, table_model_type::CALL_THETA ).toDouble() - chains_->tableData( rowShort, table_model_type::CALL_THETA ).toDouble();
        result[item_model_type::VEGA] = chains_->tableData( rowLong, table_model_type::CALL_VEGA ).toDouble() - chains_->tableData( rowShort, table_model_type::CALL_VEGA ).toDouble();
        result[item_model_type::RHO] = chains_->tableData( rowLong, table_model_type::CALL_RHO ).toDouble() - chains_->tableData( rowShort, table_model_type::CALL_RHO ).toDouble();

        result[item_model_type::TIME_VALUE] = chains_->tableData( rowShort, table_model_type::CALL_TIME_VALUE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_TIME_VALUE ).toDouble();
        result[item_model_type::OPEN_INTEREST] = qMin( chains_->tableData( rowShort, table_model_type::CALL_OPEN_INTEREST ).toInt(), chains_->tableData( rowLong, table_model_type::CALL_OPEN_INTEREST ).toInt() );
        result[item_model_type::IS_IN_THE_MONEY] = (chains_->tableData( rowShort, table_model_type::CALL_IS_IN_THE_MONEY ).toBool() || chains_->tableData( rowLong, table_model_type::CALL_IS_IN_THE_MONEY ).toBool());
        result[item_model_type::IS_OUT_OF_THE_MONEY] = !(chains_->tableData( rowShort, table_model_type::CALL_IS_IN_THE_MONEY ).toBool() && chains_->tableData( rowLong, table_model_type::CALL_IS_IN_THE_MONEY ).toBool());

        // net volatility
        // https://en.wikipedia.org/wiki/Net_volatility#:~:text=Net%20volatility%20refers%20to%20the,implied%20volatility%20of%20the%20spread.
        double theoVolNet = chains_->tableData( rowLong, table_model_type::CALL_VEGA ).toDouble() * chains_->tableData( rowLong, table_model_type::CALL_THEO_VOLATILITY ).toDouble();
        theoVolNet -= chains_->tableData( rowShort, table_model_type::CALL_VEGA ).toDouble() * chains_->tableData( rowShort, table_model_type::CALL_THEO_VOLATILITY ).toDouble();
        theoVolNet /= chains_->tableData( rowLong, table_model_type::CALL_VEGA ).toDouble() - chains_->tableData( rowShort, table_model_type::CALL_VEGA ).toDouble();

        result[item_model_type::THEO_OPTION_VALUE] = chains_->tableData( rowShort, table_model_type::CALL_THEO_OPTION_VALUE ).toDouble() - chains_->tableData( rowLong, table_model_type::CALL_THEO_OPTION_VALUE ).toDouble();
        result[item_model_type::THEO_VOLATILITY] = theoVolNet;

        result[item_model_type::IS_MINI] = (chains_->tableData( rowShort, table_model_type::CALL_IS_MINI ).toBool() || chains_->tableData( rowLong, table_model_type::CALL_IS_MINI ).toBool());
        result[item_model_type::IS_NON_STANDARD] = (chains_->tableData( rowShort, table_model_type::CALL_IS_NON_STANDARD ).toBool() || chains_->tableData( rowLong, table_model_type::CALL_IS_NON_STANDARD ).toBool());
        result[item_model_type::IS_INDEX] = (chains_->tableData( rowShort, table_model_type::CALL_IS_INDEX ).toBool() || chains_->tableData( rowLong, table_model_type::CALL_IS_INDEX ).toBool());
        result[item_model_type::IS_WEEKLY] = (chains_->tableData( rowShort, table_model_type::CALL_IS_WEEKLY ).toBool() || chains_->tableData( rowLong, table_model_type::CALL_IS_WEEKLY ).toBool());
        result[item_model_type::IS_QUARTERLY] = (chains_->tableData( rowShort, table_model_type::CALL_IS_QUARTERLY ).toBool() || chains_->tableData( rowLong, table_model_type::CALL_IS_QUARTERLY ).toBool());
        result[item_model_type::EXPIRY_DATE] = chains_->tableData( rowShort, table_model_type::CALL_EXPIRY_DATE );
        result[item_model_type::EXPIRY_TYPE] = chains_->tableData( rowShort, table_model_type::CALL_EXPIRY_TYPE );
        result[item_model_type::DAYS_TO_EXPIRY] = chains_->tableData( rowShort, table_model_type::CALL_DAYS_TO_EXPIRY );
        result[item_model_type::LAST_TRADING_DAY] = chains_->tableData( rowShort, table_model_type::CALL_LAST_TRADING_DAY );
        result[item_model_type::MULTIPLIER] = chains_->tableData( rowShort, table_model_type::CALL_MULTIPLIER );
        result[item_model_type::SETTLEMENT_TYPE] = chains_->tableData( rowShort, table_model_type::CALL_SETTLEMENT_TYPE );
        result[item_model_type::DELIVERABLE_NOTE] = chains_->tableData( rowShort, table_model_type::CALL_DELIVERABLE_NOTE );
    }
    else
    {
        result[item_model_type::TYPE] = QObject::tr( "Put Spread" );

        // Option Chain Information
        result[item_model_type::SYMBOL] = chains_->tableData( rowShort, table_model_type::PUT_SYMBOL ).toString() + "-" + chains_->tableData( rowLong, table_model_type::PUT_SYMBOL ).toString();
        result[item_model_type::DESC] = chains_->tableData( rowShort, table_model_type::PUT_DESC ).toString() + "-" + chains_->tableData( rowLong, table_model_type::PUT_DESC ).toString();

        result[item_model_type::BID_PRICE] = chains_->tableData( rowShort, table_model_type::PUT_BID_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_ASK_PRICE ).toDouble();
        result[item_model_type::BID_SIZE] = qMin( chains_->tableData( rowShort, table_model_type::PUT_BID_SIZE ).toInt(), chains_->tableData( rowLong, table_model_type::PUT_BID_SIZE ).toInt() );
        result[item_model_type::ASK_PRICE] = chains_->tableData( rowShort, table_model_type::PUT_ASK_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_BID_PRICE ).toDouble();
        result[item_model_type::ASK_SIZE] = qMin( chains_->tableData( rowShort, table_model_type::PUT_ASK_SIZE ).toInt(), chains_->tableData( rowLong, table_model_type::PUT_ASK_SIZE ).toInt() );
        result[item_model_type::BID_ASK_SIZE] = result[item_model_type::BID_SIZE].toString() + " x " + result[item_model_type::ASK_SIZE].toString();

        result[item_model_type::LAST_PRICE] = chains_->tableData( rowShort, table_model_type::PUT_LAST_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_LAST_PRICE ).toDouble();
        result[item_model_type::LAST_SIZE] = qMin( chains_->tableData( rowShort, table_model_type::PUT_LAST_SIZE ).toInt(), chains_->tableData( rowLong, table_model_type::PUT_LAST_SIZE ).toInt() );

        double breakEvenPrice = chains_->tableData( rowShort, table_model_type::PUT_MULTIPLIER ).toDouble() * (chains_->tableData( rowShort, table_model_type::PUT_MARK ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_MARK ).toDouble());
        breakEvenPrice -= 2.0 * AppDatabase::instance()->optionTradeCost();
        breakEvenPrice /= chains_->tableData( rowShort, table_model_type::PUT_MULTIPLIER ).toDouble();
        breakEvenPrice = chains_->tableData( rowShort, table_model_type::STRIKE_PRICE ).toDouble() - breakEvenPrice;

        result[item_model_type::BREAK_EVEN_PRICE] = breakEvenPrice;
        result[item_model_type::INTRINSIC_VALUE] = breakEvenPrice - underlying_;

        result[item_model_type::OPEN_PRICE] = chains_->tableData( rowShort, table_model_type::PUT_OPEN_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_OPEN_PRICE ).toDouble();
        result[item_model_type::HIGH_PRICE] = chains_->tableData( rowShort, table_model_type::PUT_HIGH_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_LOW_PRICE ).toDouble();
        result[item_model_type::LOW_PRICE] = chains_->tableData( rowShort, table_model_type::PUT_LOW_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_HIGH_PRICE ).toDouble();
        result[item_model_type::CLOSE_PRICE] = chains_->tableData( rowShort, table_model_type::PUT_CLOSE_PRICE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_CLOSE_PRICE ).toDouble();
        result[item_model_type::CHANGE] = chains_->tableData( rowShort, table_model_type::PUT_CHANGE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_CHANGE ).toDouble();
        result[item_model_type::PERCENT_CHANGE] = chains_->tableData( rowShort, table_model_type::PUT_PERCENT_CHANGE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_PERCENT_CHANGE ).toDouble();

        result[item_model_type::TOTAL_VOLUME] = qMin( chains_->tableData( rowShort, table_model_type::PUT_TOTAL_VOLUME ).toInt(), chains_->tableData( rowLong, table_model_type::PUT_TOTAL_VOLUME ).toInt() );
        result[item_model_type::QUOTE_TIME] = qMin( chains_->tableData( rowShort, table_model_type::PUT_QUOTE_TIME ).toDateTime(), chains_->tableData( rowLong, table_model_type::PUT_QUOTE_TIME ).toDateTime() );
        result[item_model_type::TRADE_TIME] = qMin( chains_->tableData( rowShort, table_model_type::PUT_TRADE_TIME ).toDateTime(), chains_->tableData( rowLong, table_model_type::PUT_TRADE_TIME ).toDateTime() );

        result[item_model_type::MARK] = chains_->tableData( rowShort, table_model_type::PUT_MARK ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_MARK ).toDouble();
        result[item_model_type::MARK_CHANGE] = chains_->tableData( rowShort, table_model_type::PUT_MARK_CHANGE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_MARK_CHANGE ).toDouble();
        result[item_model_type::MARK_PERCENT_CHANGE] = chains_->tableData( rowShort, table_model_type::PUT_MARK_PERCENT_CHANGE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_MARK_PERCENT_CHANGE ).toDouble();
        result[item_model_type::EXCHANGE_NAME] = chains_->tableData( rowShort, table_model_type::PUT_EXCHANGE_NAME );

        // net volatility
        // https://en.wikipedia.org/wiki/Net_volatility#:~:text=Net%20volatility%20refers%20to%20the,implied%20volatility%20of%20the%20spread.
        double volNet = chains_->tableData( rowLong, table_model_type::PUT_VEGA ).toDouble() * chains_->tableData( rowLong, table_model_type::PUT_VOLATILITY ).toDouble();
        volNet -= chains_->tableData( rowShort, table_model_type::PUT_VEGA ).toDouble() * chains_->tableData( rowShort, table_model_type::PUT_VOLATILITY ).toDouble();
        volNet /= chains_->tableData( rowLong, table_model_type::PUT_VEGA ).toDouble() - chains_->tableData( rowShort, table_model_type::PUT_VEGA ).toDouble();

        result[item_model_type::VOLATILITY] = volNet;
        result[item_model_type::DELTA] = chains_->tableData( rowLong, table_model_type::PUT_DELTA ).toDouble() - chains_->tableData( rowShort, table_model_type::PUT_DELTA ).toDouble();
        result[item_model_type::GAMMA] = chains_->tableData( rowLong, table_model_type::PUT_GAMMA ).toDouble() - chains_->tableData( rowShort, table_model_type::PUT_GAMMA ).toDouble();
        result[item_model_type::THETA] = chains_->tableData( rowLong, table_model_type::PUT_THETA ).toDouble() - chains_->tableData( rowShort, table_model_type::PUT_THETA ).toDouble();
        result[item_model_type::VEGA] = chains_->tableData( rowLong, table_model_type::PUT_VEGA ).toDouble() - chains_->tableData( rowShort, table_model_type::PUT_VEGA ).toDouble();
        result[item_model_type::RHO] = chains_->tableData( rowLong, table_model_type::PUT_RHO ).toDouble() - chains_->tableData( rowShort, table_model_type::PUT_RHO ).toDouble();

        result[item_model_type::TIME_VALUE] = chains_->tableData( rowShort, table_model_type::PUT_TIME_VALUE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_TIME_VALUE ).toDouble();
        result[item_model_type::OPEN_INTEREST] = qMin( chains_->tableData( rowShort, table_model_type::PUT_OPEN_INTEREST ).toInt(), chains_->tableData( rowLong, table_model_type::PUT_OPEN_INTEREST ).toInt() );
        result[item_model_type::IS_IN_THE_MONEY] = (chains_->tableData( rowShort, table_model_type::PUT_IS_IN_THE_MONEY ).toBool() || chains_->tableData( rowLong, table_model_type::PUT_IS_IN_THE_MONEY ).toBool());
        result[item_model_type::IS_OUT_OF_THE_MONEY] = !(chains_->tableData( rowShort, table_model_type::PUT_IS_IN_THE_MONEY ).toBool() && chains_->tableData( rowLong, table_model_type::PUT_IS_IN_THE_MONEY ).toBool());

        // net volatility
        // https://en.wikipedia.org/wiki/Net_volatility#:~:text=Net%20volatility%20refers%20to%20the,implied%20volatility%20of%20the%20spread.
        double theoVolNet = chains_->tableData( rowLong, table_model_type::PUT_VEGA ).toDouble() * chains_->tableData( rowLong, table_model_type::PUT_THEO_VOLATILITY ).toDouble();
        theoVolNet -= chains_->tableData( rowShort, table_model_type::PUT_VEGA ).toDouble() * chains_->tableData( rowShort, table_model_type::PUT_THEO_VOLATILITY ).toDouble();
        theoVolNet /= chains_->tableData( rowLong, table_model_type::PUT_VEGA ).toDouble() - chains_->tableData( rowShort, table_model_type::PUT_VEGA ).toDouble();

        result[item_model_type::THEO_OPTION_VALUE] = chains_->tableData( rowShort, table_model_type::PUT_THEO_OPTION_VALUE ).toDouble() - chains_->tableData( rowLong, table_model_type::PUT_THEO_OPTION_VALUE ).toDouble();
        result[item_model_type::THEO_VOLATILITY] = theoVolNet;

        result[item_model_type::IS_MINI] = (chains_->tableData( rowShort, table_model_type::PUT_IS_MINI ).toBool() || chains_->tableData( rowLong, table_model_type::PUT_IS_MINI ).toBool());
        result[item_model_type::IS_NON_STANDARD] = (chains_->tableData( rowShort, table_model_type::PUT_IS_NON_STANDARD ).toBool() || chains_->tableData( rowLong, table_model_type::PUT_IS_NON_STANDARD ).toBool());
        result[item_model_type::IS_INDEX] = (chains_->tableData( rowShort, table_model_type::PUT_IS_INDEX ).toBool() || chains_->tableData( rowLong, table_model_type::PUT_IS_INDEX ).toBool());
        result[item_model_type::IS_WEEKLY] = (chains_->tableData( rowShort, table_model_type::PUT_IS_WEEKLY ).toBool() || chains_->tableData( rowLong, table_model_type::PUT_IS_WEEKLY ).toBool());
        result[item_model_type::IS_QUARTERLY] = (chains_->tableData( rowShort, table_model_type::PUT_IS_QUARTERLY ).toBool() || chains_->tableData( rowLong, table_model_type::PUT_IS_QUARTERLY ).toBool());
        result[item_model_type::EXPIRY_DATE] = chains_->tableData( rowShort, table_model_type::PUT_EXPIRY_DATE );
        result[item_model_type::EXPIRY_TYPE] = chains_->tableData( rowShort, table_model_type::PUT_EXPIRY_TYPE );
        result[item_model_type::DAYS_TO_EXPIRY] = chains_->tableData( rowShort, table_model_type::PUT_DAYS_TO_EXPIRY );
        result[item_model_type::LAST_TRADING_DAY] = chains_->tableData( rowShort, table_model_type::PUT_LAST_TRADING_DAY );
        result[item_model_type::MULTIPLIER] = chains_->tableData( rowShort, table_model_type::PUT_MULTIPLIER );
        result[item_model_type::SETTLEMENT_TYPE] = chains_->tableData( rowShort, table_model_type::PUT_SETTLEMENT_TYPE );
        result[item_model_type::DELIVERABLE_NOTE] = chains_->tableData( rowShort, table_model_type::PUT_DELIVERABLE_NOTE );
    }

    result[item_model_type::STRIKE_PRICE] = chains_->tableData( rowShort, table_model_type::STRIKE_PRICE ).toString() + "/" + chains_->tableData( rowLong, table_model_type::STRIKE_PRICE ).toString();

    // determine historical volatility
    const QDateTime quoteTime( result[item_model_type::QUOTE_TIME].toDateTime() );

    int tradingDaysToExpiry( daysToExpiry_ );
    tradingDaysToExpiry *= AppDatabase::instance()->numTradingDays();
    tradingDaysToExpiry /= AppDatabase::instance()->numDays();

    result[item_model_type::HIST_VOLATILITY] = 100.0 * AppDatabase::instance()->historicalVolatility( chains_->symbol(), quoteTime, tradingDaysToExpiry );

    // expected dividend
    result[item_model_type::DIV_AMOUNT] = totalDivAmount_;
    result[item_model_type::DIV_YIELD] = 100.0 * totalDivYield_;
}

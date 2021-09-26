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

#include "optionprofitcalc.h"

#include "db/appdb.h"
#include "db/optionchaintablemodel.h"

#include <QObject>

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculator::OptionProfitCalculator( double underlying, const table_model_type *chains, item_model_type *results ) :
    valid_( true ),
    underlying_( underlying ),
    chains_( chains ),
    results_( results ),
    minInvestAmount_( 0.0 ),
    maxInvestAmount_( 0.0 ),
    maxLossAmount_( 0.0 ),
    minReturnOnInvestment_( 0.0 ),
    minSpreadPercent_( 0.0 ),
    minVolatility_( 0.0 ),
    maxVolatility_( 0.0 ),
    optionTypes_( ALL_OPTION_TYPES ),
    volatility_( ALL_VOLATILITY ),
    optionTradeCost_( 0.0 ),
    vertDepth_( DEFAULT_VERT_DEPTH )
{
    // validate underlying price
    if ( underlying_ <= 0.0 )
        valid_ = false;

    daysToExpiry_ = calcDaysToExpiry();

    // ignore expired options
    if ( daysToExpiry_ < 0 )
        valid_ = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
OptionProfitCalculator::~OptionProfitCalculator()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
double OptionProfitCalculator::calcDaysToExpiry() const
{
    const QDateTime now( AppDatabase::instance()->currentDateTime() );

    return now.date().daysTo( chains_->expirationDate() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void OptionProfitCalculator::populateResultModel( int row, double strike, bool isCall, item_model_type::ColumnValueMap& result ) const
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

    result[item_model_type::STRIKE_PRICE] = strike;

    // determine historical volatility
    const QDateTime quoteTime( result[item_model_type::QUOTE_TIME].toDateTime() );

    int tradingDaysToExpiry( daysToExpiry_ );
    tradingDaysToExpiry *= AppDatabase::instance()->numTradingDays();
    tradingDaysToExpiry /= AppDatabase::instance()->numDays();

    result[item_model_type::HIST_VOLATILITY] = 100.0 * AppDatabase::instance()->historicalVolatility( chains_->symbol(), quoteTime, tradingDaysToExpiry );
}


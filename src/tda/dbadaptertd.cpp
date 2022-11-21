/**
 * @file dbadaptertd.cpp
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
#include "dbadaptertd.h"
#include "stringsjson.h"

#include "../db/stringsdb.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>

#include <QtConcurrent>

// uncomment to debug content data
//#define DEBUG_JSON
//#define DEBUG_JSON_SAVE

static const QString SUCCESS( "SUCCESS" );

static const QString SINGLE( "SINGLE" );

static const QString WEEKLY( "(Weekly)" );
static const QString QUARTERLY( "(Quarterly)" );

static const QString CALL( "CALL" );
static const QString PUT( "PUT" );

static const QString NULL_STR( "NULL" );

///////////////////////////////////////////////////////////////////////////////////////////////////
TDAmeritradeDatabaseAdapter::TDAmeritradeDatabaseAdapter( QObject *parent ) :
    _Mybase( parent )
{
    // date columns
    dateColumns_.append( JSON_DIV_DATE );
    dateColumns_.append( JSON_DIVIDEND_DATE );
    dateColumns_.append( JSON_DIVIDEND_PAY_DATE );

    // date time columns
    dateTimeColumns_.append( JSON_DATETIME );
    dateTimeColumns_.append( JSON_EXPIRY_DATE );
    dateTimeColumns_.append( JSON_LAST_TRADING_DAY );
    dateTimeColumns_.append( JSON_QUOTE_TIME );
    dateTimeColumns_.append( JSON_REG_MARKET_TRADE_TIME );
    dateTimeColumns_.append( JSON_TRADE_TIME );

    dateTimeColumnsISO_.append( JSON_DIV_DATE );
    dateTimeColumnsISO_.append( JSON_DIVIDEND_DATE );
    dateTimeColumnsISO_.append( JSON_DIVIDEND_PAY_DATE );

    // quotes
    quoteFields_[JSON_52_WK_HIGH] = DB_FIFTY_TWO_WEEK_HIGH;
    quoteFields_[JSON_52_WK_LOW] = DB_FIFTY_TWO_WEEK_LOW;
    quoteFields_[JSON_ASK] = DB_ASK_PRICE;
    quoteFields_[JSON_ASK_ID] = DB_ASK_ID;
    quoteFields_[JSON_ASK_PRICE] = DB_ASK_PRICE;
    quoteFields_[JSON_ASK_SIZE] = DB_ASK_SIZE;
    quoteFields_[JSON_ASSET_MAIN_TYPE] = DB_ASSET_MAIN_TYPE;
    quoteFields_[JSON_ASSET_SUB_TYPE] = DB_ASSET_SUB_TYPE;
    quoteFields_[JSON_ASSET_TYPE] = DB_ASSET_TYPE;
    quoteFields_[JSON_BID] = DB_BID_PRICE;
    quoteFields_[JSON_BID_ASK_SIZE] = "";
    quoteFields_[JSON_BID_ID] = DB_BID_ID;
    quoteFields_[JSON_BID_PRICE] = DB_BID_PRICE;
    quoteFields_[JSON_BID_SIZE] = DB_BID_SIZE;
    quoteFields_[JSON_BID_TICK] = DB_BID_TICK;
    quoteFields_[JSON_CHANGE] = DB_CHANGE;
    quoteFields_[JSON_CLOSE] = DB_CLOSE_PRICE;
    quoteFields_[JSON_CLOSE_PRICE] = DB_CLOSE_PRICE;
    quoteFields_[JSON_CONTRACT_TYPE] = "";
    quoteFields_[JSON_CUSIP] = DB_CUSIP;
    quoteFields_[JSON_DAYS_TO_EXPIRY] = DB_DAYS_TO_EXPIRY;
    quoteFields_[JSON_DELAYED] = DB_IS_DELAYED;
    quoteFields_[JSON_DELIVERABLES] = "";
    quoteFields_[JSON_DELIVERABLE_NOTE] = DB_DELIVERABLE_NOTE;
    quoteFields_[JSON_DELTA] = DB_DELTA;
    quoteFields_[JSON_DESC] = DB_DESC;
    quoteFields_[JSON_DIGITS] = DB_DIGITS;
    quoteFields_[JSON_DIV_AMOUNT] = DB_DIV_AMOUNT;
    quoteFields_[JSON_DIV_YIELD] = DB_DIV_YIELD;
    quoteFields_[JSON_DIV_DATE] = DB_DIV_DATE;
    quoteFields_[JSON_EXCHANGE] = DB_EXCHANGE;
    quoteFields_[JSON_EXCHANGE_NAME] = DB_EXCHANGE_NAME;
    quoteFields_[JSON_EXPIRY_DATE] = DB_EXPIRY_DATE;
    quoteFields_[JSON_EXPIRY_DAY] = "";
    quoteFields_[JSON_EXPIRY_MONTH] = "";
    quoteFields_[JSON_EXPIRY_TYPE] = DB_EXPIRY_TYPE;
    quoteFields_[JSON_EXPIRY_YEAR] = "";
    quoteFields_[JSON_FIFTY_TWO_WEEK_HIGH] = DB_FIFTY_TWO_WEEK_HIGH;
    quoteFields_[JSON_FIFTY_TWO_WEEK_LOW] = DB_FIFTY_TWO_WEEK_LOW;
    quoteFields_[JSON_GAMMA] = DB_GAMMA;
    quoteFields_[JSON_HIGH_PRICE] = DB_HIGH_PRICE;
    quoteFields_[JSON_IMPLIED_YIELD] = DB_IMPLIED_YIELD;
    quoteFields_[JSON_INTRINSIC_VALUE] = DB_INTRINSIC_VALUE;
    quoteFields_[JSON_IN_THE_MONEY] = DB_IS_IN_THE_MONEY;
    quoteFields_[JSON_IS_INDEX] = DB_IS_INDEX;
    quoteFields_[JSON_IS_INDEX_OPTION] = DB_IS_INDEX;
    quoteFields_[JSON_IS_PENNY_PILOT] = DB_IS_PENNY_PILOT;
    quoteFields_[JSON_LAST] = DB_LAST_PRICE;
    quoteFields_[JSON_LAST_ID] = DB_LAST_ID;
    quoteFields_[JSON_LAST_PRICE] = DB_LAST_PRICE;
    quoteFields_[JSON_LAST_SIZE] = DB_LAST_SIZE;
    quoteFields_[JSON_LAST_TRADING_DAY] = DB_LAST_TRADING_DAY;
    quoteFields_[JSON_LOW_PRICE] = DB_LOW_PRICE;
    quoteFields_[JSON_MARGINABLE] = DB_IS_MARGINABLE;
    quoteFields_[JSON_MARK] = DB_MARK;
    quoteFields_[JSON_MARK_CHANGE] = DB_MARK_CHANGE;
    quoteFields_[JSON_MARK_PERCENT_CHANGE] = DB_MARK_PERCENT_CHANGE;
    quoteFields_[JSON_MINI] = DB_IS_MINI;
    quoteFields_[JSON_MONEY_INTRINSIC_VALUE] = DB_INTRINSIC_VALUE;
    quoteFields_[JSON_MULTIPLIER] = DB_MULTIPLIER;
    quoteFields_[JSON_NAV] = DB_NAV;
    quoteFields_[JSON_NET_CHANGE] = DB_CHANGE;
    quoteFields_[JSON_NET_PERCENT_CHANGE] = DB_PERCENT_CHANGE;
    quoteFields_[JSON_NON_STANDARD] = DB_IS_NON_STANDARD;
    quoteFields_[JSON_OPEN_INTEREST] = DB_OPEN_INTEREST;
    quoteFields_[JSON_OPEN_PRICE] = DB_OPEN_PRICE;
    quoteFields_[JSON_OPTION_DELIVERABLES_LIST] = "";
    quoteFields_[JSON_PENNY_PILOT] = DB_IS_PENNY_PILOT;
    quoteFields_[JSON_PERCENT_CHANGE] = DB_PERCENT_CHANGE;
    quoteFields_[JSON_PE_RATIO] = DB_PE_RATIO;
    quoteFields_[JSON_PUT_CALL] = DB_TYPE;
    quoteFields_[JSON_QUOTE_TIME] = DB_QUOTE_TIME;
    quoteFields_[JSON_REG_MARKET_LAST_PRICE] = DB_REG_MARKET_LAST_PRICE;
    quoteFields_[JSON_REG_MARKET_LAST_SIZE] = DB_REG_MARKET_LAST_SIZE;
    quoteFields_[JSON_REG_MARKET_NET_CHANGE] = DB_REG_MARKET_CHANGE;
    quoteFields_[JSON_REG_MARKET_PERCENT_CHANGE] = DB_REG_MARKET_PERCENT_CHANGE;
    quoteFields_[JSON_REG_MARKET_TRADE_TIME] = DB_REG_MARKET_TRADE_TIME;
    quoteFields_[JSON_RHO] = DB_RHO;
    quoteFields_[JSON_SECURITY_STATUS] = DB_SECURITY_STATUS;
    quoteFields_[JSON_SETTLEMENT_TYPE] = DB_SETTLEMENT_TYPE;
    quoteFields_[JSON_SHORTABLE] = DB_IS_SHORTABLE;
    quoteFields_[JSON_STRIKE_PRICE] = DB_STRIKE_PRICE;
    quoteFields_[JSON_SYMBOL] = DB_SYMBOL;
    quoteFields_[JSON_THEO_OPTION_VALUE] = DB_THEO_OPTION_VALUE;
    quoteFields_[JSON_THEO_VOLATILITY] = DB_THEO_VOLATILITY;
    quoteFields_[JSON_THETA] = DB_THETA;
    quoteFields_[JSON_TIME_VALUE] = DB_TIME_VALUE;
    quoteFields_[JSON_TOTAL_VOLUME] = DB_TOTAL_VOLUME;
    quoteFields_[JSON_TRADE_DATE] = ""; // index option?
    quoteFields_[JSON_TRADE_TIME] = DB_TRADE_TIME;
    quoteFields_[JSON_UNDERLYING] = DB_UNDERLYING;
    quoteFields_[JSON_UNDERLYING_PRICE] = DB_UNDERLYING_PRICE;
    quoteFields_[JSON_UV_EXPIRY_TYPE] = DB_EXPIRY_TYPE;
    quoteFields_[JSON_VEGA] = DB_VEGA;
    quoteFields_[JSON_VOLATILITY] = DB_VOLATILITY;

    // option chain
    optionChainFields_[JSON_CALL_EXP_DATE_MAP] = "";
    optionChainFields_[JSON_DAYS_TO_EXPIRY] = "";
    optionChainFields_[JSON_INTEREST_RATE] = DB_INTEREST_RATE;
    optionChainFields_[JSON_INTERVAL] = "";
    optionChainFields_[JSON_IS_DELAYED] = DB_IS_DELAYED;
    optionChainFields_[JSON_IS_INDEX] = DB_IS_INDEX;
    optionChainFields_[JSON_NUM_CONTRACTS] = DB_NUM_CONTRACTS;
    optionChainFields_[JSON_PUT_EXP_DATE_MAP] = "";
    optionChainFields_[JSON_STATUS] = "";
    optionChainFields_[JSON_STRATEGY] = "";
    optionChainFields_[JSON_SYMBOL] = DB_UNDERLYING;
    optionChainFields_[JSON_UNDERLYING] = "";
    optionChainFields_[JSON_UNDERLYING_PRICE] = DB_UNDERLYING_PRICE;
    optionChainFields_[JSON_VOLATILITY] = DB_VOLATILITY;

    // price history
    priceHistoryFields_[JSON_CANDLES] = "";
    priceHistoryFields_[JSON_EMPTY] = "";
    priceHistoryFields_[JSON_END_DATE] = DB_END_DATE;
    priceHistoryFields_[JSON_FREQUENCY] = DB_FREQUENCY;
    priceHistoryFields_[JSON_FREQUENCY_TYPE] = DB_FREQUENCY_TYPE;
    priceHistoryFields_[JSON_PERIOD] = DB_PERIOD;
    priceHistoryFields_[JSON_PERIOD_TYPE] = DB_PERIOD_TYPE;
    priceHistoryFields_[JSON_START_DATE] = DB_START_DATE;
    priceHistoryFields_[JSON_SYMBOL] = DB_SYMBOL;

    priceHistoryFields_[JSON_CLOSE] = DB_CLOSE_PRICE;
    priceHistoryFields_[JSON_DATETIME] = DB_DATETIME;
    priceHistoryFields_[JSON_HIGH] = DB_HIGH_PRICE;
    priceHistoryFields_[JSON_LOW] = DB_LOW_PRICE;
    priceHistoryFields_[JSON_OPEN] = DB_OPEN_PRICE;
    priceHistoryFields_[JSON_VOLUME] = DB_TOTAL_VOLUME;

    // market hours
    marketHoursFields_[JSON_CATEGORY] = DB_CATEGORY;
    marketHoursFields_[JSON_DATE] = DB_DATE;
    marketHoursFields_[JSON_EXCHANGE] = DB_EXCHANGE;
    marketHoursFields_[JSON_IS_OPEN] = DB_IS_OPEN;
    marketHoursFields_[JSON_MARKET_TYPE] = DB_MARKET_TYPE;
    marketHoursFields_[JSON_PRODUCT] = DB_PRODUCT;
    marketHoursFields_[JSON_PRODUCT_NAME] = DB_PRODUCT_NAME;
    marketHoursFields_[JSON_SESSION_HOURS] = "";

    marketHoursFields_[JSON_END] = DB_END;
    marketHoursFields_[JSON_START] = DB_START;

    // accounts
    accountFields_[JSON_ACCOUNT_ID] = DB_ACCOUNT_ID;
    accountFields_[JSON_CURRENT_BALANCES] = "";
    accountFields_[JSON_INITIAL_BALANCES] = "";
    accountFields_[JSON_IS_CLOSING_ONLY_RESTRICT] = DB_IS_CLOSING_ONLY_RESTRICT;
    accountFields_[JSON_IS_DAY_TRADER] = DB_IS_DAY_TRADER;
    accountFields_[JSON_ORDER_STRATEGIES] = "";
    accountFields_[JSON_POSITIONS] = "";
    accountFields_[JSON_PROJECTED_BALANCES] = "";
    accountFields_[JSON_ROUND_TRIPS] = DB_ROUND_TRIPS;
    accountFields_[JSON_TYPE] = DB_TYPE;

    accountFields_[JSON_ACCRUED_INTEREST] = DB_ACCRUED_INTEREST;
    accountFields_[JSON_CASH_BALANCE] = DB_CASH_BALANCE;
    accountFields_[JSON_CASH_RECEIPTS] = DB_CASH_RECEIPTS;
    accountFields_[JSON_LONG_OPTION_MARKET_VALUE] = DB_LONG_OPTION_MARKET_VALUE;
    accountFields_[JSON_LIQUIDATION_VALUE] = DB_LIQUIDATION_VALUE;
    accountFields_[JSON_LONG_MARKET_VALUE] = DB_LONG_MARKET_VALUE;
    accountFields_[JSON_MONEY_MARKET_FUND] = DB_MONEY_MARKET_FUND;
    accountFields_[JSON_SAVINGS] = DB_SAVINGS;
    accountFields_[JSON_SHORT_MARKET_VALUE] = DB_SHORT_MARKET_VALUE;
    accountFields_[JSON_PENDING_DEPOSITS] = DB_PENDING_DEPOSITS;
    accountFields_[JSON_SHORT_OPTION_MARKET_VALUE] = DB_SHORT_OPTION_MARKET_VALUE;
    accountFields_[JSON_MUTUAL_FUND_VALUE] = DB_MUTUAL_FUND_VALUE;
    accountFields_[JSON_BOND_VALUE] = DB_BOND_VALUE;

    accountFields_[JSON_CASH_AVAIL_FOR_TRADING] = DB_CASH_AVAIL_FOR_TRADING;
    accountFields_[JSON_CASH_AVAIL_FOR_WITHDRAWAL] = DB_CASH_AVAIL_FOR_WITHDRAWAL;
    accountFields_[JSON_CASH_CALL] = DB_CASH_CALL;
    accountFields_[JSON_LONG_NON_MARGIN_MARKET_VALUE] = DB_LONG_NON_MARGIN_MARKET_VALUE;
    accountFields_[JSON_TOTAL_CASH] = DB_TOTAL_CASH;
    accountFields_[JSON_CASH_DEBIT_CALL_VALUE] = DB_CASH_DEBIT_CALL_VALUE;
    accountFields_[JSON_UNSETTLED_CASH] = DB_UNSETTLED_CASH;

    accountFields_[JSON_AVAIL_FUNDS] = DB_AVAIL_FUNDS;
    accountFields_[JSON_AVAIL_FUNDS_NON_MARGIN_TRADE] = DB_AVAIL_FUNDS_NON_MARGIN_TRADE;
    accountFields_[JSON_BUYING_POWER] = DB_BUYING_POWER;
    accountFields_[JSON_BUYING_POWER_NON_MARGIN_TRADE] = DB_BUYING_POWER_NON_MARGIN_TRADE;
    accountFields_[JSON_DAY_TRADING_BUYING_POWER] = DB_DAY_TRADING_BUYING_POWER;
    accountFields_[JSON_DAY_TRADING_BUYING_POWER_CALL] = DB_DAY_TRADING_BUYING_POWER_CALL;
    accountFields_[JSON_EQUITY] = DB_EQUITY;
    accountFields_[JSON_EQUITY_PERCENTAGE] = DB_EQUITY_PERCENTAGE;
    accountFields_[JSON_LONG_MARGIN_VALUE] = DB_LONG_MARGIN_VALUE;
    accountFields_[JSON_MAINTENANCE_CALL] = DB_MAINTENANCE_CALL;
    accountFields_[JSON_MAINTENANCE_REQUIREMENT] = DB_MAINTENANCE_REQUIREMENT;
    accountFields_[JSON_MARGIN_BALANCE] = DB_MARGIN_BALANCE;
    accountFields_[JSON_REG_T_CALL] = DB_REG_T_CALL;
    accountFields_[JSON_SHORT_BALANCE] = DB_SHORT_BALANCE;
    accountFields_[JSON_SHORT_MARGIN_VALUE] = DB_SHORT_MARGIN_VALUE;
    accountFields_[JSON_SMA] = DB_SMA;
    accountFields_[JSON_IS_IN_CALL] = DB_IS_IN_CALL;
    accountFields_[JSON_STOCK_BUYING_POWER] = DB_STOCK_BUYING_POWER;
    accountFields_[JSON_OPTION_BUYING_POWER] = DB_OPTION_BUYING_POWER;

    accountFields_[JSON_LONG_STOCK_VALUE] = DB_LONG_STOCK_VALUE;
    accountFields_[JSON_SHORT_STOCK_VALUE] = DB_SHORT_STOCK_VALUE;
    accountFields_[JSON_ACCOUNT_VALUE] = DB_ACCOUNT_VALUE;

    accountFields_[JSON_DAY_TRADING_EQUITY_CALL] = DB_DAY_TRADING_EQUITY_CALL;
    accountFields_[JSON_MARGIN] = DB_MARGIN;
    accountFields_[JSON_MARGIN_EQUITY] = DB_MARGIN_EQUITY;

    // instrument
    instrumentFields_[JSON_ASSET_TYPE] = DB_ASSET_TYPE;
    instrumentFields_[JSON_CUSIP] = DB_CUSIP;
    instrumentFields_[JSON_DESCRIPTION] = DB_DESCRIPTION;
    instrumentFields_[JSON_EXCHANGE] = DB_EXCHANGE;
    instrumentFields_[JSON_FUNDAMENTAL] = "";
    instrumentFields_[JSON_SYMBOL] = DB_SYMBOL;

    instrumentFields_[JSON_HIGH_52] = DB_HIGH_52;
    instrumentFields_[JSON_LOW_52] = DB_LOW_52;

    instrumentFields_[JSON_DIVIDEND_AMOUNT] = DB_DIV_AMOUNT;
    instrumentFields_[JSON_DIVIDEND_YIELD] = DB_DIV_YIELD;
    instrumentFields_[JSON_DIVIDEND_DATE] = DB_DIV_DATE;

    instrumentFields_[JSON_PE_RATIO] = DB_PE_RATIO;
    instrumentFields_[JSON_PEG_RATIO] = DB_PEG_RATIO;
    instrumentFields_[JSON_PB_RATIO] = DB_PB_RATIO;
    instrumentFields_[JSON_PR_RATIO] = DB_PR_RATIO;
    instrumentFields_[JSON_PFC_RATIO] = DB_PFC_RATIO;

    instrumentFields_[JSON_GROSS_MARGIN_TTM] = DB_GROSS_MARGIN_TTM;
    instrumentFields_[JSON_GROSS_MARGIN_MRQ] = DB_GROSS_MARGIN_MRQ;
    instrumentFields_[JSON_NET_PROFIT_MARGIN_TTM] = DB_NET_PROFIT_MARGIN_TTM;
    instrumentFields_[JSON_NET_PROFIT_MARGIN_MRQ] = DB_NET_PROFIT_MARGIN_MRQ;
    instrumentFields_[JSON_OPERATING_MARGIN_TTM] = DB_OPERATING_MARGIN_TTM;
    instrumentFields_[JSON_OPERATING_MARGIN_MRQ] = DB_OPERATING_MARGIN_MRQ;

    instrumentFields_[JSON_RETURN_ON_EQUITY] = DB_RETURN_ON_EQUITY;
    instrumentFields_[JSON_RETURN_ON_ASSETS] = DB_RETURN_ON_ASSETS;
    instrumentFields_[JSON_RETURN_ON_INVESTMENT] = DB_RETURN_ON_INVESTMENT;
    instrumentFields_[JSON_QUICK_RATIO] = DB_QUICK_RATIO;
    instrumentFields_[JSON_CURRENT_RATIO] = DB_CURRENT_RATIO;
    instrumentFields_[JSON_INTEREST_COVERAGE] = DB_INTEREST_COVERAGE;
    instrumentFields_[JSON_TOTAL_DEBT_TO_CAPITAL] = DB_TOTAL_DEBT_TO_CAPITAL;
    instrumentFields_[JSON_LT_DEBT_TO_EQUITY] = DB_LT_DEBT_TO_EQUITY;
    instrumentFields_[JSON_TOTAL_DEBT_TO_EQUITY] = DB_TOTAL_DEBT_TO_EQUITY;

    instrumentFields_[JSON_EPS_TTM] = DB_EPS_TTM;
    instrumentFields_[JSON_EPS_CHANGE_PERCENT_TTM] = DB_EPS_CHANGE_PERCENT_TTM;
    instrumentFields_[JSON_EPS_CHANGE_YEAR] = DB_EPS_CHANGE_YEAR;
    instrumentFields_[JSON_EPS_CHANGE] = DB_EPS_CHANGE;
    instrumentFields_[JSON_REV_CHANGE_YEAR] = DB_REV_CHANGE_YEAR;
    instrumentFields_[JSON_REV_CHANGE_TTM] = DB_REV_CHANGE_TTM;
    instrumentFields_[JSON_REV_CHANGE_IN] = DB_REV_CHANGE_IN;

    instrumentFields_[JSON_SHARES_OUTSTANDING] = DB_SHARES_OUTSTANDING;
    instrumentFields_[JSON_MARKET_CAP_FLOAT] = DB_MARKET_CAP_FLOAT;
    instrumentFields_[JSON_MARKET_CAP] = DB_MARKET_CAP;
    instrumentFields_[JSON_BOOK_VALUE_PER_SHARE] = DB_BOOK_VALUE_PER_SHARE;
    instrumentFields_[JSON_SHORT_INT_TO_FLOAT] = DB_SHORT_INT_TO_FLOAT;
    instrumentFields_[JSON_SHORT_INT_DAY_TO_COVER] = DB_SHORT_INT_DAY_TO_COVER;
    instrumentFields_[JSON_DIV_GROWTH_RATE_3_YEAR] = DB_DIV_GROWTH_RATE_3_YEAR;
    instrumentFields_[JSON_DIVIDEND_PAY_AMOUNT] = DB_DIV_PAY_AMOUNT;
    instrumentFields_[JSON_DIVIDEND_PAY_DATE] = DB_DIV_PAY_DATE;

    instrumentFields_[JSON_BETA] = DB_BETA;
    instrumentFields_[JSON_VOL_1_DAY_AVG] = DB_VOL_1_DAY_AVG;
    instrumentFields_[JSON_VOL_10_DAY_AVG] = DB_VOL_10_DAY_AVG;
    instrumentFields_[JSON_VOL_3_MONTH_AVG] = DB_VOL_3_MONTH_AVG;

    // balances
    balances_[JSON_INITIAL_BALANCES] = DB_INITIAL_BALANCES;
    balances_[JSON_CURRENT_BALANCES] = DB_CURRENT_BALANCES;
    balances_[JSON_PROJECTED_BALANCES] = DB_PROJECTED_BALANCES;

    // session hours
    sessionHours_[JSON_PRE_MARKET] = DB_PRE_MARKET;
    sessionHours_[JSON_REGULAR_MARKET] = DB_REGULAR_MARKET;
    sessionHours_[JSON_POST_MARKET] = DB_POST_MARKET;
    sessionHours_[JSON_OUTCRY_MARKET] = DB_OUTCRY_MARKET;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
TDAmeritradeDatabaseAdapter::~TDAmeritradeDatabaseAdapter()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDatabaseAdapter::transformAccounts( const QJsonArray& a ) const
{
    QJsonArray accounts;

    // iterate account objects
    for ( QJsonArray::const_iterator accountIt( a.constBegin() ); accountIt != a.constEnd(); ++accountIt )
        if ( accountIt->isObject() )
        {
            const QJsonObject account( accountIt->toObject() );

            const QJsonObject::const_iterator securityAccountIt( account.constFind( JSON_SECURITIES_ACCOUNT ) );

            // process security account
            if ( account.constEnd() != securityAccountIt )
                if ( securityAccountIt->isObject() )
                    accounts.append( parseAccount( securityAccountIt->toObject() ) );
        }

    QJsonObject obj;
    obj[DB_ACCOUNTS] = accounts;

    complete( obj );

    LOG_TRACE << "done";
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDatabaseAdapter::transformInstruments( const QJsonObject& tdobj ) const
{
    QJsonArray instruments;

    for ( QJsonObject::const_iterator instIt( tdobj.constBegin() ); instIt != tdobj.constEnd(); ++instIt )
        if ( instIt->isObject() )
            instruments.append( parseInstrument( instIt->toObject() ) );

    QJsonObject obj;
    obj[DB_INSTRUMENTS] = instruments;

    complete( obj );

    LOG_TRACE << "done";
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDatabaseAdapter::transformMarketHours( const QJsonObject& tdobj ) const
{
    QJsonArray marketHours;

    for ( QJsonObject::const_iterator marketIt( tdobj.constBegin() ); marketIt != tdobj.constEnd(); ++marketIt )
        if ( marketIt->isObject() )
            parseMarketHours( marketIt->toObject(), &marketHours );

    QJsonObject obj;
    obj[DB_MARKET_HOURS] = marketHours;

    complete( obj );

    LOG_TRACE << "done";
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDatabaseAdapter::transformOptionChain( const QJsonObject& tdobj ) const
{
    // validate
    const QJsonObject::const_iterator status( tdobj.constFind( JSON_STATUS ) );
    const QJsonObject::const_iterator strategy( tdobj.constFind( JSON_STRATEGY ) );
    const QJsonObject::const_iterator symbol( tdobj.constFind( JSON_SYMBOL ) );

    if (( tdobj.constEnd() == status ) || ( !status->isString() ))
    {
        LOG_WARN << "bad or missing status";
        return false;
    }
    else if (( tdobj.constEnd() == strategy ) || ( !strategy->isString() ))
    {
        LOG_WARN << "bad or missing strategy";
        return false;
    }
    else if (( tdobj.constEnd() == symbol ) || ( !symbol->isString() ))
    {
        LOG_WARN << "bad or missing symbol";
        return false;
    }

    // only interested in 'SINGLE' option chain strategy
    if ( SINGLE != strategy->toString() )
        return true;

    // check success status
    if ( SUCCESS != status->toString() )
    {
        LOG_INFO << "non-success status " << qPrintable( status->toString() );

        // emit empty chain
        QJsonObject optionChain;
        optionChain[DB_OPTIONS] = QJsonObject();
        optionChain[DB_UNDERLYING] = symbol->toString();

        QJsonObject obj;
        obj[DB_OPTION_CHAIN] = optionChain;

        complete( obj );

        return true;
    }

    // ---- //

    LOG_DEBUG << "transform option chain for " << qPrintable( symbol->toString() ) << "...";

    const QJsonObject::const_iterator calls( tdobj.constFind( JSON_CALL_EXP_DATE_MAP ) );
    const QJsonObject::const_iterator puts( tdobj.constFind( JSON_PUT_EXP_DATE_MAP ) );

    const double underlyingPrice( tdobj[JSON_UNDERLYING_PRICE].toDouble() );

    // parse out calls and puts
    QJsonArray options;
    QMutex m;

    QFutureSynchronizer<void> f;

    if (( tdobj.constEnd() != calls ) && ( calls->isObject() ))
    {
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
        f.addFuture( QtConcurrent::run( &_Myt::parseOptionChain, this, calls, underlyingPrice, &options, &m ) );
#else
        f.addFuture( QtConcurrent::run( this, &_Myt::parseOptionChain, calls, underlyingPrice, &options, &m ) );
#endif
    }

    if (( tdobj.constEnd() != puts ) && ( puts->isObject() ))
    {
#if QT_VERSION_CHECK( 6, 2, 0 ) <= QT_VERSION
        f.addFuture( QtConcurrent::run( &_Myt::parseOptionChain, this, puts, underlyingPrice, &options, &m ) );
#else
        f.addFuture( QtConcurrent::run( this, &_Myt::parseOptionChain, puts, underlyingPrice, &options, &m ) );
#endif
    }

    // transform!
    QJsonObject optionChain;
    transform( tdobj, optionChainFields_, optionChain );

    // parse underlying (optional)
    const QJsonObject::const_iterator underlying( tdobj.constFind( JSON_UNDERLYING ) );

    if (( tdobj.constEnd() != underlying ) && ( underlying->isObject() ))
    {
        QJsonArray quotes;
        quotes.append( parseQuote( underlying->toObject() ) );

        optionChain[DB_QUOTES] = quotes;
    }

    // wait for completion
    f.waitForFinished();

    optionChain[DB_OPTIONS] = options;

    QJsonObject obj;
    obj[DB_OPTION_CHAIN] = optionChain;

    complete( obj );

    LOG_TRACE << "done";
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDatabaseAdapter::transformPriceHistory( const QJsonObject& tdobj ) const
{
    // validate
    const QJsonObject::const_iterator candles( tdobj.constFind( JSON_CANDLES ) );
    const QJsonObject::const_iterator empty( tdobj.constFind( JSON_EMPTY ) );
    const QJsonObject::const_iterator freqType( tdobj.constFind( JSON_FREQUENCY_TYPE ) );
    const QJsonObject::const_iterator symbol( tdobj.constFind( JSON_SYMBOL ) );

    if (( tdobj.constEnd() == empty ) || ( !empty->isBool() ))
    {
        LOG_WARN << "bad or missing empty";
        return false;
    }
    else if (( tdobj.constEnd() == freqType ) || ( !freqType->isString() ))
    {
        LOG_WARN << "bad or missing frequency type";
        return false;
    }
    else if (( tdobj.constEnd() == symbol ) || ( !symbol->isString() ))
    {
        LOG_WARN << "bad or missing symbol";
        return false;
    }

    // nothing to parse
    if ( empty->toBool() )
        return true;

    // ---- //

    LOG_DEBUG << "transform price history for " << qPrintable( symbol->toString() ) << "...";

    // transform!
    QJsonObject quoteHistory;
    transform( tdobj, priceHistoryFields_, quoteHistory );

    // parse out candles
    if (( tdobj.constEnd() != candles ) && ( candles->isArray() ))
        quoteHistory[DB_HISTORY] = parsePriceHistory( candles->toArray() );

    QJsonObject obj;
    obj[DB_QUOTE_HISTORY] = quoteHistory;

    complete( obj );

    LOG_TRACE << "done";
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool TDAmeritradeDatabaseAdapter::transformQuotes( const QJsonObject& tdobj ) const
{
    const QDateTime now( QDateTime::currentDateTime() );

    QJsonArray quotes;

    foreach ( const QJsonValue& quoteVal, tdobj )
        if ( quoteVal.isObject() )
            quotes.append( parseQuote( quoteVal.toObject(), now ) );

    QJsonObject obj;
    obj[DB_QUOTES] = quotes;

    complete( obj );

    LOG_TRACE << "done";
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDatabaseAdapter::transform( const QJsonObject& obj, const FieldMap& fieldMap, QJsonObject& result ) const
{
    // iterate all json fields
    for ( QJsonObject::const_iterator f( obj.constBegin() ); f != obj.constEnd(); ++f )
    {
        if ( f->isNull() )
            continue;

        // determine key name
        const QString key( f.key() );

        QString keyAlt( key );

        if ( key.endsWith( "InDouble" ) )
            keyAlt.remove( "InDouble" );
        else if ( key.endsWith( "InLong" ) )
            keyAlt.remove( "InLong" );

        // determine mapping of this field
        FieldMap::const_iterator mapping( fieldMap.constFind( keyAlt ) );

        if ( fieldMap.constEnd() == mapping )
        {
            LOG_WARN << "unhandled field " << qPrintable( key );
            continue;
        }

        const QString mappedKey( mapping.value() );

        if ( mappedKey.isEmpty() )
            continue;

        // datetime
        if (( dateTimeColumns_.contains( keyAlt ) ) || ( dateTimeColumnsISO_.contains( keyAlt ) ))
        {
            QDateTime dt;

            // iso datetime -or-
            // epoch time
            if ( dateTimeColumnsISO_.contains( keyAlt ) )
                dt = QDateTime::fromString( f->toString(), Qt::ISODate );
            else
                dt = QDateTime::fromMSecsSinceEpoch( f->toVariant().toLongLong() );

            if ( dateColumns_.contains( keyAlt ) )
                result[ mappedKey ] = dt.date().toString( Qt::ISODate );
            else
                result[ mappedKey ] = dt.toString( Qt::ISODateWithMs );
        }
        else
        {
            result[ mappedKey ] = f.value();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDatabaseAdapter::complete( const QJsonObject& obj ) const
{
#ifdef DEBUG_JSON
    saveObject( obj, "transform.json" );
#endif

    emit transformComplete( obj );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonObject TDAmeritradeDatabaseAdapter::parseAccount( const QJsonObject& obj ) const
{
    QJsonObject result;
    transform( obj, accountFields_, result );

    for ( FieldMap::const_iterator balance( balances_.constBegin() ); balance != balances_.constEnd(); ++balance )
    {
        const QJsonObject::const_iterator it( obj.constFind( balance.key() ) );

        if (( obj.constEnd() != it ) && ( it->isObject() ))
        {
            QJsonObject balances;
            transform( it->toObject(), accountFields_, balances );

            result[balance.value()] = balances;
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonObject TDAmeritradeDatabaseAdapter::parseInstrument( const QJsonObject& obj ) const
{
    const QJsonObject::const_iterator fundamentalIt( obj.constFind( JSON_FUNDAMENTAL ) );

    QJsonObject result;
    transform( obj, instrumentFields_, result );

    // set fundamental
    if (( obj.constEnd() != fundamentalIt ) && ( fundamentalIt->isObject() ))
    {
        QJsonObject fundamental;
        transform( fundamentalIt->toObject(), instrumentFields_, fundamental );

        result[DB_FUNDAMENTAL] = fundamental;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDatabaseAdapter::parseMarketHours( const QJsonObject& market, QJsonArray *result ) const
{
    // iterate over all products
    foreach ( const QJsonValue& productVal, market )
        if ( productVal.isObject() )
        {
            const QJsonObject product( productVal.toObject() );

            const QJsonObject::const_iterator sessionHours( product.constFind( JSON_SESSION_HOURS ) );

            // transform!
            QJsonObject obj;
            transform( product, marketHoursFields_, obj );

            if ( !obj.contains( DB_IS_OPEN ) )
            {
                LOG_WARN << "missing open flag";
                continue;
            }

            // parse session hours (optional)
            if (( product.constEnd() != sessionHours ) && ( sessionHours->isObject() ))
                obj[DB_SESSION_HOURS] = parseSessionHours( sessionHours->toObject() );

            // ---- //

            // check for 'NULL' category
            if ( obj.contains( DB_CATEGORY ) )
                if (( obj[DB_CATEGORY].isString() ) && ( NULL_STR == obj[DB_CATEGORY].toString() ))
                    obj.remove( DB_CATEGORY );

            // check for 'NULL' exchange
            if ( obj.contains( DB_EXCHANGE ) )
                if (( obj[DB_EXCHANGE].isString() ) && ( NULL_STR == obj[DB_EXCHANGE].toString() ))
                    obj.remove( DB_EXCHANGE );

            // check for closed market, override invalid product to NULL
            if (( !obj[DB_IS_OPEN].toBool() ) && ( obj.contains( DB_PRODUCT ) ))
                obj[DB_PRODUCT] = NULL_STR;

            result->append( obj );
        }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDatabaseAdapter::parseOptionChain( const QJsonObject::const_iterator& it, double underlyingPrice, QJsonArray *result, QMutex *m ) const
{
    QStringList checkForBadValues;
    checkForBadValues.append( DB_VOLATILITY );
    checkForBadValues.append( DB_DELTA );
    checkForBadValues.append( DB_GAMMA );
    checkForBadValues.append( DB_THETA );
    checkForBadValues.append( DB_VEGA );
    checkForBadValues.append( DB_RHO );
    checkForBadValues.append( DB_THEO_OPTION_VALUE );

    // iterate all expirations
    foreach ( const QJsonValue& expiry, it->toObject() )
        if ( expiry.isObject() )
        {
            // iterate all strike prices
            foreach ( const QJsonValue& strikes, expiry.toObject() )
                if ( strikes.isArray() )
                {
                    // process array (for some reason they embed the option within an array)
                    foreach ( const QJsonValue& strikeVal, strikes.toArray() )
                        if ( strikeVal.isObject() )
                        {
                            const QJsonObject strike( strikeVal.toObject() );

                            // check for bad/invalid option
                            const int bidSize( strike[JSON_BID_SIZE].toInt() );
                            const int askSize( strike[JSON_ASK_SIZE].toInt() );
                            const qint64 quoteTime( strike[JSON_QUOTE_TIME_IN_LONG].toVariant().toLongLong() );

                            if (( !bidSize ) && ( !askSize ) && ( !quoteTime ))
                                continue;

                            // transform!
                            QJsonObject obj;
                            transform( strike, quoteFields_, obj );

                            const QString desc( strike[JSON_DESC].toString() );
                            const QString type( strike[JSON_PUT_CALL].toString() );

                            obj[DB_STAMP] = obj[DB_QUOTE_TIME];
                            obj[DB_BID_ASK_SIZE] = QString::number( bidSize ) + " x " + QString::number( askSize );
                            obj[DB_IS_WEEKLY] = desc.contains( "(Weekly)" );
                            obj[DB_IS_QUARTERLY] = desc.contains( "(Quarterly)" );

                            if ( CALL == type )
                                obj[DB_INTRINSIC_VALUE] = underlyingPrice - strike[JSON_STRIKE_PRICE].toDouble();
                            else if ( PUT == type )
                                obj[DB_INTRINSIC_VALUE] = strike[JSON_STRIKE_PRICE].toDouble() - underlyingPrice;

                            // fixup bad values
                            foreach ( const QString& f, checkForBadValues )
                                if (( "NaN" == obj[f].toString() ) ||
                                    ( -999.0 == obj[f].toDouble() ) ||
                                    ( -1.0 == obj[f].toDouble() && DB_THEO_OPTION_VALUE == f ))
                                    obj[f] = QJsonValue();

                            QMutexLocker guard( m );
                            result->append( obj );
                        }
                }
        }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonArray TDAmeritradeDatabaseAdapter::parsePriceHistory( const QJsonArray& a ) const
{
    QJsonArray result;

    foreach ( const QJsonValue& historyVal, a )
        if ( historyVal.isObject() )
        {
            // transform!
            QJsonObject obj;
            transform( historyVal.toObject(), priceHistoryFields_, obj );

            result.append( obj );
        }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonObject TDAmeritradeDatabaseAdapter::parseQuote( const QJsonObject& quote, const QDateTime& stamp ) const
{
    const QString symbol( quote[JSON_SYMBOL].toString() );

    if ( symbol.isEmpty() )
    {
        LOG_WARN << "bad or missing symbol";
        return QJsonObject();
    }

    LOG_DEBUG << "transform quote for " << qPrintable( symbol ) << "...";

    QJsonObject result;
    transform( quote, quoteFields_, result );

    // set stamp
    if ( stamp.isValid() )
        result[DB_STAMP] = stamp.toString( Qt::ISODateWithMs );
    else if ( result.contains( DB_QUOTE_TIME ) )
        result[DB_STAMP] = result[DB_QUOTE_TIME];

    // set bid/ask size
    if (( quote.contains( JSON_BID_SIZE ) ) && ( quote.contains( JSON_ASK_SIZE )))
        result[DB_BID_ASK_SIZE] = QString::number( quote[JSON_BID_SIZE].toInt() ) + " x " + QString::number( quote[JSON_ASK_SIZE].toInt() );

    result[DB_PERCENT_BELOW_FIFTY_TWO_WEEK_HIGH] = 100.0 * (1.0 - (result[DB_MARK].toDouble() / result[DB_FIFTY_TWO_WEEK_HIGH].toDouble()));
    result[DB_PERCENT_ABOVE_FIFTY_TWO_WEEK_LOW] = 100.0 * ((result[DB_MARK].toDouble() / result[DB_FIFTY_TWO_WEEK_LOW].toDouble()) - 1.0);

    result[DB_FIFTY_TWO_WEEK_PRICE_RANGE] = 100.0 *
            ((result[DB_MARK].toDouble() - result[DB_FIFTY_TWO_WEEK_LOW].toDouble()) /
             (result[DB_FIFTY_TWO_WEEK_HIGH].toDouble() - result[DB_FIFTY_TWO_WEEK_LOW].toDouble()));

    // set option fields
    if ( quote.contains( JSON_UNDERLYING ) )
    {
        const QString contractType( quote[JSON_CONTRACT_TYPE].toString() );
        const QString desc( quote[JSON_DESC].toString() );

        const int expiryYear( quote[JSON_EXPIRY_YEAR].toInt() );
        const int expiryMonth( quote[JSON_EXPIRY_MONTH].toInt() );
        const int expiryDay( quote[JSON_EXPIRY_DAY].toInt() );

        result[DB_IS_WEEKLY] = desc.contains( WEEKLY );
        result[DB_IS_QUARTERLY] = desc.contains( QUARTERLY );

        if ( "C" == contractType )
        {
            result[DB_TYPE] = CALL;
            result[DB_IS_IN_THE_MONEY] = (quote[JSON_STRIKE_PRICE].toDouble() <= quote[JSON_UNDERLYING_PRICE].toDouble());
        }
        else if ( "P" == contractType )
        {
            result[DB_TYPE] = PUT;
            result[DB_IS_IN_THE_MONEY] = (quote[JSON_UNDERLYING_PRICE].toDouble() <= quote[JSON_STRIKE_PRICE].toDouble());
        }

        if (( expiryYear ) && ( expiryMonth ) && ( expiryDay ))
        {
            const QDateTime expirationDate(
                QDate( expiryYear, expiryMonth, expiryDay ),
                QTime( 16, 0 ) );

            result[DB_EXPIRY_DATE] = expirationDate.toString( Qt::ISODateWithMs );
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
QJsonObject TDAmeritradeDatabaseAdapter::parseSessionHours( const QJsonObject& sessionHours ) const
{
    const int offsetFromUtc( QDateTime::currentDateTime().offsetFromUtc() );

    QJsonObject result;

    for ( QJsonObject::const_iterator typeIt( sessionHours.constBegin() ); typeIt != sessionHours.constEnd(); ++typeIt )
        if ( typeIt->isArray() )
        {
            const QJsonArray type( typeIt->toArray() );

            // for some reason they embed hours in an array
            if ( 1 == type.size() )
            {
                if ( !sessionHours_.contains( typeIt.key() ) )
                    LOG_WARN << "unhandled session hours type " << qPrintable( typeIt.key() );
                else if ( !type[0].isObject() )
                    LOG_WARN << "not an object";
                else
                {
                    const QString sessionHoursType( sessionHours_[typeIt.key()] );
                    const QJsonObject sessionHours( type[0].toObject() );

                    const QJsonObject::const_iterator start( sessionHours.find( JSON_START ) );
                    const QJsonObject::const_iterator end( sessionHours.find( JSON_END ) );

                    QJsonObject obj;

                    // session start
                    if (( sessionHours.constEnd() != start ) && ( start->isString() ))
                    {
                        QDateTime dt( QDateTime::fromString( start->toString(), Qt::ISODate ) );

                        // convert to local time
                        const int delta( offsetFromUtc - dt.offsetFromUtc() );

                        if ( delta )
                            dt = dt.addSecs( delta );

                        obj[DB_START] = dt.toLocalTime().toString( Qt::ISODate );
                    }

                    // session end
                    if (( sessionHours.constEnd() != end ) && ( end->isString() ))
                    {
                        QDateTime dt( QDateTime::fromString( end->toString(), Qt::ISODate ) );

                        // convert to local time
                        const int delta( offsetFromUtc - dt.offsetFromUtc() );

                        if ( delta )
                            dt = dt.addSecs( delta );

                        obj[DB_END] = dt.toLocalTime().toString( Qt::ISODate );
                    }

                    result[sessionHoursType] = obj;
                }
            }
        }

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void TDAmeritradeDatabaseAdapter::saveObject( const QJsonObject& obj, const QString& filename )
{
#if !defined( DEBUG_JSON )
    Q_UNUSED( obj )
    Q_UNUSED( filename )
#elif !defined( DEBUG_JSON_SAVE )
    Q_UNUSED( filename )
#endif

#ifdef DEBUG_JSON
    const QJsonDocument doc( obj );
    const QByteArray a( doc.toJson() );

#if defined(HAVE_CLIO_H)
    LOG_TRACE << HEX_DUMP( a.constData(), a.length() );
#else
    LOG_TRACE << a;
#endif

#ifdef DEBUG_JSON_SAVE
    QFile f( filename );

    if ( f.open( QFile::WriteOnly ) )
    {
        f.write( a );
        f.close();
    }
#endif
#endif
}


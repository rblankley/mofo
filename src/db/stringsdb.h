/**
 * @file stringsdb.h
 * String values for application database.
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

#ifndef STRINGSDB_H
#define STRINGSDB_H

#include <QString>

///////////////////////////////////////////////////////////////////////////////////////////////////

inline static const QString DB_ACCOUNTS                             = "accounts";
inline static const QString DB_INSTRUMENTS                          = "instruments";
inline static const QString DB_MARKET_HOURS                         = "marketHours";
inline static const QString DB_OPTION_CHAIN                         = "optionChain";
inline static const QString DB_QUOTES                               = "quotes";
inline static const QString DB_QUOTE_HISTORY                        = "quoteHistory";
inline static const QString DB_TREAS_BILL_RATES                     = "treasuryBillRates";
inline static const QString DB_TREAS_YIELD_CURVE_RATES              = "treasuryYieldCurveRates";

inline static const QString DB_DESC                                 = "description";
inline static const QString DB_STAMP                                = "stamp";
inline static const QString DB_SYMBOL                               = "symbol";
inline static const QString DB_TYPE                                 = "type";

inline static const QString DB_INITIAL_BALANCES                     = "INITIAL";
inline static const QString DB_CURRENT_BALANCES                     = "CURRENT";
inline static const QString DB_PROJECTED_BALANCES                   = "PROJECTED";

inline static const QString DB_PRE_MARKET                           = "PRE_MARKET";
inline static const QString DB_REGULAR_MARKET                       = "REGULAR_MARKET";
inline static const QString DB_POST_MARKET                          = "POST_MARKET";
inline static const QString DB_OUTCRY_MARKET                        = "OUTCRY_MARKET";

inline static const QString DB_TREAS_BILL                           = "TREAS_BILL";
inline static const QString DB_TREAS_YIELD_CURVE                    = "TREAS_YIELD_CURVE";

// Account
inline static const QString DB_ACCOUNT_ID                           = "accountId";
inline static const QString DB_IS_CLOSING_ONLY_RESTRICT             = "isClosingOnlyRestricted";
inline static const QString DB_IS_DAY_TRADER                        = "isDayTrader";
inline static const QString DB_ROUND_TRIPS                          = "roundTrips";

// Account Balances
inline static const QString DB_ACCRUED_INTEREST                     = "accruedInterest";
inline static const QString DB_CASH_BALANCE                         = "cashBalance";
inline static const QString DB_CASH_RECEIPTS                        = "cashReceipts";
inline static const QString DB_LONG_OPTION_MARKET_VALUE             = "longOptionMarketValue";
inline static const QString DB_LIQUIDATION_VALUE                    = "liquidationValue";
inline static const QString DB_LONG_MARKET_VALUE                    = "longMarketValue";
inline static const QString DB_MONEY_MARKET_FUND                     = "moneyMarketFund";
inline static const QString DB_SAVINGS                              = "savings";
inline static const QString DB_SHORT_MARKET_VALUE                   = "shortMarketValue";
inline static const QString DB_PENDING_DEPOSITS                     = "pendingDeposits";
inline static const QString DB_SHORT_OPTION_MARKET_VALUE            = "shortOptionMarketValue";
inline static const QString DB_MUTUAL_FUND_VALUE                    = "mutualFundValue";
inline static const QString DB_BOND_VALUE                           = "bondValue";

inline static const QString DB_CASH_AVAIL_FOR_TRADING               = "cashAvailableForTrading";
inline static const QString DB_CASH_AVAIL_FOR_WITHDRAWAL            = "cashAvailableForWithdrawal";
inline static const QString DB_CASH_CALL                            = "cashCall";
inline static const QString DB_LONG_NON_MARGIN_MARKET_VALUE         = "longNonMarginableMarketValue";
inline static const QString DB_TOTAL_CASH                           = "totalCash";
inline static const QString DB_CASH_DEBIT_CALL_VALUE                = "cashDebitCallValue";
inline static const QString DB_UNSETTLED_CASH                       = "unsettledCash";

inline static const QString DB_AVAIL_FUNDS                          = "availableFunds";
inline static const QString DB_AVAIL_FUNDS_NON_MARGIN_TRADE         = "availableFundsNonMarginableTrade";
inline static const QString DB_BUYING_POWER                         = "buyingPower";
inline static const QString DB_BUYING_POWER_NON_MARGIN_TRADE        = "buyingPowerNonMarginableTrade";
inline static const QString DB_DAY_TRADING_BUYING_POWER             = "dayTradingBuyingPower";
inline static const QString DB_DAY_TRADING_BUYING_POWER_CALL        = "dayTradingBuyingPowerCall";
inline static const QString DB_EQUITY                               = "equity";
inline static const QString DB_EQUITY_PERCENTAGE                    = "equityPercentage";
inline static const QString DB_LONG_MARGIN_VALUE                    = "longMarginValue";
inline static const QString DB_MAINTENANCE_CALL                     = "maintenanceCall";
inline static const QString DB_MAINTENANCE_REQUIREMENT              = "maintenanceRequirement";
inline static const QString DB_MARGIN_BALANCE                       = "marginBalance";
inline static const QString DB_REG_T_CALL                           = "regTCall";
inline static const QString DB_SHORT_BALANCE                        = "shortBalance";
inline static const QString DB_SHORT_MARGIN_VALUE                   = "shortMarginValue";
inline static const QString DB_SMA                                  = "sma";
inline static const QString DB_IS_IN_CALL                           = "isInCall";
inline static const QString DB_STOCK_BUYING_POWER                   = "stockBuyingPower";
inline static const QString DB_OPTION_BUYING_POWER                  = "optionBuyingPower";

inline static const QString DB_LONG_STOCK_VALUE                     = "longStockValue";
inline static const QString DB_SHORT_STOCK_VALUE                    = "shortStockValue";
inline static const QString DB_ACCOUNT_VALUE                        = "accountValue";

inline static const QString DB_DAY_TRADING_EQUITY_CALL              = "dayTradingEquityCall";
inline static const QString DB_MARGIN                               = "margin";
inline static const QString DB_MARGIN_EQUITY                        = "marginEquity";

// Market Hours
inline static const QString DB_DATE                                 = "date";
inline static const QString DB_MARKET_TYPE                          = "marketType";
inline static const QString DB_PRODUCT                              = "product";
inline static const QString DB_IS_OPEN                              = "isOpen";
inline static const QString DB_CATEGORY                             = "category";

inline static const QString DB_PRODUCT_NAME                         = "productName";

// Session Hours
inline static const QString DB_SESSION_HOURS                        = "sessionHours";
inline static const QString DB_START                                = "start";
inline static const QString DB_END                                  = "end";

inline static const QString DB_SESSION_HOURS_TYPE                   = "sessionHoursType";

// Quotes
inline static const QString DB_UNDERLYING                           = "underlying";

inline static const QString DB_ASSET_MAIN_TYPE                      = "assetMainType";
inline static const QString DB_ASSET_SUB_TYPE                       = "assetSubType";
inline static const QString DB_ASSET_TYPE                           = "assetType";
inline static const QString DB_CUSIP                                = "cusip";

inline static const QString DB_BID_ASK_SIZE                         = "bidAskSize";

inline static const QString DB_BID_PRICE                            = "bidPrice";
inline static const QString DB_BID_SIZE                             = "bidSize";
inline static const QString DB_BID_ID                               = "bidId";
inline static const QString DB_BID_TICK                             = "bidTick";

inline static const QString DB_ASK_PRICE                            = "askPrice";
inline static const QString DB_ASK_SIZE                             = "askSize";
inline static const QString DB_ASK_ID                               = "askId";

inline static const QString DB_LAST_PRICE                           = "lastPrice";
inline static const QString DB_LAST_SIZE                            = "lastSize";
inline static const QString DB_LAST_ID                              = "lastId";

inline static const QString DB_OPEN_PRICE                           = "openPrice";
inline static const QString DB_HIGH_PRICE                           = "highPrice";
inline static const QString DB_LOW_PRICE                            = "lowPrice";
inline static const QString DB_CLOSE_PRICE                          = "closePrice";

inline static const QString DB_CHANGE                               = "change";
inline static const QString DB_PERCENT_CHANGE                       = "percentChange";

inline static const QString DB_TOTAL_VOLUME                         = "totalVolume";
inline static const QString DB_QUOTE_TIME                           = "quoteTime";
inline static const QString DB_TRADE_TIME                           = "tradeTime";

inline static const QString DB_MARK                                 = "mark";
inline static const QString DB_MARK_CHANGE                          = "markChange";
inline static const QString DB_MARK_PERCENT_CHANGE                  = "markPercentChange";

inline static const QString DB_FIFTY_TWO_WEEK_HIGH                  = "fiftyTwoWeekHigh";
inline static const QString DB_FIFTY_TWO_WEEK_LOW                   = "fiftyTwoWeekLow";
inline static const QString DB_PERCENT_BELOW_FIFTY_TWO_WEEK_HIGH    = "percentBelowFiftyTwoWeekHigh";
inline static const QString DB_PERCENT_ABOVE_FIFTY_TWO_WEEK_LOW     = "percentAboveFiftyTwoWeekLow";
inline static const QString DB_FIFTY_TWO_WEEK_PRICE_RANGE           = "fiftyTwoWeekPriceRange";

inline static const QString DB_EXCHANGE                             = "exchange";
inline static const QString DB_EXCHANGE_NAME                        = "exchangeName";

inline static const QString DB_IS_MARGINABLE                        = "isMarginable";
inline static const QString DB_IS_SHORTABLE                         = "isShortable";
inline static const QString DB_IS_DELAYED                           = "isDelayed";
inline static const QString DB_IS_PENNY_PILOT                       = "isPennyPilot";

inline static const QString DB_VOLATILITY                           = "volatility";
inline static const QString DB_DIGITS                               = "digits";

inline static const QString DB_NAV                                  = "nAV";
inline static const QString DB_PE_RATIO                             = "peRatio";

inline static const QString DB_DIV_AMOUNT                           = "divAmount";
inline static const QString DB_DIV_YIELD                            = "divYield";
inline static const QString DB_DIV_DATE                             = "divDate";
inline static const QString DB_DIV_FREQUENCY                        = "divFrequency";

inline static const QString DB_IMPLIED_YIELD                        = "impliedYield";

inline static const QString DB_SECURITY_STATUS                      = "securityStatus";

inline static const QString DB_REG_MARKET_LAST_PRICE                = "regularMarketLastPrice";
inline static const QString DB_REG_MARKET_LAST_SIZE                 = "regularMarketLastSize";
inline static const QString DB_REG_MARKET_CHANGE                    = "regularMarketChange";
inline static const QString DB_REG_MARKET_PERCENT_CHANGE            = "regularMarketPercentChange";
inline static const QString DB_REG_MARKET_TRADE_TIME                = "regularMarketTradeTime";

inline static const QString DB_TICK                                 = "tick";
inline static const QString DB_TICK_AMOUNT                          = "tickAmount";

inline static const QString DB_TRADING_HOURS                        = "tradingHours";
inline static const QString DB_IS_TRADABLE                          = "isTradable";

inline static const QString DB_MARKET_MAKER                         = "marketMaker";

// Option Chains
inline static const QString DB_INTEREST_RATE                        = "interestRate";
inline static const QString DB_IS_INDEX                             = "isIndex";
inline static const QString DB_NUM_CONTRACTS                        = "numberOfContracts";
inline static const QString DB_UNDERLYING_PRICE                     = "underlyingPrice";

inline static const QString DB_OPTIONS                              = "options";

// Options
inline static const QString DB_STRIKE_PRICE                         = "strikePrice";
inline static const QString DB_BREAK_EVEN_PRICE                     = "breakEvenPrice";
inline static const QString DB_INTRINSIC_VALUE                      = "intrinsicValue";

inline static const QString DB_DELTA                                = "delta";
inline static const QString DB_GAMMA                                = "gamma";
inline static const QString DB_THETA                                = "theta";
inline static const QString DB_VEGA                                 = "vega";
inline static const QString DB_RHO                                  = "rho";

inline static const QString DB_TIME_VALUE                           = "timeValue";
inline static const QString DB_OPEN_INTEREST                        = "openInterest";

inline static const QString DB_IS_IN_THE_MONEY                      = "isInTheMoney";
inline static const QString DB_THEO_OPTION_VALUE                    = "theoreticalOptionValue";
inline static const QString DB_THEO_VOLATILITY                      = "theoreticalVolatility";

inline static const QString DB_IS_MINI                              = "isMini";
inline static const QString DB_IS_NON_STANDARD                      = "isNonStandard";
inline static const QString DB_IS_WEEKLY                            = "isWeekly";
inline static const QString DB_IS_QUARTERLY                         = "isQuarterly";

inline static const QString DB_EXPIRY_DATE                          = "expirationDate";
inline static const QString DB_EXPIRY_TYPE                          = "expirationType";
inline static const QString DB_DAYS_TO_EXPIRY                       = "daysToExpiration";
inline static const QString DB_LAST_TRADING_DAY                     = "lastTradingDay";
inline static const QString DB_MULTIPLIER                           = "multiplier";

inline static const QString DB_SETTLEMENT_TYPE                      = "settlementType";
inline static const QString DB_DELIVERABLE_NOTE                     = "deliverableNote";

// Quote History
inline static const QString DB_PERIOD                               = "period";
inline static const QString DB_PERIOD_TYPE                          = "periodType";
inline static const QString DB_FREQUENCY                            = "frequency";
inline static const QString DB_FREQUENCY_TYPE                       = "frequencyType";
inline static const QString DB_START_DATE                           = "startDate";
inline static const QString DB_END_DATE                             = "endDate";

inline static const QString DB_HISTORY                              = "history";
inline static const QString DB_DATETIME                             = "datetime";

inline static const QString DB_MA_DEPTH                             = "maDepth";
inline static const QString DB_HV_DEPTH                             = "hvDepth";
inline static const QString DB_RSI_DEPTH                            = "rsiDepth";
inline static const QString DB_MACD                                 = "macd";

inline static const QString DB_AVERAGE                              = "average";
inline static const QString DB_DEPTH                                = "depth";
inline static const QString DB_VALUE                                = "value";

inline static const QString DB_EMA12                                = "ema12";
inline static const QString DB_EMA26                                = "ema26";
inline static const QString DB_SIGNAL_VALUE                         = "signalValue";
inline static const QString DB_DIFF                                 = "diff";

// Instrument (Fundamental Data)
inline static const QString DB_DESCRIPTION                          = "description";
inline static const QString DB_FUNDAMENTAL                          = "fundamental";

inline static const QString DB_HIGH_52                              = "high52";
inline static const QString DB_LOW_52                               = "low52";

inline static const QString DB_PEG_RATIO                            = "pegRatio";
inline static const QString DB_PB_RATIO                             = "pbRatio";
inline static const QString DB_PR_RATIO                             = "prRatio";
inline static const QString DB_PFC_RATIO                            = "pcfRatio";

inline static const QString DB_GROSS_MARGIN_TTM                     = "grossMarginTTM";
inline static const QString DB_GROSS_MARGIN_MRQ                     = "grossMarginMRQ";
inline static const QString DB_NET_PROFIT_MARGIN_TTM                = "netProfitMarginTTM";
inline static const QString DB_NET_PROFIT_MARGIN_MRQ                = "netProfitMarginMRQ";
inline static const QString DB_OPERATING_MARGIN_TTM                 = "operatingMarginTTM";
inline static const QString DB_OPERATING_MARGIN_MRQ                 = "operatingMarginMRQ";

inline static const QString DB_RETURN_ON_EQUITY                     = "returnOnEquity";
inline static const QString DB_RETURN_ON_ASSETS                     = "returnOnAssets";
inline static const QString DB_RETURN_ON_INVESTMENT                 = "returnOnInvestment";
inline static const QString DB_QUICK_RATIO                          = "quickRatio";
inline static const QString DB_CURRENT_RATIO                        = "currentRatio";
inline static const QString DB_INTEREST_COVERAGE                    = "interestCoverage";
inline static const QString DB_TOTAL_DEBT_TO_CAPITAL                = "totalDebtToCapital";
inline static const QString DB_LT_DEBT_TO_EQUITY                    = "ltDebtToEquity";
inline static const QString DB_TOTAL_DEBT_TO_EQUITY                 = "totalDebtToEquity";

inline static const QString DB_EPS_TTM                              = "epsTTM";
inline static const QString DB_EPS_CHANGE_PERCENT_TTM               = "epsChangePercentTTM";
inline static const QString DB_EPS_CHANGE_YEAR                      = "epsChangeYear";
inline static const QString DB_EPS_CHANGE                           = "epsChange";
inline static const QString DB_REV_CHANGE_YEAR                      = "revChangeYear";
inline static const QString DB_REV_CHANGE_TTM                       = "revChangeTTM";
inline static const QString DB_REV_CHANGE_IN                        = "revChangeIn";

inline static const QString DB_SHARES_OUTSTANDING                   = "sharesOutstanding";
inline static const QString DB_MARKET_CAP_FLOAT                     = "marketCapFloat";
inline static const QString DB_MARKET_CAP                           = "marketCap";
inline static const QString DB_BOOK_VALUE_PER_SHARE                 = "bookValuePerShare";
inline static const QString DB_SHORT_INT_TO_FLOAT                   = "shortIntToFloat";
inline static const QString DB_SHORT_INT_DAY_TO_COVER               = "shortIntDayToCover";
inline static const QString DB_DIV_GROWTH_RATE_3_YEAR               = "divGrowthRate3Year";
inline static const QString DB_DIV_PAY_AMOUNT                       = "divPayAmount";
inline static const QString DB_DIV_PAY_DATE                         = "divPayDate";

inline static const QString DB_BETA                                 = "beta";
inline static const QString DB_VOL_1_DAY_AVG                        = "vol1DayAvg";
inline static const QString DB_VOL_10_DAY_AVG                       = "vol10DayAvg";
inline static const QString DB_VOL_3_MONTH_AVG                      = "vol3MonthAvg";

// Treasury Bill Rates
inline static const QString DB_UPDATED                              = "updated";
inline static const QString DB_DATA                                 = "data";

inline static const QString DB_DATA_ID                              = "dataId";
inline static const QString DB_MATURITY_DATE                        = "maturityDate";
inline static const QString DB_ROUND_CLOSE                          = "roundClose";
inline static const QString DB_ROUND_YIELD                          = "roundYield";
inline static const QString DB_CLOSE_AVG                            = "closeAverage";
inline static const QString DB_YIELD_AVG                            = "yieldAverage";
inline static const QString DB_WEEK                                 = "week";

// Treasury Yield Curve Rates
inline static const QString DB_MONTHS                               = "months";
inline static const QString DB_RATE                                 = "rate";

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // STRINGSDB_H

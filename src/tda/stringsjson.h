/**
 * @file stringsjson.h
 * String values for TDA JSON documents.
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

#ifndef STRINGSJSON_H
#define STRINGSJSON_H

#include <QString>

///////////////////////////////////////////////////////////////////////////////////////////////////

// Account
inline static const QString JSON_SECURITIES_ACCOUNT                 = "securitiesAccount";

inline static const QString JSON_TYPE                               = "type";
inline static const QString JSON_ACCOUNT_ID                         = "accountId";
inline static const QString JSON_ROUND_TRIPS                        = "roundTrips";
inline static const QString JSON_IS_DAY_TRADER                      = "isDayTrader";
inline static const QString JSON_IS_CLOSING_ONLY_RESTRICT           = "isClosingOnlyRestricted";
inline static const QString JSON_POSITIONS                          = "positions";
inline static const QString JSON_ORDER_STRATEGIES                   = "orderStrategies";

inline static const QString JSON_INITIAL_BALANCES                   = "initialBalances";
inline static const QString JSON_CURRENT_BALANCES                   = "currentBalances";
inline static const QString JSON_PROJECTED_BALANCES                 = "projectedBalances";

inline static const QString JSON_ACCRUED_INTEREST                   = "accruedInterest";
inline static const QString JSON_CASH_BALANCE                       = "cashBalance";
inline static const QString JSON_CASH_RECEIPTS                      = "cashReceipts";
inline static const QString JSON_LONG_OPTION_MARKET_VALUE           = "longOptionMarketValue";
inline static const QString JSON_LIQUIDATION_VALUE                  = "liquidationValue";
inline static const QString JSON_LONG_MARKET_VALUE                  = "longMarketValue";
inline static const QString JSON_MONEY_MARKET_FUND                  = "moneyMarketFund";
inline static const QString JSON_SAVINGS                            = "savings";
inline static const QString JSON_SHORT_MARKET_VALUE                 = "shortMarketValue";
inline static const QString JSON_PENDING_DEPOSITS                   = "pendingDeposits";
inline static const QString JSON_SHORT_OPTION_MARKET_VALUE          = "shortOptionMarketValue";
inline static const QString JSON_MUTUAL_FUND_VALUE                  = "mutualFundValue";
inline static const QString JSON_BOND_VALUE                         = "bondValue";

inline static const QString JSON_CASH_AVAIL_FOR_TRADING             = "cashAvailableForTrading";
inline static const QString JSON_CASH_AVAIL_FOR_WITHDRAWAL          = "cashAvailableForWithdrawal";
inline static const QString JSON_CASH_CALL                          = "cashCall";
inline static const QString JSON_LONG_NON_MARGIN_MARKET_VALUE       = "longNonMarginableMarketValue";
inline static const QString JSON_TOTAL_CASH                         = "totalCash";
inline static const QString JSON_CASH_DEBIT_CALL_VALUE              = "cashDebitCallValue";
inline static const QString JSON_UNSETTLED_CASH                     = "unsettledCash";

inline static const QString JSON_AVAIL_FUNDS                        = "availableFunds";
inline static const QString JSON_AVAIL_FUNDS_NON_MARGIN_TRADE       = "availableFundsNonMarginableTrade";
inline static const QString JSON_BUYING_POWER                       = "buyingPower";
inline static const QString JSON_BUYING_POWER_NON_MARGIN_TRADE      = "buyingPowerNonMarginableTrade";
inline static const QString JSON_DAY_TRADING_BUYING_POWER           = "dayTradingBuyingPower";
inline static const QString JSON_DAY_TRADING_BUYING_POWER_CALL      = "dayTradingBuyingPowerCall";
inline static const QString JSON_EQUITY                             = "equity";
inline static const QString JSON_EQUITY_PERCENTAGE                  = "equityPercentage";
inline static const QString JSON_LONG_MARGIN_VALUE                  = "longMarginValue";
inline static const QString JSON_MAINTENANCE_CALL                   = "maintenanceCall";
inline static const QString JSON_MAINTENANCE_REQUIREMENT            = "maintenanceRequirement";
inline static const QString JSON_MARGIN_BALANCE                     = "marginBalance";
inline static const QString JSON_REG_T_CALL                         = "regTCall";
inline static const QString JSON_SHORT_BALANCE                      = "shortBalance";
inline static const QString JSON_SHORT_MARGIN_VALUE                 = "shortMarginValue";
inline static const QString JSON_SMA                                = "sma";
inline static const QString JSON_IS_IN_CALL                         = "isInCall";
inline static const QString JSON_STOCK_BUYING_POWER                 = "stockBuyingPower";
inline static const QString JSON_OPTION_BUYING_POWER                = "optionBuyingPower";

inline static const QString JSON_LONG_STOCK_VALUE                   = "longStockValue";
inline static const QString JSON_SHORT_STOCK_VALUE                  = "shortStockValue";
inline static const QString JSON_ACCOUNT_VALUE                      = "accountValue";

inline static const QString JSON_DAY_TRADING_EQUITY_CALL            = "dayTradingEquityCall";
inline static const QString JSON_MARGIN                             = "margin";
inline static const QString JSON_MARGIN_EQUITY                      = "marginEquity";

inline static const QString JSON_SHORT_QUANTITY                     = "shortQuantity";
inline static const QString JSON_LONG_QUANTITY                      = "longQuantity";
inline static const QString JSON_AGED_QUANTITY                      = "agedQuantity";
inline static const QString JSON_SETTLED_SHORT_QUANTITY             = "settledShortQuantity";
inline static const QString JSON_SETTLED_LONG_QUANTITY              = "settledLongQuantity";
inline static const QString JSON_PREV_SESSION_SHORT_QUANTITY        = "previousSessionShortQuantity";
inline static const QString JSON_PREV_SESSION_LONG_QUANTITY         = "previousSessionLongQuantity";

inline static const QString JSON_AVERAGE_PRICE                      = "averagePrice";
inline static const QString JSON_CURRENT_DAY_COST                   = "currentDayCost";
inline static const QString JSON_CURRENT_DAY_PROFIT_LOSS            = "currentDayProfitLoss";
inline static const QString JSON_CURRENT_DAY_PROFIT_LOSS_PERCENT    = "currentDayProfitLossPercentage";
inline static const QString JSON_MARKET_VALUE                       = "marketValue";

inline static const QString JSON_INSTRUMENT                         = "instrument";

// Market Hours
inline static const QString JSON_CATEGORY                           = "category";
inline static const QString JSON_DATE                               = "date";
inline static const QString JSON_EXCHANGE                           = "exchange";
inline static const QString JSON_IS_OPEN                            = "isOpen";
inline static const QString JSON_MARKET_TYPE                        = "marketType";
inline static const QString JSON_PRODUCT                            = "product";
inline static const QString JSON_PRODUCT_NAME                       = "productName";
inline static const QString JSON_SESSION_HOURS                      = "sessionHours";

inline static const QString JSON_PRE_MARKET                         = "preMarket";
inline static const QString JSON_REGULAR_MARKET                     = "regularMarket";
inline static const QString JSON_POST_MARKET                        = "postMarket";
inline static const QString JSON_OUTCRY_MARKET                      = "outcryMarket";

inline static const QString JSON_END                                = "end";
inline static const QString JSON_START                              = "start";

// Quote
inline static const QString JSON_SYMBOL                             = "symbol";
inline static const QString JSON_DESC                               = "description";
inline static const QString JSON_ASSET_MAIN_TYPE                    = "assetMainType";
inline static const QString JSON_ASSET_SUB_TYPE                     = "assetSubType";
inline static const QString JSON_ASSET_TYPE                         = "assetType";
inline static const QString JSON_CUSIP                              = "cusip";

inline static const QString JSON_BID_ASK_SIZE                       = "bidAskSize";

inline static const QString JSON_BID                                = "bid";
inline static const QString JSON_BID_PRICE                          = "bidPrice";
inline static const QString JSON_BID_SIZE                           = "bidSize";
inline static const QString JSON_BID_ID                             = "bidId";
inline static const QString JSON_BID_TICK                           = "bidTick";

inline static const QString JSON_ASK                                = "ask";
inline static const QString JSON_ASK_PRICE                          = "askPrice";
inline static const QString JSON_ASK_SIZE                           = "askSize";
inline static const QString JSON_ASK_ID                             = "askId";

inline static const QString JSON_LAST                               = "last";
inline static const QString JSON_LAST_PRICE                         = "lastPrice";
inline static const QString JSON_LAST_SIZE                          = "lastSize";
inline static const QString JSON_LAST_ID                            = "lastId";

inline static const QString JSON_OPEN                               = "open";
inline static const QString JSON_OPEN_PRICE                         = "openPrice";
inline static const QString JSON_HIGH_PRICE                         = "highPrice";
inline static const QString JSON_LOW_PRICE                          = "lowPrice";
inline static const QString JSON_CLOSE                              = "close";
inline static const QString JSON_CLOSE_PRICE                        = "closePrice";

inline static const QString JSON_CHANGE                             = "change";
inline static const QString JSON_PERCENT_CHANGE                     = "percentChange";

inline static const QString JSON_NET_CHANGE                         = "netChange";
inline static const QString JSON_NET_PERCENT_CHANGE                 = "netPercentChange";

inline static const QString JSON_52_WK_HIGH                         = "52WkHigh";
inline static const QString JSON_FIFTY_TWO_WEEK_HIGH                = "fiftyTwoWeekHigh";

inline static const QString JSON_52_WK_LOW                          = "52WkLow";
inline static const QString JSON_FIFTY_TWO_WEEK_LOW                 = "fiftyTwoWeekLow";

inline static const QString JSON_MARK                               = "mark";
inline static const QString JSON_MARK_CHANGE                        = "markChange";
inline static const QString JSON_MARK_PERCENT_CHANGE                = "markPercentChange";

inline static const QString JSON_DIGITS                             = "digits";
inline static const QString JSON_EXCHANGE_NAME                      = "exchangeName";
inline static const QString JSON_QUOTE_TIME                         = "quoteTime";
inline static const QString JSON_QUOTE_TIME_IN_LONG                 = "quoteTimeInLong";
inline static const QString JSON_TOTAL_VOLUME                       = "totalVolume";
inline static const QString JSON_TRADE_TIME                         = "tradeTime";
inline static const QString JSON_MARGINABLE                         = "marginable";
inline static const QString JSON_SHORTABLE                          = "shortable";
inline static const QString JSON_DELAYED                            = "delayed";
inline static const QString JSON_SECURITY_STATUS                    = "securityStatus";

inline static const QString JSON_NAV                                = "nAV";
inline static const QString JSON_PE_RATIO                           = "peRatio";
inline static const QString JSON_DIV_AMOUNT                         = "divAmount";
inline static const QString JSON_DIV_YIELD                          = "divYield";
inline static const QString JSON_DIV_DATE                           = "divDate";

inline static const QString JSON_REG_MARKET_LAST_PRICE              = "regularMarketLastPrice";
inline static const QString JSON_REG_MARKET_LAST_SIZE               = "regularMarketLastSize";
inline static const QString JSON_REG_MARKET_NET_CHANGE              = "regularMarketNetChange";
inline static const QString JSON_REG_MARKET_PERCENT_CHANGE          = "regularMarketPercentChange";
inline static const QString JSON_REG_MARKET_TRADE_TIME              = "regularMarketTradeTime";

inline static const QString JSON_TICK                               = "tick";
inline static const QString JSON_TICK_AMOUNT                        = "tickAmount";
inline static const QString JSON_TRADING_HOURS                      = "tradingHours";

inline static const QString JSON_IS_TRADABLE                        = "isTradable";
inline static const QString JSON_IS_MARGINABLE                      = "isMarginable";
inline static const QString JSON_IS_SHORTABLE                       = "isShortable";
inline static const QString JSON_IS_DELAYED                         = "isDelayed";
inline static const QString JSON_MARKET_MAKER                       = "marketMaker";

inline static const QString JSON_CONTRACT_TYPE                      = "contractType";
inline static const QString JSON_DELIVERABLES                       = "deliverables";
inline static const QString JSON_EXPIRY_DAY                         = "expirationDay";
inline static const QString JSON_EXPIRY_MONTH                       = "expirationMonth";
inline static const QString JSON_EXPIRY_YEAR                        = "expirationYear";
inline static const QString JSON_IMPLIED_YIELD                      = "impliedYield";

inline static const QString JSON_IS_PENNY_PILOT                     = "isPennyPilot";
inline static const QString JSON_MONEY_INTRINSIC_VALUE              = "moneyIntrinsicValue";
inline static const QString JSON_UV_EXPIRY_TYPE                     = "uvExpirationType";

inline static const QString JSON_PENNY_PILOT                        = "pennyPilot";
inline static const QString JSON_INTRINSIC_VALUE                    = "intrinsicValue";

// Option Chain
inline static const QString JSON_DAYS_TO_EXPIRY                     = "daysToExpiration";
inline static const QString JSON_INTEREST_RATE                      = "interestRate";
inline static const QString JSON_INTERVAL                           = "interval";
inline static const QString JSON_IS_INDEX                           = "isIndex";
inline static const QString JSON_NUM_CONTRACTS                      = "numberOfContracts";
inline static const QString JSON_STATUS                             = "status";
inline static const QString JSON_STRATEGY                           = "strategy";
inline static const QString JSON_UNDERLYING_PRICE                   = "underlyingPrice";
inline static const QString JSON_VOLATILITY                         = "volatility";

inline static const QString JSON_CALL_EXP_DATE_MAP                  = "callExpDateMap";
inline static const QString JSON_PUT_EXP_DATE_MAP                   = "putExpDateMap";

inline static const QString JSON_UNDERLYING                         = "underlying";
inline static const QString JSON_PUT_CALL                           = "putCall";

inline static const QString JSON_DELTA                              = "delta";
inline static const QString JSON_GAMMA                              = "gamma";
inline static const QString JSON_THETA                              = "theta";
inline static const QString JSON_VEGA                               = "vega";
inline static const QString JSON_RHO                                = "rho";

inline static const QString JSON_TIME_VALUE                         = "timeValue";
inline static const QString JSON_OPEN_INTEREST                      = "openInterest";

inline static const QString JSON_IS_IN_THE_MONEY                    = "isInTheMoney";
inline static const QString JSON_THEO_OPTION_VALUE                  = "theoreticalOptionValue";
inline static const QString JSON_THEO_VOLATILITY                    = "theoreticalVolatility";

inline static const QString JSON_IS_MINI                            = "isMini";
inline static const QString JSON_IS_NON_STANDARD                    = "isNonStandard";

inline static const QString JSON_STRIKE_PRICE                       = "strikePrice";
inline static const QString JSON_EXPIRY_DATE                        = "expirationDate";
inline static const QString JSON_EXPIRY_TYPE                        = "expirationType";
inline static const QString JSON_LAST_TRADING_DAY                   = "lastTradingDay";
inline static const QString JSON_MULTIPLIER                         = "multiplier";

inline static const QString JSON_SETTLEMENT_TYPE                    = "settlementType";
inline static const QString JSON_DELIVERABLE_NOTE                   = "deliverableNote";
inline static const QString JSON_IS_INDEX_OPTION                    = "isIndexOption";

inline static const QString JSON_OPTION_DELIVERABLES_LIST           = "optionDeliverablesList";
inline static const QString JSON_TRADE_DATE                         = "tradeDate";

inline static const QString JSON_MINI                               = "mini";
inline static const QString JSON_NON_STANDARD                       = "nonStandard";
inline static const QString JSON_IN_THE_MONEY                       = "inTheMoney";

// Price History
inline static const QString JSON_CANDLES                            = "candles";
inline static const QString JSON_EMPTY                              = "empty";

inline static const QString JSON_PERIOD                             = "period";
inline static const QString JSON_PERIOD_TYPE                        = "periodType";
inline static const QString JSON_FREQUENCY                          = "frequency";
inline static const QString JSON_FREQUENCY_TYPE                     = "frequencyType";
inline static const QString JSON_START_DATE                         = "startDate";
inline static const QString JSON_END_DATE                           = "endDate";

inline static const QString JSON_DATETIME                           = "datetime";
inline static const QString JSON_HIGH                               = "high";
inline static const QString JSON_LOW                                = "low";
inline static const QString JSON_VOLUME                             = "volume";

// Instrument (Fundamental Data)
inline static const QString JSON_FUNDAMENTAL                        = "fundamental";

inline static const QString JSON_HIGH_52                            = "high52";
inline static const QString JSON_LOW_52                             = "low52";

inline static const QString JSON_DIVIDEND_AMOUNT                    = "dividendAmount";
inline static const QString JSON_DIVIDEND_YIELD                     = "dividendYield";
inline static const QString JSON_DIVIDEND_DATE                      = "dividendDate";

inline static const QString JSON_PEG_RATIO                          = "pegRatio";
inline static const QString JSON_PB_RATIO                           = "pbRatio";
inline static const QString JSON_PR_RATIO                           = "prRatio";
inline static const QString JSON_PFC_RATIO                          = "pcfRatio";

inline static const QString JSON_GROSS_MARGIN_TTM                   = "grossMarginTTM";
inline static const QString JSON_GROSS_MARGIN_MRQ                   = "grossMarginMRQ";
inline static const QString JSON_NET_PROFIT_MARGIN_TTM              = "netProfitMarginTTM";
inline static const QString JSON_NET_PROFIT_MARGIN_MRQ              = "netProfitMarginMRQ";
inline static const QString JSON_OPERATING_MARGIN_TTM               = "operatingMarginTTM";
inline static const QString JSON_OPERATING_MARGIN_MRQ               = "operatingMarginMRQ";

inline static const QString JSON_RETURN_ON_EQUITY                   = "returnOnEquity";
inline static const QString JSON_RETURN_ON_ASSETS                   = "returnOnAssets";
inline static const QString JSON_RETURN_ON_INVESTMENT               = "returnOnInvestment";
inline static const QString JSON_QUICK_RATIO                        = "quickRatio";
inline static const QString JSON_CURRENT_RATIO                      = "currentRatio";
inline static const QString JSON_INTEREST_COVERAGE                  = "interestCoverage";
inline static const QString JSON_TOTAL_DEBT_TO_CAPITAL              = "totalDebtToCapital";
inline static const QString JSON_LT_DEBT_TO_EQUITY                  = "ltDebtToEquity";
inline static const QString JSON_TOTAL_DEBT_TO_EQUITY               = "totalDebtToEquity";

inline static const QString JSON_EPS_TTM                            = "epsTTM";
inline static const QString JSON_EPS_CHANGE_PERCENT_TTM             = "epsChangePercentTTM";
inline static const QString JSON_EPS_CHANGE_YEAR                    = "epsChangeYear";
inline static const QString JSON_EPS_CHANGE                         = "epsChange";
inline static const QString JSON_REV_CHANGE_YEAR                    = "revChangeYear";
inline static const QString JSON_REV_CHANGE_TTM                     = "revChangeTTM";
inline static const QString JSON_REV_CHANGE_IN                      = "revChangeIn";

inline static const QString JSON_SHARES_OUTSTANDING                 = "sharesOutstanding";
inline static const QString JSON_MARKET_CAP_FLOAT                   = "marketCapFloat";
inline static const QString JSON_MARKET_CAP                         = "marketCap";
inline static const QString JSON_BOOK_VALUE_PER_SHARE               = "bookValuePerShare";
inline static const QString JSON_SHORT_INT_TO_FLOAT                 = "shortIntToFloat";
inline static const QString JSON_SHORT_INT_DAY_TO_COVER             = "shortIntDayToCover";
inline static const QString JSON_DIV_GROWTH_RATE_3_YEAR             = "divGrowthRate3Year";
inline static const QString JSON_DIVIDEND_PAY_AMOUNT                = "dividendPayAmount";
inline static const QString JSON_DIVIDEND_PAY_DATE                  = "dividendPayDate";

inline static const QString JSON_BETA                               = "beta";
inline static const QString JSON_VOL_1_DAY_AVG                      = "vol1DayAvg";
inline static const QString JSON_VOL_10_DAY_AVG                     = "vol10DayAvg";
inline static const QString JSON_VOL_3_MONTH_AVG                    = "vol3MonthAvg";

inline static const QString JSON_MATURITY_DATE                      = "maturityDate";
inline static const QString JSON_VARIABLE_RATE                      = "variableRate";
inline static const QString JSON_FACTOR                             = "factor";

inline static const QString JSON_UNDERLYING_SYMBOL                  = "underlyingSymbol";
inline static const QString JSON_OPTION_MULTIPLIER                  = "optionMultiplier";
inline static const QString JSON_OPTION_DELIVERABLES                = "optionDeliverables";

// Transaction
inline static const QString JSON_TRANS_ID                           = "transactionId";
inline static const QString JSON_TRANS_DATE                         = "transactionDate";
inline static const QString JSON_TRANS_ITEM                         = "transactionItem";
inline static const QString JSON_TRANS_SUB_TYPE                     = "transactionSubType";

inline static const QString JSON_AMOUNT                             = "amount";
inline static const QString JSON_PARENT_ORDER_KEY                   = "parentOrderKey";
inline static const QString JSON_PARENT_CHILD_INDICATOR             = "parentChildIndicator";
inline static const QString JSON_COST                               = "cost";
inline static const QString JSON_INSTRUCTION                        = "instruction";
inline static const QString JSON_POSITION_EFFECT                    = "positionEffect";
inline static const QString JSON_PRICE                              = "price";

inline static const QString JSON_BOND_MATURITY_DATE                 = "bondMaturityDate";
inline static const QString JSON_BOND_INTEREST_RATE                 = "bondInterestRate";
inline static const QString JSON_OPTION_EXPIRY_DATE                 = "optionExpirationDate";
inline static const QString JSON_OPTION_STRIKE_PRICE                = "optionStrikePrice";

inline static const QString JSON_ORDER_DATE                         = "orderDate";
inline static const QString JSON_ORDER_ID                           = "orderId";
inline static const QString JSON_SETTLEMENT_DATE                    = "settlementDate";

inline static const QString JSON_SUB_ACCOUNT                        = "subAccount";
inline static const QString JSON_ACH_STATUS                         = "achStatus";
inline static const QString JSON_CLEARING_REFERENCE_NUM             = "clearingReferenceNumber";
inline static const QString JSON_DAY_TRADE_BUYING_POWER_EFFECT      = "dayTradeBuyingPowerEffect";
inline static const QString JSON_REQUIREMENT_REALLOC_AMOUNT         = "requirementReallocationAmount";
inline static const QString JSON_NET_AMOUNT                         = "netAmount";
inline static const QString JSON_CASH_BALANCE_EFFECT_FLAG           = "cashBalanceEffectFlag";
inline static const QString JSON_FEES                               = "fees";

inline static const QString JSON_ADDITIONAL_FEE                     = "additionalFee";
inline static const QString JSON_CDSC_FEE                           = "cdscFee";
inline static const QString JSON_COMMISSION                         = "commission";
inline static const QString JSON_OPT_REG_FEE                        = "optRegFee";
inline static const QString JSON_OTHER_CHARGES                      = "otherCharges";
inline static const QString JSON_RFEE                               = "rFee";
inline static const QString JSON_REG_FEE                            = "regFee";
inline static const QString JSON_SEC_FEE                            = "secFee";

///////////////////////////////////////////////////////////////////////////////////////////////////

#endif // STRINGSJSON_H

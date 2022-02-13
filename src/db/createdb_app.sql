
/* ------------------------------------------------------------------------------------------------
 * settings
 * ------------------------------------------------------------------------------------------------ */

/* settings table */
CREATE TABLE settings(
    key                                             text PRIMARY KEY not null,
    value                                           text );


/* ------------------------------------------------------------------------------------------------
 * market hours
 * ------------------------------------------------------------------------------------------------ */

/* market types */
CREATE TABLE marketType(
    type                                            text PRIMARY KEY not null,
    hasMarketHours                                  boolean );


/* product type */
CREATE TABLE productType(
    type                                            text PRIMARY KEY not null,
    name                                            text );


/* market hours */
CREATE TABLE marketHours(
    date                                            text not null,          /* DATE */
    marketType                                      text not null,
    product                                         text not null,
    isOpen                                          boolean,
    category                                        text,
    exchange                                        text,
    PRIMARY KEY (date, marketType, product),
    FOREIGN KEY (marketType) REFERENCES marketType(type),
    FOREIGN KEY (product) REFERENCES productType(type) );

CREATE INDEX marketHoursIdx ON marketHours(date, marketType);


/* ------------------------------------------------------------------------------------------------
 * session hours
 * ------------------------------------------------------------------------------------------------ */

/* session hours type */
CREATE TABLE sessionHoursType(
    type                                            text PRIMARY KEY not null,
    isExtendedHours                                 boolean,
    isElectronic                                    boolean );


/* session hours */
CREATE TABLE sessionHours(
    date                                            text not null,          /* DATE */
    marketType                                      text not null,
    product                                         text not null,
    sessionHoursType                                text not null,
    start                                           text,                   /* DATETIME */
    end                                             text,                   /* DATETIME */
    PRIMARY KEY (date, marketType, product, sessionHoursType),
    FOREIGN KEY (date, marketType, product) REFERENCES marketHours(date, marketType, product),
    FOREIGN KEY (product) REFERENCES productType(type),
    FOREIGN KEY (sessionHoursType) REFERENCES sessionHoursType(type) );

CREATE INDEX sessionHoursIdx ON sessionHours(date, marketType, product);


/* ------------------------------------------------------------------------------------------------
 * watchlists
 * ------------------------------------------------------------------------------------------------ */

/* market indices */
CREATE TABLE indices(
    name                                            text PRIMARY KEY not null,
    description                                     text );


/* watchlists */
CREATE TABLE watchlist(
    name                                            text not null,
    symbol                                          text not null,
    description                                     text,
    PRIMARY KEY (name, symbol) );

CREATE INDEX watchlistIdx ON watchlist(name);


/* ------------------------------------------------------------------------------------------------
 * account information
 * ------------------------------------------------------------------------------------------------ */

/* account types */
CREATE TABLE accountTypes(
    type                                            text PRIMARY KEY not null );


/* accounts */
CREATE TABLE accounts(
    accountId                                       text PRIMARY KEY not null,
    type                                            text,
    isClosingOnlyRestricted                         boolean,
    isDayTrader                                     boolean,
    roundTrips                                      integer,
    FOREIGN KEY (type) REFERENCES accountTypes(type) );


/* ------------------------------------------------------------------------------------------------
 * balance information
 * ------------------------------------------------------------------------------------------------ */

CREATE TABLE balanceTypes(
    type                                            text PRIMARY KEY not null );


CREATE TABLE balances(
    stamp                                           text not null,          /* DATETIME */
    accountId                                       text not null,
    type                                            text not null,
    accruedInterest                                 real,
    cashBalance                                     real,
    cashReceipts                                    real,
    longOptionMarketValue                           real,
    liquidationValue                                real,
    longMarketValue                                 real,
    moneyMarketFund                                 real,
    savings                                         real,
    shortMarketValue                                real,
    pendingDeposits                                 real,
    shortOptionMarketValue                          real,
    mutualFundValue                                 real,
    bondValue                                       real,
    /* Cash Account */
    cashAvailableForTrading                         real,
    cashAvailableForWithdrawal                      real,
    cashCall                                        real,
    longNonMarginableMarketValue                    real,
    totalCash                                       real,
    cashDebitCallValue                              real,
    unsettledCash                                   real,
    /* Cash Account (Initial ) */
    longStockValue                                  real,
    shortStockValue                                 real,
    accountValue                                    real,
    /* Margin Account */
    availableFunds                                  real,
    availableFundsNonMarginableTrade                real,
    buyingPower                                     real,
    buyingPowerNonMarginableTrade                   real,
    dayTradingBuyingPower                           real,
    dayTradingBuyingPowerCall                       real,
    equity                                          real,
    equityPercentage                                real,
    longMarginValue                                 real,
    maintenanceCall                                 real,
    maintenanceRequirement                          real,
    marginBalance                                   real,
    regTCall                                        real,
    shortBalance                                    real,
    shortMarginValue                                real,
    sma                                             real,
    isInCall                                        boolean,
    stockBuyingPower                                real,
    optionBuyingPower                               real,
    /* Margin Account (Initial) */
    dayTradingEquityCall                            real,
    margin                                          real,
    marginEquity                                    real,
    PRIMARY KEY (stamp, accountId, type),
    FOREIGN KEY (accountId) REFERENCES accounts(accountId),
    FOREIGN KEY (type) REFERENCES balanceTypes(type) );

CREATE INDEX balancesIdx ON balances(accountId, type);


/* ------------------------------------------------------------------------------------------------
 * risk free interest rates
 * ------------------------------------------------------------------------------------------------ */

CREATE TABLE interestRateSources(
    name                                            text PRIMARY KEY not null );


CREATE TABLE riskFreeInterestRates(
    date                                            text not null,          /* DATE */
    term                                            real not null,
    source                                          text not null,
    rate                                            real,
    PRIMARY KEY (date, term, source),
    FOREIGN KEY (source) REFERENCES interestRateSources(name) );

CREATE INDEX riskFreeInterestRatesIdx ON riskFreeInterestRates(date, source);


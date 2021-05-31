/* database pragmas */
PRAGMA foreign_keys = ON;
PRAGMA journal_mode = MEMORY;
PRAGMA synchronous = OFF;

/* ------------------------------------------------------------------------------------------------
 * settings
 * ------------------------------------------------------------------------------------------------ */

/* settings table */
CREATE TABLE settings(
    key                                             text PRIMARY KEY not null,
    value                                           text );


/* ------------------------------------------------------------------------------------------------
 * option chains
 * ------------------------------------------------------------------------------------------------ */

/* option chains */
CREATE TABLE optionChains(
    stamp                                           text not null,          /* DATETIME */
    underlying                                      text not null,
    underlyingPrice                                 real,
    interestRate                                    real,
    isDelayed                                       boolean,
    isIndex                                         boolean,
    numberOfContracts                               integer,
    volatility                                      real,
    PRIMARY KEY (stamp, underlying) );

CREATE INDEX optionChainsIdx ON optionChains(underlying);


/* ------------------------------------------------------------------------------------------------
 * options
 * ------------------------------------------------------------------------------------------------ */

/* option types */
CREATE TABLE optionType(
    type                                            text PRIMARY KEY not null );


/* options */
CREATE TABLE options(
    stamp                                           text not null,          /* DATETIME */
    symbol                                          text not null,
    underlying                                      text not null,
    type                                            text not null,
    strikePrice                                     real not null,
    description                                     text,
    bidAskSize                                      text,
    bidPrice                                        real,
    bidSize                                         integer,
    askPrice                                        real,
    askSize                                         integer,
    lastPrice                                       real,
    lastSize                                        integer,
    breakEvenPrice                                  real,
    intrinsicValue                                  real,
    openPrice                                       real,
    highPrice                                       real,
    lowPrice                                        real,
    closePrice                                      real,
    change                                          real,
    percentChange                                   real,
    totalVolume                                     integer,
    quoteTime                                       text,                   /* DATETIME */
    tradeTime                                       text,                   /* DATETIME */
    mark                                            real,
    markChange                                      real,
    markPercentChange                               real,
    exchangeName                                    text,
    volatility                                      real,
    delta                                           real,
    gamma                                           real,
    theta                                           real,
    vega                                            real,
    rho                                             real,
    timeValue                                       real,
    openInterest                                    integer,
    isInTheMoney                                    boolean,
    theoreticalOptionValue                          real,
    theoreticalVolatility                           real,
    isMini                                          boolean,
    isNonStandard                                   boolean,
    isIndex                                         boolean,
    isWeekly                                        boolean,
    isQuarterly                                     boolean,
    expirationDate                                  text not null,          /* DATETIME */
    expirationType                                  text,
    daysToExpiration                                integer,
    lastTradingDay                                  text,                   /* DATETIME */
    multiplier                                      integer,
    settlementType                                  text,
    deliverableNote                                 text,
    PRIMARY KEY (stamp, symbol),
    FOREIGN KEY (type) REFERENCES optionType(type) );

CREATE INDEX optionsIdx ON options(symbol);


/* ------------------------------------------------------------------------------------------------
 * option chain strike prices
 * ------------------------------------------------------------------------------------------------ */

/* strike prices */
CREATE TABLE optionChainStrikePrices(
    stamp                                           text not null,          /* DATETIME */
    underlying                                      text not null,
    expirationDate                                  text not null,          /* DATE */
    strikePrice                                     real not null,
    callStamp                                       text,
    callSymbol                                      text,
    putStamp                                        text,
    putSymbol                                       text,
    PRIMARY KEY (stamp, underlying, expirationDate, strikePrice),
    FOREIGN KEY (stamp, underlying) REFERENCES optionChains(stamp, underlying),
    FOREIGN KEY (callStamp, callSymbol) REFERENCES options(stamp, symbol),
    FOREIGN KEY (putStamp, putSymbol) REFERENCES options(stamp, symbol) );

CREATE INDEX optionChainStrikePricesIdx ON optionChainStrikePrices(stamp, underlying, expirationDate);


/* option chain bid/ask view */
CREATE VIEW optionChainBidAskView AS
    SELECT
        s.stamp,
        s.underlying,
        s.expirationDate,
        c.isInTheMoney AS callIsInTheMoney,
        c.bidPrice AS callBidPrice,
        c.askPrice AS callAskPrice,
        c.bidAskSize AS callBidAskSize,
        s.strikePrice,
        p.bidPrice AS putBidPrice,
        p.askPrice AS putAskPrice,
        p.bidAskSize AS putBidAskSize,
        p.isInTheMoney AS putIsInTheMoney
    FROM optionChainStrikePrices s
        LEFT JOIN options c ON s.callStamp=c.stamp AND s.callSymbol=c.symbol
        LEFT JOIN options p ON s.putStamp=p.stamp AND s.putSymbol=p.symbol
    ORDER BY s.stamp, s.underlying, DATE(s.expirationDate), s.strikePrice;


/* option chain view */
CREATE VIEW optionChainView AS
    SELECT
        s.stamp,
        s.underlying,
        s.expirationDate,
        /* CALL */
        s.callSymbol,
        c.description AS callDescription,
        c.bidAskSize AS callBidAskSize,
        c.bidPrice AS callBidPrice,
        c.bidSize AS callBidSize,
        c.askPrice AS callAskPrice,
        c.askSize AS callAskSize,
        c.lastPrice AS callLastPrice,
        c.lastSize AS callLastSize,
        c.breakEvenPrice AS callBreakEvenPrice,
        c.intrinsicValue AS callIntrinsicValue,
        c.openPrice AS callOpenPrice,
        c.highPrice AS callHighPrice,
        c.lowPrice AS callLowPrice,
        c.closePrice AS callClosePrice,
        c.change AS callChange,
        c.percentChange AS callPercentChange,
        c.totalVolume AS callTotalVolume,
        c.quoteTime AS callQuoteTime,
        c.tradeTime AS callTradeTime,
        c.mark AS callMark,
        c.markChange AS callMarkChange,
        c.markPercentChange AS callMarkPercentChange,
        c.exchangeName AS callExchangeName,
        c.volatility AS callVolatility,
        c.delta AS callDelta,
        c.gamma AS callGamma,
        c.theta AS callTheta,
        c.vega AS callVega,
        c.rho AS callRho,
        c.timeValue AS callTimeValue,
        c.openInterest AS callOpenInterest,
        c.isInTheMoney AS callIsInTheMoney,
        c.theoreticalOptionValue AS callTheoreticalOptionValue,
        c.theoreticalVolatility AS callTheoreticalVolatility,
        c.isMini AS callIsMini,
        c.isNonStandard AS callIsNonStandard,
        c.isIndex AS callIsIndex,
        c.isWeekly AS callIsWeekly,
        c.isQuarterly AS callIsQuarterly,
        c.expirationDate AS callExpirationDate,
        c.expirationType AS callExpirationType,
        c.daysToExpiration AS callDaysToExpiration,
        c.lastTradingDay AS callLastTradingDay,
        c.multiplier AS callMultiplier,
        c.settlementType AS callSettlementType,
        c.deliverableNote AS callDeliverableNote,
        /* STRIKE */
        s.strikePrice,
        /* PUT */
        s.putSymbol,
        p.description AS putDescription,
        p.bidAskSize AS putBidAskSize,
        p.bidPrice AS putBidPrice,
        p.bidSize AS putBidSize,
        p.askPrice AS putAskPrice,
        p.askSize AS putAskSize,
        p.lastPrice AS putLastPrice,
        p.lastSize AS putLastSize,
        p.breakEvenPrice AS putBreakEvenPrice,
        p.intrinsicValue AS putIntrinsicValue,
        p.openPrice AS putOpenPrice,
        p.highPrice AS putHighPrice,
        p.lowPrice AS putLowPrice,
        p.closePrice AS putClosePrice,
        p.change AS putChange,
        p.percentChange AS putPercentChange,
        p.totalVolume AS putTotalVolume,
        p.quoteTime AS putQuoteTime,
        p.tradeTime AS putTradeTime,
        p.mark AS putMark,
        p.markChange AS putMarkChange,
        p.markPercentChange AS putMarkPercentChange,
        p.exchangeName AS putExchangeName,
        p.volatility AS putVolatility,
        p.delta AS putDelta,
        p.gamma AS putGamma,
        p.theta AS putTheta,
        p.vega AS putVega,
        p.rho AS putRho,
        p.timeValue AS putTimeValue,
        p.openInterest AS putOpenInterest,
        p.isInTheMoney AS putIsInTheMoney,
        p.theoreticalOptionValue AS putTheoreticalOptionValue,
        p.theoreticalVolatility AS putTheoreticalVolatility,
        p.isMini AS putIsMini,
        p.isNonStandard AS putIsNonStandard,
        p.isIndex AS putIsIndex,
        p.isWeekly AS putIsWeekly,
        p.isQuarterly AS putIsQuarterly,
        p.expirationDate AS putExpirationDate,
        p.expirationType AS putExpirationType,
        p.daysToExpiration AS putDaysToExpiration,
        p.lastTradingDay AS putLastTradingDay,
        p.multiplier AS putMultiplier,
        p.settlementType AS putSettlementType,
        p.deliverableNote AS putDeliverableNote
    FROM optionChainStrikePrices s
        LEFT JOIN options c ON s.callStamp=c.stamp AND s.callSymbol=c.symbol
        LEFT JOIN options p ON s.putStamp=p.stamp AND s.putSymbol=p.symbol
    ORDER BY s.stamp, s.underlying, DATE(s.expirationDate), s.strikePrice;


/* ------------------------------------------------------------------------------------------------
 * quotes
 * ------------------------------------------------------------------------------------------------ */

/* quotes */
CREATE TABLE quotes(
    stamp                                           text not null,          /* DATETIME */
    symbol                                          text not null,
    description                                     text,
    assetMainType                                   text,
    assetSubType                                    text,
    assetType                                       text,
    cusip                                           text,
    bidAskSize                                      text,
    bidPrice                                        real,
    bidSize                                         integer,
    bidId                                           text,
    bidTick                                         text,
    askPrice                                        real,
    askSize                                         integer,
    askId                                           text,
    lastPrice                                       real,
    lastSize                                        integer,
    lastId                                          text,
    openPrice                                       real,
    highPrice                                       real,
    lowPrice                                        real,
    closePrice                                      real,
    change                                          real,
    percentChange                                   real,
    totalVolume                                     integer,
    quoteTime                                       text,                   /* DATETIME */
    tradeTime                                       text,                   /* DATETIME */
    mark                                            real,
    markChange                                      real,
    markPercentChange                               real,
    fiftyTwoWeekHigh                                real,
    fiftyTwoWeekLow                                 real,
    exchange                                        text,
    exchangeName                                    text,
    isMarginable                                    boolean,
    isShortable                                     boolean,
    isDelayed                                       boolean,
    volatility                                      real,
    digits                                          integer,
    nAV                                             real,
    peRatio                                         real,
    impliedYield                                    real,
    divAmount                                       real,
    divYield                                        real,
    divDate                                         text,                   /* DATETIME */
    divFrequency                                    text,
    securityStatus                                  text,
    regularMarketLastPrice                          real,
    regularMarketLastSize                           integer,
    regularMarketChange                             real,
    regularMarketPercentChange                      real,
    regularMarketTradeTime                          text,                   /* DATETIME */
    /* Forex */
    tick                                            real,
    tickAmount                                      real,
    product                                         text,
    tradingHours                                    text,
    isTradable                                      boolean,
    marketMaker                                     text,
    PRIMARY KEY (stamp, symbol) );

CREATE INDEX quotesIdx ON quotes(symbol);


/* ------------------------------------------------------------------------------------------------
 * history
 * ------------------------------------------------------------------------------------------------ */

/* quote history */
CREATE TABLE quoteHistory(
    date                                            text not null,          /* DATE */
    symbol                                          text not null,
    openPrice                                       real,
    highPrice                                       real,
    lowPrice                                        real,
    closePrice                                      real,
    totalVolume                                     integer,
    depth                                           integer,
    PRIMARY KEY (date, symbol) );

CREATE INDEX quoteHistoryIdx ON quoteHistory(symbol);


/* historical volatility */
CREATE TABLE historicalVolatility(
    date                                            text not null,          /* DATE */
    symbol                                          text not null,
    depth                                           integer not null,
    volatility                                      real,
    PRIMARY KEY (date, symbol, depth),
    FOREIGN KEY (date, symbol) REFERENCES quoteHistory(date, symbol) );

CREATE INDEX historicalVolatilityIdx ON historicalVolatility(symbol, depth);

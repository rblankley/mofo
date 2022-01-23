
DROP TABLE quotes;

DROP TABLE historicalVolatility;

DROP TABLE quoteHistory;

DELETE FROM settings WHERE key='lastQuoteHistory';

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
    percentBelowFiftyTwoWeekHigh                    real,
    percentAboveFiftyTwoWeekLow                     real,
    fiftyTwoWeekPriceRange                          real,
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
    maDepth                                         integer,
    hvDepth                                         integer,
    rsiDepth                                        integer,
    PRIMARY KEY (date, symbol) );

CREATE INDEX quoteHistoryIdx ON quoteHistory(symbol);


/* moving average */
CREATE TABLE movingAverage(
    date                                            text not null,          /* DATE */
    symbol                                          text not null,
    type                                            text not null,
    depth                                           integer not null,
    average                                         real,
    PRIMARY KEY (date, symbol, type, depth),
    FOREIGN KEY (date, symbol) REFERENCES quoteHistory(date, symbol) );

CREATE INDEX movingAverageIdx ON movingAverage(symbol, type, depth);


/* historical volatility */
CREATE TABLE historicalVolatility(
    date                                            text not null,          /* DATE */
    symbol                                          text not null,
    depth                                           integer not null,
    volatility                                      real,
    PRIMARY KEY (date, symbol, depth),
    FOREIGN KEY (date, symbol) REFERENCES quoteHistory(date, symbol) );

CREATE INDEX historicalVolatilityIdx ON historicalVolatility(symbol, depth);


/* relative strength index */
CREATE TABLE relativeStrengthIndex(
    date                                            text not null,          /* DATE */
    symbol                                          text not null,
    depth                                           integer not null,
    value                                           real,
    PRIMARY KEY (date, symbol, depth),
    FOREIGN KEY (date, symbol) REFERENCES quoteHistory(date, symbol) );

CREATE INDEX relativeStrengthIndexIdx ON relativeStrengthIndex(symbol, depth);

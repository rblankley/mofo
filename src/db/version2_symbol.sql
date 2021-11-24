/* ------------------------------------------------------------------------------------------------
 * fundamentals
 * ------------------------------------------------------------------------------------------------ */

/* fundamentals */
CREATE TABLE fundamentals(
    stamp                                           text not null,          /* DATETIME */
    symbol                                          text not null,
    high52                                          real,
    low52                                           real,
    divAmount                                       real,
    divYield                                        real,
    divDate                                         text,                   /* DATE */
    divFrequency                                    text,
    peRatio                                         real,
    pegRatio                                        real,
    pbRatio                                         real,
    prRatio                                         real,
    pcfRatio                                        real,
    grossMarginTTM                                  real,
    grossMarginMRQ                                  real,
    netProfitMarginTTM                              real,
    netProfitMarginMRQ                              real,
    operatingMarginTTM                              real,
    operatingMarginMRQ                              real,
    returnOnEquity                                  real,
    returnOnAssets                                  real,
    returnOnInvestment                              real,
    quickRatio                                      real,
    currentRatio                                    real,
    interestCoverage                                real,
    totalDebtToCapital                              real,
    ltDebtToEquity                                  real,
    totalDebtToEquity                               real,
    epsTTM                                          real,
    epsChangePercentTTM                             real,
    epsChangeYear                                   real,
    epsChange                                       real,
    revChangeYear                                   real,
    revChangeTTM                                    real,
    revChangeIn                                     real,
    sharesOutstanding                               integer,
    marketCapFloat                                  real,
    marketCap                                       real,
    bookValuePerShare                               real,
    shortIntToFloat                                 real,
    shortIntDayToCover                              real,
    divGrowthRate3Year                              real,
    divPayAmount                                    real,
    divPayDate                                      text,                   /* DATE */
    beta                                            real,
    vol1DayAvg                                      integer,
    vol10DayAvg                                     integer,
    vol3MonthAvg                                    integer,
    PRIMARY KEY (stamp, symbol) );

CREATE INDEX fundamentalsIdx ON fundamentals(symbol);

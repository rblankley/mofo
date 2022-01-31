
ALTER TABLE quoteHistory
    ADD macd boolean;

/* moving average convergence divergence*/
CREATE TABLE movingAverageConvergenceDivergence(
    date                                            text not null,          /* DATE */
    symbol                                          text not null,
    ema12                                           real,
    ema26                                           real,
    value                                           real,
    signalValue                                     real,
    diff                                            real,
    PRIMARY KEY (date, symbol),
    FOREIGN KEY (date, symbol) REFERENCES quoteHistory(date, symbol) );

CREATE INDEX movingAverageConvergenceDivergenceIdx ON movingAverageConvergenceDivergence(symbol);


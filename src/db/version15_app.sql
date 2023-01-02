
/* ------------------------------------------------------------------------------------------------
 * positions
 * ------------------------------------------------------------------------------------------------ */

CREATE TABLE positions(
    stamp                                           text not null,          /* DATETIME */
    accountId                                       text not null,
    symbol                                          text not null,
    assetType                                       text,
    assetSubType                                    text,
    cusip                                           text,
    description                                     text,
    /* Option */
    underlyingSymbol                                text,
    putCall                                         text,
    optionMultiplier                                real,
    /* Fixed Income */
    maturityDate                                    text,
    variableRate                                    real,
    factor                                          real,
    /* Position */
    shortQuantity                                   real,
    averagePrice                                    real,
    currentDayCost                                  real,
    currentDayProfitLoss                            real,
    currentDayProfitLossPercentage                  real,
    longQuantity                                    real,
    settledShortQuantity                            real,
    settledLongQuantity                             real,
    previousSessionShortQuantity                    real,
    previousSessionLongQuantity                     real,
    agedQuantity                                    real,
    marketValue                                     real,
    maintenanceRequirement                          real,
    PRIMARY KEY (stamp, accountId, symbol),
    FOREIGN KEY (accountId) REFERENCES accounts(accountId),
    FOREIGN KEY (assetType) REFERENCES assetTypes(type) );

CREATE INDEX positionsIdx ON positions(stamp, accountId);


INSERT INTO assetTypes(type) VALUES
    ('INDEX'),
    ('CURRENCY');

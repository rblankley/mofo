
/* ------------------------------------------------------------------------------------------------
 * transaction information
 * ------------------------------------------------------------------------------------------------ */

CREATE TABLE achStatusTypes(
    type                                            text PRIMARY KEY not null );

CREATE TABLE assetTypes(
    type                                            text PRIMARY KEY not null );

CREATE TABLE instructionTypes(
    type                                            text PRIMARY KEY not null );

CREATE TABLE positionEffectTypes(
    type                                            text PRIMARY KEY not null );

CREATE TABLE transactionTypes(
    type                                            text PRIMARY KEY not null,
    effectPosition                                  boolean );

CREATE TABLE transactionSubTypes(
    type                                            text PRIMARY KEY not null,
    description                                     text );


CREATE TABLE transactions(
    accountId                                       text not null,
    transId                                         text not null,
    transDate                                       text,                   /* DATETIME */
    transType                                       text,
    transSubType                                    text,
    transDescription                                text,
    clearingReferenceNumber                         text,
    subAccount                                      text,
    settlementDate                                  text,                   /* DATE */
    netAmount                                       real,
    sma                                             real,
    requirementReallocationAmount                   real,
    dayTradeBuyingPowerEffect                       real,
    cashBalanceEffect                               boolean,
    achStatus                                       text,
    accruedInterest                                 real,
    reconciled                                      boolean,
    /* Order Information */
    orderId                                         text,
    orderDate                                       text,                   /* DATETIME */
    parentOrderKey                                  integer,
    parentChildIndicator                            text,
    instruction                                     text,
    positionEffect                                  text,
    amount                                          real,
    price                                           real,
    cost                                            real,
    /* Instrument */
    assetType                                       text,
    assetSubType                                    text,
    description                                     text,
    cusip                                           text,
    symbol                                          text,
    underlyingSymbol                                text,
    optionExpirationDate                            text,                   /* DATETIME */
    optionStrikePrice                               real,
    bondMaturityDate                                text,                   /* DATETIME */
    bondInterestRate                                real,
    /* Fees */
    commission                                      real,
    cdscFee                                         real,
    regFee                                          real,
    secFee                                          real,
    additionalFee                                   real,
    optRegFee                                       real,
    rFee                                            real,
    otherCharges                                    real,
    PRIMARY KEY (accountId, transId),
    FOREIGN KEY (accountId) REFERENCES accounts(accountId),
    FOREIGN KEY (transType) REFERENCES transactionTypes(type),
    FOREIGN KEY (transSubType) REFERENCES transactionSubTypes(type),
    FOREIGN KEY (achStatus) REFERENCES achStatusTypes(type),
    FOREIGN KEY (instruction) REFERENCES instructionTypes(type),
    FOREIGN KEY (positionEffect) REFERENCES positionEffectTypes(type),
    FOREIGN KEY (assetType) REFERENCES assetTypes(type) );


INSERT INTO achStatusTypes(type) VALUES
    ('Approved'),
    ('Rejected'),
    ('Cancel'),
    ('Error');

INSERT INTO assetTypes(type) VALUES
    ('EQUITY'),
    ('MUTUAL_FUND'),
    ('OPTION'),
    ('FIXED_INCOME'),
    ('CASH_EQUIVALENT');

INSERT INTO instructionTypes(type) VALUES
    ('BUY'),
    ('SELL');

INSERT INTO positionEffectTypes(type) VALUES
    ('OPENING'),
    ('CLOSING'),
    ('AUTOMATIC');

INSERT INTO transactionTypes(type, effectPosition) VALUES
    ('TRADE', true),
    ('RECEIVE_AND_DELIVER', true),
    ('DIVIDEND_OR_INTEREST', false),
    ('ACH_RECEIPT', false),
    ('ACH_DISBURSEMENT', false),
    ('CASH_RECEIPT', false),
    ('CASH_DISBURSEMENT', false),
    ('ELECTRONIC_FUND', false),
    ('WIRE_OUT', false),
    ('WIRE_IN', false),
    ('JOURNAL', false),
    ('MEMORANDUM', false),
    ('MARGIN_CALL', false),
    ('MONEY_MARKET', true),
    ('SMA_ADJUSTMENT', false);

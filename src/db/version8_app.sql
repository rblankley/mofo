
/* ------------------------------------------------------------------------------------------------
 * price history states
 * ------------------------------------------------------------------------------------------------ */

/* price history states table */
CREATE TABLE priceHistoryStates(
    groupName                                       text not null,
    name                                            text not null,
    state                                           text,
    PRIMARY KEY (groupName, name) );

CREATE INDEX priceHistoryStatesIdx ON priceHistoryStates(groupName);

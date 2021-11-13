
DELETE FROM settings WHERE key='optionTradingViewHeaderState';

DELETE FROM settings WHERE key='optionViewerSplitterState';

DELETE FROM settings WHERE key='optionChainViewHeaderState';

/* ------------------------------------------------------------------------------------------------
 * header states
 * ------------------------------------------------------------------------------------------------ */

/* header states table */
CREATE TABLE headerStates(
    groupName                                       text not null,
    name                                            text not null,
    state                                           text,
    PRIMARY KEY (groupName, name) );

CREATE INDEX headerStatesIdx ON headerStates(groupName);




DELETE FROM headerStates WHERE groupName='optionViewerSplitterState';

UPDATE headerStates SET groupName='optionChainView' WHERE groupName='optionChainViewHeaderState';

UPDATE headerStates SET groupName='optionTradingView' WHERE groupName='optionTradingViewHeaderState';

/* ------------------------------------------------------------------------------------------------
 * splitter states
 * ------------------------------------------------------------------------------------------------ */

/* splitter states table */
CREATE TABLE splitterStates(
    groupName                                       text not null,
    name                                            text not null,
    state                                           text,
    PRIMARY KEY (groupName, name) );

CREATE INDEX splitterStatesIdx ON splitterStates(groupName);



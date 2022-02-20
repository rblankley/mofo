
/* ------------------------------------------------------------------------------------------------
 * dialog states
 * ------------------------------------------------------------------------------------------------ */

/* dialog states table */
CREATE TABLE dialogStates(
    groupName                                       text not null,
    name                                            text not null,
    state                                           text,
    PRIMARY KEY (groupName, name) );

CREATE INDEX dialogStatesIdx ON dialogStates(groupName);



BEGIN TRANSACTION;

CREATE TABLE `dep_backup` (
    `id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `version`           text NOT NULL,
    `cashregisterid`    int(11) NOT NULL,
    `datetime`          datetime NOT NULL,
    `text`              text
);
INSERT INTO dep_backup SELECT `id`,`version`,`cashregisterid`,`datetime`,`text` FROM `dep`;
DROP TABLE `dep`;

CREATE TABLE `dep` (
    `id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `version`           text NOT NULL,
    `cashregisterid`    text NOT NULL,
    `datetime`          datetime NOT NULL,
    `text`              text
);

INSERT INTO dep SELECT `id`,`version`,`cashregisterid`,`datetime`,`text` FROM `dep_backup`;
UPDATE dep SET cashregisterid = (SELECT strValue FROM globals WHERE name = 'shopCashRegisterId');
DROP TABLE `dep_backup`;

COMMIT;

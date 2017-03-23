BEGIN TRANSACTION;

CREATE TABLE `journal_backup` (
    `id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `version`           text NOT NULL,
    `cashregisterid`    text NOT NULL,
    `datetime`          datetime NOT NULL,
    `text`              text
);
INSERT INTO journal_backup SELECT `id`,`version`,`cashregisterid`, `datetime`, `text` FROM `journal`;
DROP TABLE `journal`;

CREATE TABLE `journal` (
    `id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `version`           text NOT NULL,
    `cashregisterid`    text NOT NULL,
    `datetime`          datetime NOT NULL,
    `text`              text
);
INSERT INTO `journal`(id,version,cashregisterid,datetime,text) VALUES (NULL,'0.15.1222',0,CURRENT_TIMESTAMP, '');
INSERT INTO journal SELECT NULL,`version`,`cashregisterid`, `datetime`, `text` FROM `journal_backup`;
DROP TABLE `journal_backup`;

COMMIT;

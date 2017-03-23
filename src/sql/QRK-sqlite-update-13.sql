BEGIN TRANSACTION;

CREATE TEMPORARY TABLE `reports_backup` (
`id`	INTEGER NOT NULL,
`receiptNum`	int(11),
`text`	text,
PRIMARY KEY(id)
);
INSERT INTO reports_backup SELECT `id`,`receiptNum`,`text` FROM `reports`;
DROP TABLE `reports`;

CREATE TABLE `reports` (
`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
`receiptNum`	int(11),
`timestamp`	datetime,
`text`	text
);

INSERT INTO reports SELECT `id`,`receiptNum`,'', `text` FROM `reports_backup`;
DROP TABLE `reports_backup`;

UPDATE reports SET `timestamp` = (SELECT `timestamp` FROM receipts WHERE reports.receiptNum = receipts.receiptNum);

CREATE TEMPORARY TABLE `reports_backup` (
`id`	INTEGER NOT NULL,
`receiptNum`	int(11),
`timestamp`	datetime,
`text`	text,
PRIMARY KEY(id)
);
INSERT INTO reports_backup SELECT `id`,`receiptNum`,`timestamp`, `text` FROM `reports`;
DROP TABLE `reports`;

CREATE TABLE `reports` (
`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
`receiptNum`	int(11),
`timestamp`	datetime NOT NULL,
`text`	text
);

CREATE TEMPORARY TABLE `receipts_backup` (
`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
`timestamp`	datetime NOT NULL,
`receiptNum`	int(11) DEFAULT NULL,
`payedBy`	int(11) NOT NULL DEFAULT '0',
`gross`	double NOT NULL DEFAULT '0',
`net`	double NOT NULL DEFAULT '0',
`storno`	int(11) NOT NULL DEFAULT '0',
`stornoId`	int(11) NOT NULL DEFAULT '0'
);
INSERT INTO receipts_backup SELECT `id`,`timestamp`,`receiptNum`,`payedBy`,`gross`,`net`,`storno`,`stornoId` FROM `receipts`;
DROP TABLE `receipts`;

CREATE TABLE `receipts` (
`id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
`timestamp`	datetime NOT NULL,
`infodate`	datetime NOT NULL,
`receiptNum`	int(11) DEFAULT NULL,
`payedBy`	int(11) NOT NULL DEFAULT '0',
`gross`	double NOT NULL DEFAULT '0',
`net`	double NOT NULL DEFAULT '0',
`storno`	int(11) NOT NULL DEFAULT '0',
`stornoId`	int(11) NOT NULL DEFAULT '0'
);
INSERT INTO receipts SELECT `id`,`timestamp`,`timestamp`,`receiptNum`,`payedBy`,`gross`,`net`,`storno`,`stornoId` FROM `receipts_backup`;
DROP TABLE `receipts_backup`;

INSERT INTO reports SELECT `id`,`receiptNum`, `timestamp`, `text` FROM `reports_backup`;
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,5,'Startbeleg','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,6,'Kontrollbeleg','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,7,'Sammelbeleg','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,8,'Monatsbeleg','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,9,'Schlussbeleg','PayedByText');

UPDATE `receipts` SET `infodate` = (SELECT `timestamp` FROM `reports` WHERE receipts.receiptNum=reports.receiptNum) WHERE `receiptNum`= (SELECT `receiptNum` FROM `reports` WHERE receipts.receiptNum = reports.receiptNum);

COMMIT;

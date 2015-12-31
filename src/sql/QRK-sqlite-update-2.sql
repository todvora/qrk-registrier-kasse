BEGIN TRANSACTION;

INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,3,'Tagesabschluss','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,4,'Monatsabschluss','PayedByText');
UPDATE `actionTypes` SET `comment`='PayedByText';

UPDATE `globals` SET `name`='lastReceiptNum' WHERE `name`='lastInvoiceNum';

CREATE TEMPORARY TABLE  `receipts_backup` (
        `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
        `timestamp`	datetime NOT NULL,
        `invoiceNum`	int(11) DEFAULT NULL,
        `payedBy`	int(11) NOT NULL DEFAULT '0',
        `gross`	double NOT NULL DEFAULT '0',
        `net`	double NOT NULL DEFAULT '0',
        `storno`	int(11) NOT NULL DEFAULT '0',
        `stornoId`	int(11) NOT NULL DEFAULT '0',
        `signature`	text
);

INSERT INTO receipts_backup SELECT `id`,`timestamp`,`invoiceNum`,`payedBy`,`gross`,`net`,`storno`,`stornoId`,`signature` FROM `receipts`;
DROP TABLE `receipts`;

CREATE TABLE  `receipts` (
        `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
        `timestamp`	datetime NOT NULL,
        `receiptNum`	int(11) DEFAULT NULL,
        `payedBy`	int(11) NOT NULL DEFAULT '0',
        `gross`	double NOT NULL DEFAULT '0',
        `net`	double NOT NULL DEFAULT '0',
        `storno`	int(11) NOT NULL DEFAULT '0',
        `stornoId`	int(11) NOT NULL DEFAULT '0',
        `signature`	text
);

INSERT INTO receipts SELECT `id`,`timestamp`,`invoiceNum`,`payedBy`,`gross`,`net`,`storno`,`stornoId`,`signature` FROM `receipts_backup`;
DROP TABLE `receipts_backup`;

CREATE INDEX `receipts_stornoId_index` ON `receipts` (`stornoId`);

CREATE TABLE `reports` (
        `id`            INTEGER NOT NULL,
        `receiptNum`	int(11),
        `text`          text,
        PRIMARY KEY(id)
);
CREATE INDEX `reports_receiptNum_index` ON `reports` (`receiptNum`);

COMMIT;

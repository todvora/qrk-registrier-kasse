BEGIN TRANSACTION;

ALTER TABLE dep RENAME TO journal;

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
        `text`	text
);

INSERT INTO reports SELECT `id`,`receiptNum`,`text` FROM `reports_backup`;
DROP TABLE `reports_backup`;

CREATE TABLE `dep` (
        `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
        `receiptNum`	int(11),
        `data`	text
);

CREATE TEMPORARY TABLE `receipts_backup` (
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
INSERT INTO receipts_backup SELECT `id`,`timestamp`,`receiptNum`,`payedBy`,`gross`,`net`,`storno`,`stornoId`,`signature` FROM `receipts`;
DROP TABLE `receipts`;

CREATE TABLE `receipts` (
        `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
        `timestamp`	datetime NOT NULL,
        `receiptNum`	int(11) DEFAULT NULL,
        `payedBy`	int(11) NOT NULL DEFAULT '0',
        `gross`	double NOT NULL DEFAULT '0',
        `net`	double NOT NULL DEFAULT '0',
        `storno`	int(11) NOT NULL DEFAULT '0',
        `stornoId`	int(11) NOT NULL DEFAULT '0'
);
INSERT INTO receipts SELECT `id`,`timestamp`,`receiptNum`,`payedBy`,`gross`,`net`,`storno`,`stornoId` FROM `receipts_backup`;
DROP TABLE `receipts_backup`;

CREATE TEMPORARY TABLE `products_backup` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `itemnum`	text NOT NULL,
    `barcode`	text NOT NULL,
    `name`	text NOT NULL,
    `sold`	double NOT NULL DEFAULT 0,
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `group`	INTEGER NOT NULL DEFAULT 2,
    `visible`	tinyint(1) NOT NULL DEFAULT 1,
    `completer`	tinyint(1) NOT NULL DEFAULT 1,
    `tax`	double NOT NULL DEFAULT '20',
    `color`     text DEFAULT '#808080',
    `button`    text DEFAULT '',
    `image`     text DEFAULT '',
    CONSTRAINT `group` FOREIGN KEY (`group`) REFERENCES `groups` (`id`)
);
INSERT INTO products_backup SELECT `id`,`itemnum`,`barcode`,`name`,`sold`,`net`,`gross`,`group`,`visible`,`completer`,`tax`,`color`,`button`,`image` FROM `products`;
DROP TABLE `products`;

CREATE TABLE `products` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `itemnum`	text NOT NULL,
    `barcode`	text NOT NULL,
    `name`	text NOT NULL,
    `sold`	double NOT NULL DEFAULT 0,
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `group`	INTEGER NOT NULL DEFAULT 2,
    `visible`	tinyint(1) NOT NULL DEFAULT 1,
    `completer`	tinyint(1) NOT NULL DEFAULT 1,
    `tax`	double NOT NULL DEFAULT '20',
    `color`     text DEFAULT '#808080',
    `button`    text DEFAULT '',
    `image`     text DEFAULT '',
    `coupon`	tinyint(1) NOT NULL DEFAULT 0,
    CONSTRAINT `group` FOREIGN KEY (`group`) REFERENCES `groups` (`id`)
);
INSERT INTO products SELECT `id`,`itemnum`,`barcode`,`name`,`sold`,`net`,`gross`,`group`,`visible`,`completer`,`tax`,`color`,`button`,`image`, 0 FROM `products_backup`;
DROP TABLE `products_backup`;

COMMIT;

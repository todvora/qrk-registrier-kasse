CREATE TABLE `actionTypes` (
        `id`	INTEGER PRIMARY KEY AUTOINCREMENT,
        `actionId`	INTEGER NOT NULL,
        `actionText`	TEXT NOT NULL,
        `comment`	TEXT
);

INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,0,'BAR','payedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,1,'Bankomat','payedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,2,'Kreditkarte','payedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,3,'Tagesabschluss','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,4,'Monatsabschluss','PayedByText');

CREATE TABLE `taxTypes` (
        `id`        INTEGER PRIMARY KEY AUTOINCREMENT,
        `tax`       int(11),
        `comment`   TEXT
);

INSERT INTO `taxTypes`(`id`,`tax`,`comment`) VALUES (NULL,20,'Satz-Normal');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`) VALUES (NULL,10,'Satz-Ermaessigt-1');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`) VALUES (NULL,12,'Satz-Ermaessigt-2');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`) VALUES (NULL,13,'Satz-Besonders');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`) VALUES (NULL,0,'Satz-Null');


CREATE TABLE  `globals` (
    `name`      	text NOT NULL,
    `value`     	int(11),
    `strValue`  	text
);

CREATE TABLE  `groups` (
    `id`        	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`      	text NOT NULL,
    `visible`   	tinyint(1) NOT NULL DEFAULT '1'
);

CREATE TABLE `products` (
    `id`		INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`		text NOT NULL,
    `gross`		double NOT NULL,
    `group`		int(11) NOT NULL DEFAULT 0,
    `visible`		tinyint(1) NOT NULL DEFAULT 1,
    `tax`		int(11) NOT NULL DEFAULT '20'
);

CREATE TABLE `orders` (
    `id`		INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptId`		int(11) NOT NULL,
    `product`		int(11) NOT NULL,
    `count`		int(11) NOT NULL DEFAULT '1',
    `gross`		double NOT NULL,
    `tax`		int(11) NOT NULL DEFAULT '0',
    FOREIGN KEY(product) REFERENCES products(id)
);

CREATE INDEX `orders_receiptId_index` ON `orders` (`receiptId`);


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

CREATE INDEX `receipts_stornoId_index` ON `receipts` (`stornoId`);

CREATE TABLE `dep` (
    `id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `version`           text NOT NULL,
    `cashregisterid`    int(11) NOT NULL,
    `datetime`          datetime NOT NULL,
    `text`              text
);

CREATE TABLE `reports` (
        `id`            INTEGER NOT NULL,
        `receiptNum`	int(11),
        `text`          text,
        PRIMARY KEY(id)
);
CREATE INDEX `reports_receiptNum_index` ON `reports` (`receiptNum`);

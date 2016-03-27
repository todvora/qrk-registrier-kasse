USE QRK;

CREATE TABLE  `globals` (
  `name` text NOT NULL,
  `value` int(11),
  `strValue` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `actionTypes` (
        `id`	INTEGER PRIMARY KEY AUTOINCREMENT,
        `actionId`	INTEGER NOT NULL,
        `actionText`	TEXT NOT NULL,
        `comment`	TEXT
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,0,'BAR','payedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,1,'Bankomat','payedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,2,'Kreditkarte','payedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,3,'Tagesabschluss','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,4,'Monatsabschluss','PayedByText');

CREATE TABLE `taxTypes` (
        `id`        INTEGER PRIMARY KEY AUTOINCREMENT,
        `tax`       double,
        `comment`   TEXT,
        `taxlocation`   TEXT
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,20.0,'Satz-Normal','AT');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,10.0,'Satz-Ermaessigt-1','AT');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,13.0,'Satz-Ermaessigt-2','AT');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,19.0,'Satz-Besonders','AT');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,0.0,'Satz-Null','AT');

INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,19.0,'Satz-Normal','DE');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,7.0,'Satz-Ermaessigt-1','DE');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,0.0,'Satz-Null','DE');

INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,8.0,'Satz-Normal','CH');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,2.5,'Satz-Ermaessigt-1','CH');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,3.8,'Satz-Besonders','CH');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,0.0,'Satz-Null','CH');

CREATE TABLE  `globals` (
    `name`      	text NOT NULL,
    `value`     	int(11),
    `strValue`  	text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE  `groups` (
    `id`        	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`      	text NOT NULL,
    `color`      	text DEFAULT '',
    `button`     	text DEFAULT '',
    `image`     	text DEFAULT '',
    `visible`   	tinyint(1) NOT NULL DEFAULT 1,
    PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
INSERT INTO `groups`(`name`,`visible`) VALUES ('auto', 0);
INSERT INTO `groups`(`name`,`visible`) VALUES ('Standard', 1);

CREATE TABLE `products` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`	text NOT NULL,
    `sold`	double NOT NULL DEFAULT 0,
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `group`	INTEGER NOT NULL DEFAULT 2,
    `visible`	tinyint(1) NOT NULL DEFAULT 1,
    `completer`	tinyint(1) NOT NULL DEFAULT 1,
    `tax`	INTEGER NOT NULL DEFAULT '20',
    `color`     text DEFAULT '#808080',
    `button`    text DEFAULT '',
    `image`     text DEFAULT '',
    PRIMARY KEY (`id`),
    KEY `group` (`group`),
    CONSTRAINT `group` FOREIGN KEY (`group`) REFERENCES `groups` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `orders` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptId`	INTEGER NOT NULL,
    `product`	INTEGER NOT NULL,
    `count`	double NOT NULL DEFAULT '1',
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `tax`	double NOT NULL DEFAULT '0.0',
    FOREIGN KEY(`product`) REFERENCES products ( id )
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE INDEX `orders_receiptId_index` ON `orders` (`receiptId`);


CREATE TABLE `receipts` (
    `id`            	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `timestamp`     	datetime NOT NULL,
    `receiptNum`    	int(11) DEFAULT NULL,
    `payedBy`       	int(11) NOT NULL DEFAULT '0',
    `gross`         	double NOT NULL DEFAULT '0',
    `net`           	double NOT NULL DEFAULT '0',
    `storno`            int(11) NOT NULL DEFAULT '0',
    `stornoId`          int(11) NOT NULL DEFAULT '0',
    `signature`     	text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE INDEX `receipts_stornoId_index` ON `receipts` (`stornoId`);

CREATE TABLE `customer` (
    `id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptNum`	int(11) DEFAULT NULL,
    `text`              text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `dep` (
    `id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `version`           text NOT NULL,
    `cashregisterid`    text NOT NULL,
    `datetime`          datetime NOT NULL,
    `text`              text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `reports` (
        `id`            INTEGER NOT NULL,
        `receiptNum`	int(11),
        `text`          text,
        PRIMARY KEY(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE INDEX `reports_receiptNum_index` ON `reports` (`receiptNum`);

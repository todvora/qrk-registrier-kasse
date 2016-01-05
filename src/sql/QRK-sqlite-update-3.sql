BEGIN TRANSACTION;

CREATE TEMPORARY TABLE  `orders_backup` (
`id`		INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
`receiptId`		int(11) NOT NULL,
`product`		int(11) NOT NULL,
`count`		int(11) NOT NULL DEFAULT '1',
`gross`		double NOT NULL,
`tax`		int(11) NOT NULL DEFAULT '0',
FOREIGN KEY(product) REFERENCES products(id)
);
INSERT INTO orders_backup SELECT `id`,`receiptId`,`product`,`count`,`gross`,`tax` FROM `orders`;
DROP TABLE `orders`;

CREATE TABLE `orders` (
    `id`		INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptId`		int(11) NOT NULL,
    `product`		int(11) NOT NULL,
    `count`		int(11) NOT NULL DEFAULT '1',
    `gross`		double NOT NULL,
    `tax`		double NOT NULL DEFAULT '0.0',
    FOREIGN KEY(product) REFERENCES products(id)
);

INSERT INTO orders SELECT `id`,`receiptId`,`product`,`count`,`gross`,`tax` FROM `orders_backup`;
DROP TABLE `orders_backup`;

DROP TABLE `taxTypes`;

CREATE TABLE `taxTypes` (
        `id`        INTEGER PRIMARY KEY AUTOINCREMENT,
        `tax`       double,
        `comment`   TEXT,
        `taxlocation`   TEXT
);

INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,20.0,'Satz-Normal','AT');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,10.0,'Satz-Ermaessigt-1','AT');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,12.0,'Satz-Ermaessigt-2','AT');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,13.0,'Satz-Besonders','AT');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,0.0,'Satz-Null','AT');

INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,19.0,'Satz-Normal','DE');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,7.0,'Satz-Ermaessigt-1','DE');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,0.0,'Satz-Null','DE');

INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,8.0,'Satz-Normal','CH');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,2.5,'Satz-Ermaessigt-1','CH');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,3.8,'Satz-Besonders','CH');
INSERT INTO `taxTypes`(`id`,`tax`,`comment`,`taxlocation`) VALUES (NULL,0.0,'Satz-Null','CH');

COMMIT;

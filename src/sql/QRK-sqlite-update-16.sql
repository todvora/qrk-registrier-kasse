BEGIN TRANSACTION;

CREATE TABLE `orders_backup` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptId`	INTEGER NOT NULL,
    `product`	INTEGER NOT NULL,
    `count`	double NOT NULL DEFAULT '1',
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `tax`	double NOT NULL DEFAULT '0.0',
    FOREIGN KEY(`product`) REFERENCES products ( id )
);

INSERT INTO orders_backup SELECT `id`,`receiptId`,`product`,`count`,`net`,`gross`,`tax` FROM `orders`;
DROP TABLE `orders`;

CREATE TABLE `orders` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptId`	INTEGER NOT NULL,
    `product`	INTEGER NOT NULL,
    `count`	double NOT NULL DEFAULT '1',
    `discount`	double NOT NULL DEFAULT '0',
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `tax`	double NOT NULL DEFAULT '0.0',
    FOREIGN KEY(`product`) REFERENCES products ( id )
);
INSERT INTO orders SELECT `id`,`receiptId`,`product`,`count`,'0',`net`,`gross`,`tax` FROM `orders_backup`;
DROP TABLE `orders_backup`;
CREATE INDEX `orders_receiptId_index` ON `orders` (`receiptId`);

COMMIT;

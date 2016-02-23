BEGIN TRANSACTION;

CREATE INDEX `orders_receiptId_index` ON `orders` (`receiptId`);

CREATE TABLE `orders_backup` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptId`	INTEGER NOT NULL,
    `product`	INTEGER NOT NULL,
    `count`	INTEGER NOT NULL DEFAULT '1',
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `tax`	double NOT NULL DEFAULT '0.0',
    FOREIGN KEY(`product`) REFERENCES products ( id )
);
INSERT INTO orders_backup SELECT `id`,`receiptId`,`product`,`count`,`net`,`gross`, `tax` FROM `orders`;
DROP TABLE `orders`;

CREATE TABLE `orders` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptId`	INTEGER NOT NULL,
    `product`	INTEGER NOT NULL,
    `count`	double NOT NULL DEFAULT '1',
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `tax`	double NOT NULL DEFAULT '0.0',
    FOREIGN KEY(`product`) REFERENCES products ( id )
);

CREATE INDEX `orders_receiptId_index` ON `orders` (`receiptId`);
CREATE INDEX `orders_receiptId_index` ON `orders` (`receiptId`);

INSERT INTO orders SELECT `id`,`receiptId`,`product`,`count`,`net`,`gross`, `tax` FROM `orders_backup`;
DROP TABLE orders_backup;


CREATE TABLE `products_backup` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`	text NOT NULL,
    `sold`	INTEGER NOT NULL DEFAULT 0,
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `group`	INTEGER NOT NULL DEFAULT 0,
    `visible`	tinyint(1) NOT NULL DEFAULT 1,
    `tax`	INTEGER NOT NULL DEFAULT '20'
);

INSERT INTO products_backup SELECT `id`,`name`,`sold`,`net`,`gross`, `group`, `visible`, `tax` FROM `products`;
DROP TABLE `products`;

CREATE TABLE `products` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`	text NOT NULL,
    `sold`	double NOT NULL DEFAULT 0,
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `group`	INTEGER NOT NULL DEFAULT 0,
    `visible`	tinyint(1) NOT NULL DEFAULT 1,
    `completer`	tinyint(1) NOT NULL DEFAULT 1,
    `tax`	double NOT NULL DEFAULT '20'
);

INSERT INTO products SELECT `id`,`name`,`sold`,`net`,`gross`, `group`, `visible`, 1 as completer, `tax` FROM `products_backup`;
DROP TABLE `products_backup`;

COMMIT;

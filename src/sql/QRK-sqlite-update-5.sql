BEGIN TRANSACTION;

CREATE TABLE `orders_backup` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptId`	INTEGER NOT NULL,
    `product`	INTEGER NOT NULL,
    `count`	INTEGER NOT NULL DEFAULT '1',
    `gross`	double NOT NULL,
    `tax`	double NOT NULL DEFAULT '0.0',
    FOREIGN KEY(`product`) REFERENCES products ( id )
);
INSERT INTO orders_backup SELECT `id`,`receiptId`,`product`,`count`,`gross`, `tax` FROM `orders`;
DROP TABLE `orders`;

CREATE TABLE `orders` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptId`	INTEGER NOT NULL,
    `product`	INTEGER NOT NULL,
    `count`	INTEGER NOT NULL DEFAULT '1',
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `tax`	double NOT NULL DEFAULT '0.0',
    FOREIGN KEY(`product`) REFERENCES products ( id )
);
CREATE INDEX `orders_receiptId_index` ON `orders` (`receiptId`);

INSERT INTO orders SELECT id,receiptId,product,count,gross - (gross * tax / (100 + tax)) as net, gross, tax FROM orders_backup;
DROP TABLE orders_backup;


CREATE TABLE `products_backup` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`	text NOT NULL,
    `gross`	double NOT NULL,
    `group`	INTEGER NOT NULL DEFAULT 0,
    `visible`	tinyint(1) NOT NULL DEFAULT 1,
    `tax`	INTEGER NOT NULL DEFAULT '20'
);
INSERT INTO products_backup SELECT `id`,`name`,`gross`,`group`,`visible`, `tax` FROM `products`;
DROP TABLE `products`;


CREATE TABLE `products` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`	text NOT NULL,
    `sold`	INTEGER NOT NULL DEFAULT 0,
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `group`	INTEGER NOT NULL DEFAULT 0,
    `visible`	tinyint(1) NOT NULL DEFAULT 1,
    `tax`	INTEGER NOT NULL DEFAULT '20'
);
INSERT INTO products SELECT `id`,`name`, 0 as sold, gross - (gross * tax / (100 + tax)) as net, `gross`,`group`,`visible`, `tax` FROM `products_backup`;
DROP TABLE products_backup;

DROP TABLE groups;
CREATE TABLE  `groups` (
    `id`        	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`      	text NOT NULL,
    `color`      	text DEFAULT '',
    `button`     	text DEFAULT '',
    `image`     	text DEFAULT '',
    `visible`   	tinyint(1) NOT NULL DEFAULT 1
);
INSERT INTO `groups`(`name`) VALUES ('Standard');

COMMIT;

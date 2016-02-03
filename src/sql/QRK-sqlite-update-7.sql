BEGIN TRANSACTION;

DROP TABLE `groups`;

CREATE TABLE  `groups` (
    `id`        	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`      	text NOT NULL,
    `color`      	text DEFAULT '',
    `button`     	text DEFAULT '',
    `image`     	text DEFAULT '',
    `visible`   	tinyint(1) NOT NULL DEFAULT 1
);
INSERT INTO `groups`(`name`,`visible`) VALUES ('auto', 0);
INSERT INTO `groups`(`name`,`visible`) VALUES ('Standard', 1);

CREATE TABLE `products_backup` (
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

INSERT INTO products_backup SELECT `id`,`name`,`sold`,`net`,`gross`, `group`, `visible`, `completer` , `tax` FROM `products`;
DROP TABLE `products`;

CREATE TABLE `products` (
    `id`	INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `name`	text NOT NULL,
    `sold`	double NOT NULL DEFAULT 0,
    `net`	double NOT NULL,
    `gross`	double NOT NULL,
    `group`	INTEGER NOT NULL DEFAULT 2,
    `visible`	tinyint(1) NOT NULL DEFAULT 1,
    `completer`	tinyint(1) NOT NULL DEFAULT 1,
    `tax`	double NOT NULL DEFAULT '20',
    CONSTRAINT `group` FOREIGN KEY (`group`) REFERENCES `groups` (`id`)
);

INSERT INTO products SELECT `id`,`name`,`sold`,`net`,`gross`, `group`, `visible`, `completer`, `tax` FROM `products_backup`;
DROP TABLE `products_backup`;

COMMIT;

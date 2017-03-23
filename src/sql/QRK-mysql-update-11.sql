SET FOREIGN_KEY_CHECKS=0;
SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;

CREATE TABLE `products_backup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `sold` double NOT NULL DEFAULT '0',
  `net` double NOT NULL,
  `gross` double NOT NULL,
  `group` int(11) NOT NULL DEFAULT '2',
  `visible` tinyint(1) NOT NULL DEFAULT '1',
  `completer` tinyint(1) NOT NULL DEFAULT '1',
  `tax` double NOT NULL DEFAULT '20',
  `color` varchar(255) DEFAULT '#808080',
  `button` varchar(255) DEFAULT '',
  `image` varchar(255) DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `group` (`group`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;
INSERT INTO products_backup SELECT `id`, `name`,`sold`,`net`,`gross`, `group`, `visible`, `completer`, `tax`, `color`, `button`, `image` FROM `products`;
DROP TABLE `products`;

CREATE TABLE `products` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `itemnum` text NOT NULL,
  `barcode` text NOT NULL,
  `name` text NOT NULL,
  `sold` double NOT NULL DEFAULT '0',
  `net` double NOT NULL,
  `gross` double NOT NULL,
  `group` int(11) NOT NULL DEFAULT '2',
  `visible` tinyint(1) NOT NULL DEFAULT '1',
  `completer` tinyint(1) NOT NULL DEFAULT '1',
  `tax` double NOT NULL DEFAULT '20',
  `color` varchar(255) DEFAULT '#808080',
  `button` varchar(255) DEFAULT '',
  `image` varchar(255) DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `group` (`group`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

INSERT INTO products SELECT `id`, '', '',`name`,`sold`,`net`,`gross`, `group`, `visible`, `completer`, `tax`, `color`, `button`, `image` FROM `products_backup`;
DROP TABLE `products_backup`;

SET FOREIGN_KEY_CHECKS=1;
COMMIT;

SET FOREIGN_KEY_CHECKS=0;
SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;

USE QRK;

CREATE TABLE `actionTypes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `actionId` int(11) NOT NULL,
  `actionText` text NOT NULL,
  `comment` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=6 ;

INSERT INTO `actionTypes` (`id`, `actionId`, `actionText`, `comment`) VALUES
(1, 0, 'BAR', 'payedByText'),
(2, 1, 'Bankomat', 'payedByText'),
(3, 2, 'Kreditkarte', 'payedByText'),
(4, 3, 'Tagesabschluss', 'PayedByText'),
(5, 4, 'Monatsabschluss', 'PayedByText');

CREATE TABLE `customer` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `receiptNum` int(11) DEFAULT NULL,
  `text` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

CREATE TABLE `dep` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `version` text NOT NULL,
  `cashregisterid` text NOT NULL,
  `datetime` datetime NOT NULL,
  `text` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

CREATE TABLE `globals` (
  `name` text NOT NULL,
  `value` int(11) DEFAULT NULL,
  `strValue` text
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `groups` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `color` text,
  `button` text,
  `image` text,
  `visible` tinyint(1) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=3 ;

INSERT INTO `groups` (`id`, `name`, `color`, `button`, `image`, `visible`) VALUES
(1, 'auto', '', '', '', 0),
(2, 'Standard', '#008b8b', '', '', 1);

CREATE TABLE `orders` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `receiptId` int(11) NOT NULL,
  `product` int(11) NOT NULL,
  `count` double NOT NULL DEFAULT '1',
  `net` double NOT NULL,
  `gross` double NOT NULL,
  `tax` double NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `product` (`product`),
  KEY `orders_receiptId_index` (`receiptId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

CREATE TABLE `products` (
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

CREATE TABLE `receipts` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `timestamp` datetime NOT NULL,
  `receiptNum` int(11) DEFAULT NULL,
  `payedBy` int(11) NOT NULL DEFAULT '0',
  `gross` double NOT NULL DEFAULT '0',
  `net` double NOT NULL DEFAULT '0',
  `storno` int(11) NOT NULL DEFAULT '0',
  `stornoId` int(11) NOT NULL DEFAULT '0',
  `signature` text,
  PRIMARY KEY (`id`),
  KEY `receipts_stornoId_index` (`stornoId`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

CREATE TABLE `reports` (
  `id` int(11) NOT NULL,
  `receiptNum` int(11) DEFAULT NULL,
  `text` text,
  PRIMARY KEY (`id`),
  KEY `reports_receiptNum_index` (`receiptNum`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE `taxTypes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tax` double DEFAULT NULL,
  `comment` text,
  `taxlocation` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=13 ;

--
-- Dumping data for table `taxTypes`
--

INSERT INTO `taxTypes` (`id`, `tax`, `comment`, `taxlocation`) VALUES
(1, 20, 'Satz-Normal', 'AT'),
(2, 10, 'Satz-Ermaessigt-1', 'AT'),
(3, 13, 'Satz-Ermaessigt-2', 'AT'),
(4, 19, 'Satz-Besonders', 'AT'),
(5, 0, 'Satz-Null', 'AT'),
(6, 19, 'Satz-Normal', 'DE'),
(7, 7, 'Satz-Ermaessigt-1', 'DE'),
(8, 0, 'Satz-Null', 'DE'),
(9, 8, 'Satz-Normal', 'CH'),
(10, 2.5, 'Satz-Ermaessigt-1', 'CH'),
(11, 3.8, 'Satz-Besonders', 'CH'),
(12, 0, 'Satz-Null', 'CH');

ALTER TABLE `orders`
  ADD CONSTRAINT `product` FOREIGN KEY (`product`) REFERENCES `products` (`id`);

ALTER TABLE `products`
  ADD CONSTRAINT `group` FOREIGN KEY (`group`) REFERENCES `groups` (`id`);
SET FOREIGN_KEY_CHECKS=1;
COMMIT;

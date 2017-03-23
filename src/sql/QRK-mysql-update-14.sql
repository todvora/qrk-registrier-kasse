SET FOREIGN_KEY_CHECKS=0;
SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;

CREATE TABLE `journal_backup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `version` text NOT NULL,
  `cashregisterid` text NOT NULL,
  `datetime` datetime NOT NULL,
  `text` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;
INSERT INTO journal_backup SELECT `id`, `version`,`cashregisterid`,`datetime`,`text` FROM `journal`;
DROP TABLE `journal`;

CREATE TABLE `journal` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `version` text NOT NULL,
  `cashregisterid` text NOT NULL,
  `datetime` datetime NOT NULL,
  `text` text,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;
INSERT INTO `journal`(id,version,cashregisterid,datetime,text) VALUES (NULL,'0.15.1222',0,CURRENT_TIMESTAMP, '');
INSERT INTO journal SELECT NULL, `version`,`cashregisterid`,`datetime`,`text` FROM `journal_backup`;
DROP TABLE `journal_backup`;

SET FOREIGN_KEY_CHECKS=1;
COMMIT;

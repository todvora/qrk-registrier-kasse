SET FOREIGN_KEY_CHECKS=0;
SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;

ALTER TABLE `reports` ADD `timestamp` datetime AFTER `receiptNum`;
UPDATE reports SET `timestamp` = (SELECT `timestamp` FROM receipts WHERE reports.receiptNum = receipts.receiptNum);
ALTER TABLE reports CHANGE `timestamp` `timestamp` datetime NOT NULL;
ALTER TABLE `receipts` ADD `infodate` datetime AFTER `timestamp`;
UPDATE receipts SET `infodate` = `timestamp`;
UPDATE `receipts` SET `infodate` = (SELECT `timestamp` FROM `reports` WHERE receipts.receiptNum=reports.receiptNum) WHERE `receiptNum`= (SELECT `receiptNum` FROM `reports` WHERE receipts.receiptNum = reports.receiptNum);
ALTER TABLE receipts CHANGE `infodate` `infodate` datetime NOT NULL;

INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,5,'Startbeleg','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,6,'Kontrollbeleg','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,7,'Sammelbeleg','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,8,'Monatsbeleg','PayedByText');
INSERT INTO `actionTypes`(`id`,`actionId`,`actionText`,`comment`) VALUES (NULL,9,'Schlussbeleg','PayedByText');

SET FOREIGN_KEY_CHECKS=1;
COMMIT;

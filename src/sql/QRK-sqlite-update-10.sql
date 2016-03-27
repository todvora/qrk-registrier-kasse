BEGIN TRANSACTION;

DROP TABLE `taxTypes`;

CREATE TABLE `taxTypes` (
        `id`        INTEGER PRIMARY KEY AUTOINCREMENT,
        `tax`       double,
        `comment`   TEXT,
        `taxlocation`   TEXT
);

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

COMMIT;

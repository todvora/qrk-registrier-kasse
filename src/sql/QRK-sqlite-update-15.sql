BEGIN TRANSACTION;

CREATE TABLE `globals_backup` (
    `name`              text NOT NULL,
    `value`             int(11),
    `strValue`          text
);
INSERT INTO globals_backup SELECT `name`,`value`,`strValue` FROM `globals`;
DROP TABLE `globals`;

CREATE TABLE `globals` (
    `id`                INTEGER PRIMARY KEY AUTOINCREMENT,
    `name`              text NOT NULL,
    `value`             int(11),
    `strValue`          text
);

INSERT INTO globals SELECT NULL,`name`,`value`, `strValue` FROM `globals_backup`;
DROP TABLE `globals_backup`;

COMMIT;

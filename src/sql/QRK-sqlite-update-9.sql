BEGIN TRANSACTION;

CREATE TABLE `customer` (
    `id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
    `receiptNum`	int(11) DEFAULT NULL,
    `text`              text
);

COMMIT;

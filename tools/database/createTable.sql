CREATE TABLE `itl_highscore` (
  `id` INT NOT NULL AUTO_INCREMENT, 
  `name` VARCHAR(80) NOT NULL, 
  `level` VARCHAR(40) NOT NULL, 
  `time` FLOAT(11) NOT NULL,
  `added` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
);

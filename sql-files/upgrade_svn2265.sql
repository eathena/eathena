#Removed the autoincrement field `id`
#its no more needed
#if u get problems with your Control Panel
#its your problem, fix the cp
#... the field is really deprecated
#it only decrases the performance :)

ALTER TABLE `cart_inventory` DROP `id`;
ALTER TABLE `inventory` DROP `id`;

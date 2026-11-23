# Flash driver V6

This driver supports H5 series (dual bank devices, flash sectors of 8kB).
Flash is programmed by 128-bit flash words (quad-words) with ECC, so a flash word can be programmed only once after being erased.
Sectors are erased by bank and sector number, the bank is selected from the sector address.
The instruction cache is invalidated after each program or erase operation, so the contents read afterwards are not stale.
The low level driver code is taken or heavily inspired in the STCube MX HAL drivers from STMicroelectronics.

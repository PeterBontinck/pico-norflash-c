Use this library supports:
- one SPI-norflash chip connected to a normal 1xSPI port of a RP2030/RP2350
- up to 128Mbit flash memory

 Commands
 - **norflash_init** : To be called before the library can be used.
 - **norflash_chip_erase** : Full chip erase.
 - **norflash_write_page** : Blocking page write, lets you write up 256 bytes with 1 functions call.
 - **norflash_read_blocking** : Blocking read to a given buffer.

 - repeated async read : 
    **norflash_start_async_read** :
        Read a data-structure of a given length to a given buffer. 
        This is done for a given number of times.
        Using a given callback function:
            You need to process the data every time a new data-structure is read.
    **norflash_next_async_read** :
        used to request the next data-structure to be read.
    **norflash_abort_async_read** :
        stop early.

        




# Use this library supports:
- One SPI-norflash chip connected to a normal 1xSPI port of a RP2030/RP2350
- Up to 128Mbit flash memory

 # Commands
 ### norflash_init(...) : 
 To be called before the library can be used, to set baudrate and initialize.
 ### norflash_chip_erase() : 
 Full chip erase.
 ### norflash_write_page(...) : 
 Blocking page write, write up 256 bytes with one functions call.
 ### norflash_read_blocking(...) : 
 Blocking read to a given buffer.

 ## repeated async read -> 3 functions:
### norflash_start_async_read(...) :

Read a data-structure of a given length to a given buffer. 
This is done for a given number of times.
Use a given callback function to process the data every time a new data-structure is read.
The buffer will be overwritten every time.

 ### norflash_next_async_read() :
 Used to request the next data-structure to be read.

### norflash_abort_async_read() :
stop early.

        




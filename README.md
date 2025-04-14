
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
 Used to request the next data-structure to be read and to end en cleanup a async read.
 This ***function is mandatory in the callback function*** even if single repetition.


### norflash_abort_async_read() :
stop early.

## console Commands

### norflash_from_console()
Start listening to console instructions to Read, Write, Erase norflash.
This mode is a **blocking loop**.

All \<values> are exchanged as lowercase asci hex-digits:
- \<adr> and \<len> are 3 bytes (6 hex-digits).
- \<data[ ]> is a string of bytes  (2 hex-digits per byte)
- No spaces are allowed in between values or data bytes.

#### R, W, S, O , Q commands (uppercase)
- **R**\<adr>\<len> : Read from address, 
- **W**\<adr>\<data[ ]>**S** : Write up to one page. (don't forget the '**S**' to stop/terminate the data )
- **O**\<adr>\<len> : Obliterate / erase , only full erase supported \<adr>=0 , \<len>=0
- **Q** : quit norflash_from_console loop



#ifndef NORFLASH_H
#define NORFLASH_H

#include "hardware/spi.h"

#ifndef NORFLASH_SPI_PORT
#define NORFLASH_SPI_PORT spi1
#endif

#ifndef NORFLASH_PIN_RX
#define NORFLASH_PIN_RX 12
#endif

#ifndef NORFLASH_PIN_CS
#define NORFLASH_PIN_CS (NORFLASH_PIN_RX + 1)
#endif

#define NORFLASH_PIN_SCK (NORFLASH_PIN_RX + 2)
#define NORFLASH_PIN_TX (NORFLASH_PIN_RX + 3)

#define NORFLASH_PAGE_SIZE 256
#define NORFLASH_SECTOR_SIZE 0x1000
#define NORFLASH_BLOCK_SIZE 0x10000

#define NORFLASH_EMPTY_BYTE_VAL 0xFF
//TODO add dual chip support

typedef void(*norflash_rd_callback_t) (void);

typedef struct norflash_dma{
    uint ch_tx;
    uint ch_rx;
    void *dst_pt;
    uint sizeof_struct;
    norflash_rd_callback_t callback;
    uint reads_left;
    bool first_run;
}norflash_dma_t;

typedef struct norflash
{    
    uint baudrate;
    bool init_ok;
    uint max_addr;
    uint addr_len;
    uint8_t page_buffer[256];
    uint8_t page_len;
    norflash_dma_t dma;
} norflash_t;

/*! \brief  Get the pointer to the first SPI-norflash chip data structure.
 *  \return *norflash_t
*/
norflash_t* norflash_get_pt_singleton_chip1();

/*! \brief  Initaliases the SPI norflash via a 1xSPI and tests the interface by queryin the device characteristics.
 *
 *  \param  baudrate SPI clock freqfentie in Hz
 *
 *  \return enum pico_error_codes
 *
 *  \def    NORFLASH_SPI_PORT , the SPI instance.  Default = spi1
 *  \def    NORFLASH_PIN_RX , the GPIO Pin for SPI MOSI signal. Default = 12
 *  \def    NORFLASH_PIN_CS , the GPIO Pin for SPI Chip Select signal. Default = NORFLASH_PIN_RX + 1 (so 13 if PIN_RX was not changed)
 *
 *  \remark SPI Serial clock signal is always NORFLASH_PIN_RX + 2 (so 14 if PIN_RX was not changed)
 *  \remark SPI MISO signal (TX) is always NORFLASH_PIN_RX + 3 (so 15 if PIN_RX was not changed)
 *  \remark stdio is used to report on success or failure.
 */
int norflash_init(uint baudrate);

/*! \brief Erases the whole chip, while blocking. (takes +-40sec) 

*   \return void
*   \remark stdio is used to report on status.
*   \remark there is no real validation of success.
*/
void norflash_chip_erase();

/*! \brief Write a copy of self.page_buffer[256] to a page in the norflash, while blocking.
*
*   \param  flash_addr 3 byte starting norflash address to write to
*   \param  len number of bytes to write
*   \return enum pico_error_codes
*   \remark The write operation needs to stay in one page.
            The norflash is organized in pages of 256 addresses.
            so : (least significant byte of flash_addr) > len 
*   \remark stdio is used to report on success or failure.
*/
int norflash_write_page(uint flash_addr, uint len);

/*! \brief Read successive bytes form the norflash, while blocking.
*
*   \param  flash_addr 3 byte starting norflash address to write to.
*   \param  out_buffer a pointer to the out buffer.
*   \param  len number of bytes to read.
*
*   \return enum pico_error_codes
*/
int norflash_read_blocking(uint flash_addr, void *out_buffer, uint len);

/*! \brief  Setup and start a repeated dma-read of a data structure type.
*           The next norflash address is incremented after every read.
*           Calls back after first data structure is read from norflash.
*   \note   - Use \b norflash_next_async_read() in callback to initiate the next read.  
*   \note   - Use \b norflash_abort_async_read() to stop the repeated dma-read early.
*   \note   - The spi norflash nCS (Chip Select) remains active until the last data is read or aborted.       
*   \param  flash_addr 3 byte starting norflash address to write to.
*   \param  struct_size sizeof(data_structure_type_to_read)
*   \param  dst Destination buffer to write to. Will be over written on each repetition.
*   \param  irq_callback function called when new data is available.
*   \param  num_strucs_to_read number of repeated dma-reads of the data structure type
*/
int norflash_start_async_read(
    uint flash_addr,
    uint struct_size,
    void *dst,
    norflash_rd_callback_t irq_callback,
    uint num_strucs_to_read
);

/*! \brief  Used in the callback of \b norflash_start_async_read(), to get the next data structure.
*/
void norflash_next_async_read();
/*! \brief  Used after a \b norflash_start_async_read() , 
*   to abort before the given the number of repeated dma-reads .
*/
void norflash_abort_async_read();

void print_byte_buffer(uint8_t *buf, uint len, uint rows);

#endif
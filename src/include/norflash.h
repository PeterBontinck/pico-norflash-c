#ifndef NORFLASH_H
#define NORFLASH_H

#include "hardware/spi.h"
#include "norflash_structs.h"

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

typedef struct norflash norflash_t;

/*! \brief  Initaliases the SPI norflash via a 1xSPI and tests the interface by queryin the device characteristics.
 *
 *  \param  self pointer to a norflash_t struct instance.
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
int norflash_init(norflash_t *self, uint baudrate);

/*! \brief Erases the whole chip, while blocking. (takes +-40sec) 
*   \param  self pointer to a norflash_t struct instance.
*
*   \return void
*   \remark stdio is used to report on status.
*   \remark there is no real validation of success.
*/
void norflash_chip_erase(norflash_t *self);

/*! \brief Write a copy of self.page_buffer[256] to a page in the norflash, while blocking.
*
*   \param  self pointer to a norflash_t struct instance
*   \param  flash_addr 3 byte starting norflash address to write to
*   \param  len number of bytes to write
*   \return enum pico_error_codes
*   \remark The write operation needs to stay in one page.
            The norflash is organized in pages of 256 addresses.
            so : (least significant byte of flash_addr) > len 
*   \remark stdio is used to report on success or failure.
*/
int norflash_write_page(norflash_t *self, uint flash_addr, uint len);

/*! \brief Read successive bytes form the norflash, while blocking.
*
*   \param  self pointer to a norflash_t struct instance.
*   \param  flash_addr 3 byte starting norflash address to write to.
*   \param  out_buffer a pointer to the out buffer.
*   \param  len number of bytes to read.
*
*   \return enum pico_error_codes
*/
int norflash_read_blocking(norflash_t *self, uint flash_addr, void *out_buffer, uint len);

#endif
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
#define NORFLASH_PIN_TX  (NORFLASH_PIN_RX + 3)


typedef struct norflash norflash_t;

int norflash_init(norflash_t *self, uint baudrate);

void norflash_chip_erase(norflash_t *self);

int norflash_write_page(norflash_t *self, uint flashMemAddr, uint len);

int norflash_read(norflash_t *self, uint flashMemAddr, void* out_buffer, uint len);

#endif
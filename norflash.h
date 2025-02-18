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
#define NORFLASH_PIN_TX  (NORFLASH_PIN_RX + 3)


typedef union {
    uint8_t buffer[6];
    struct cmd_addr_buffer {
        uint8_t cmd;
        uint8_t addr[4];
        uint8_t dummy; //used for fast read
    } items;
}cmd_addr_buffer_t;

typedef struct norflash
{    
    uint baudrate;
    bool validate_before_wr;
    bool validate_after_wr;
    bool init_ok;
    uint max_addr;
    uint addr_len;

    cmd_addr_buffer_t cmd_addr;
    uint8_t page_buffer[256];
    uint8_t page_len;

} norflash_t;

int norflash_init(norflash_t *self, uint baudrate);

void norflash_chip_erase(norflash_t *self);

int norflash_write_page(norflash_t *self, uint flashMemAddr, uint len);
int norflash_read(norflash_t *self, uint flashMemAddr, void* out_buffer, uint len);


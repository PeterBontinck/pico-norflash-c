#include <stdio.h>
#include "pico/stdlib.h"
#include "norflash.h"

#define ADRR_TO_WRITE 0x940u

int main()
{

    norflash_t w25q128jv;
    stdio_init_all();
    norflash_init(&w25q128jv, 1000 * 1000);

    // norflash_chip_erase(&w25q128jv);

    uint8_t test_buff[256];

    for (uint i = 0; i <= 127u; i++)
    {
        w25q128jv.page_buffer[i] = (uint8_t)i + 1;
        w25q128jv.page_buffer[255 - i] = (uint8_t)i + 1;
    }

    norflash_write_page(&w25q128jv, ADRR_TO_WRITE, 256);

    norflash_read_blocking(&w25q128jv, ADRR_TO_WRITE, test_buff, 256);

    for (uint i = 0; i <= 255u; i++)
    {
        printf("%02x\t", test_buff[i]);
        if (!((i + 1) % 16))
            printf("\n");
    }

    printf("\n");
}

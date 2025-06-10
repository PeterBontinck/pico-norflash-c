#include <stdio.h>
#include "pico/stdlib.h"
#include "norflash.h"

#define ADRR_TO_WRITE 0x0u



void my_callback(){
    norflash_t *w25q128jv = norflash_get_pt_singleton_chip1();

    print_byte_buffer(w25q128jv->page_buffer,3,16);

    norflash_next_async_read();

}


int main()
{
    stdio_init_all();
    norflash_init(1000 * 1000); //1Mhz

    //norflash_chip_erase();

    uint8_t test_buff[256];

    for (uint i = 0; i <= 127u; i++)
    {
        norflash_get_pt_singleton_chip1()->page_buffer[i] = (uint8_t)i + 1;
        norflash_get_pt_singleton_chip1()->page_buffer[255 - i] = (uint8_t)i + 1;
    }

    //norflash_write_page(ADRR_TO_WRITE, 256);

    norflash_read_blocking(ADRR_TO_WRITE, test_buff, 256);

    print_byte_buffer((uint8_t*)&test_buff, 256, 16);
   
    printf("start DMA \n");
    norflash_start_async_read(ADRR_TO_WRITE,3,norflash_get_pt_singleton_chip1()->page_buffer,my_callback,4);
    sleep_us(30000);
    norflash_start_async_read(ADRR_TO_WRITE,3,norflash_get_pt_singleton_chip1()->page_buffer,my_callback,4);
    sleep_us(200);
    norflash_abort_async_read();

    while(true) 
    {
        tight_loop_contents();
        printf("wait for console\n");
        norflash_from_console();
    }
}

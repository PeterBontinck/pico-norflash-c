#include <stdio.h>
#include "pico/stdlib.h" 
#include "norflash.h"



int main()
{

    norflash_t ;
    stdio_init_all();
    norflash_init(&w25q128jv, 1000*1000);
    
    //norflash_chip_erase(&w25q128jv);

    uint8_t test_buff[256];

    for (uint i = 0; i <= 127u; i++) {
        w25q128jv.page_buffer[i] = (uint8_t)i;
        w25q128jv.page_buffer[255-i] = (uint8_t)i;
    }

    norflash_write_page(&w25q128jv, 0x100u, 256);

    norflash_read(&w25q128jv, 0x100u, test_buff, 256);

    for (uint i = 0; i <= 255u; i++) {
        printf("%x\t", test_buff[i]);
        if(!((i+1)%16)) printf("\n");
    }
        
    
    printf("\n");




}

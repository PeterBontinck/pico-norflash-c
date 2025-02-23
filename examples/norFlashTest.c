#include <stdio.h>
#include "pico/stdlib.h"
#include "norflash.h"




#define ADRR_TO_WRITE 0x1001u



//norflash_t w25q128jv;

void my_callback(){
    norflash_t *w25q128jv = norflash_get_pt_singleton_chip1();

    print_byte_buffer(w25q128jv->page_buffer,sizeof(uint16_t),16);

    norflash_next_async_read();

}


int main()
{
    norflash_t *w25q128jv = norflash_get_pt_singleton_chip1();
    stdio_init_all();
    norflash_init(w25q128jv, 1000 * 10);

    gpio_set_dir(DEBUG_PIN_A , GPIO_OUT);
    
    gpio_set_function(DEBUG_PIN_A, GPIO_FUNC_SIO);


    //norflash_chip_erase(&w25q128jv);

    uint8_t test_buff[256];

    for (uint i = 0; i <= 127u; i++)
    {
        w25q128jv->page_buffer[i] = (uint8_t)i + 1;
        w25q128jv->page_buffer[255 - i] = (uint8_t)i + 1;
    }

    norflash_write_page(w25q128jv, ADRR_TO_WRITE, 256);

    norflash_read_blocking(w25q128jv, ADRR_TO_WRITE, test_buff, 256);

    print_byte_buffer(&test_buff,256,16);
   

    printf("start DMA \n");
    norflash_start_async_read(ADRR_TO_WRITE,sizeof(uint16_t),w25q128jv->page_buffer,my_callback,3);
    printf("end DMA \n");


    while(true) 
    {
        tight_loop_contents();
    }
}

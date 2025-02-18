#ifdef NDEBUG           /* required by ANSI standard */
# define IFDBUG(__f) ((void)0)
#else
# define IFDBUG(__f) __f
#endif

#include <stdio.h>
#include "norflash.h"
#include "norflash_cmd.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "string.h"
//TODO, improuve for prd Write validation before / after
//TODO, support 4bit adderssing support for  256Mbit chips


//to calculate the max adress : (result<<4) - 1 
uint cmd9Fh_to_max_addr(uint8_t cmd9Fh_density){   
    switch (cmd9Fh_density)
    {
    case 0x13: return   0x7FFFFu; //4Mbit 
    case 0x14: return   0xFFFFFu; //8Mbit
    case 0x15: return  0x10FFFFu; //16Mbit
    case 0x16: return  0x3FFFFFu; //32Mbit
    case 0x17: return  0x7FFFFFu; //64Mbit
    case 0x18: return  0xFFFFFFu; //128Mbit
    case 0x19: return 0x1FFFFFFu; //256Mbit

    default:
        return 0;
    }

}



static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(NORFLASH_PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(NORFLASH_PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

static void norflash_get(norflash_t *self, uint8_t *out_buff, size_t out_len){
    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, self->cmd_addr.buffer,1);
    spi_read_blocking(NORFLASH_SPI_PORT,0,out_buff,out_len);
    cs_deselect();
}

static void norflash_send_cmd(norflash_t *self){
    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, self->cmd_addr.buffer, 1);
    cs_deselect();
}

static int norflash_validate(norflash_t *self, uint8_t *source_pt, size_t len , bool inc, bool exhaustive){
    uint inc_val = (inc)? 1 : 0; 
    uint8_t *validation_pt = source_pt;
    int exception = PICO_OK;
    
    self->cmd_addr.items.cmd = NORFLASH_CMD_FAST_READ;

    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, self->cmd_addr.buffer ,1+3+1); //1 byte cmd + 3 byte addr + 1 byte dummy for fast read
    for(uint i=0; i<len; i++){
        uint8_t temp;
        spi_read_blocking(NORFLASH_SPI_PORT, 0, &temp, 1);
        if (temp != *validation_pt){
            exception = PICO_ERROR_GENERIC;
            printf("Validation Error adress: %02x %02x %02x + %02x is %02x. %02x was expected\n", 
                self->cmd_addr.items.addr[0],
                self->cmd_addr.items.addr[1],
                self->cmd_addr.items.addr[2],
                i,
                temp,
                *validation_pt
                );
        
            if (!exhaustive) break;       
        }
        validation_pt += inc_val;
    }
    cs_deselect();

    return exception;
}


void write_enable(norflash_t *self){
    self->cmd_addr.items.cmd = NORFLASH_CMD_WR_EN;
    norflash_send_cmd(self);
}

static void wait_for_ready(norflash_t *self){

    bool busy = true;
    
    while (busy) {
        self->cmd_addr.items.cmd = NORFLASH_CMD_RD_STATUS1;
        uint8_t status;
        norflash_get(self, &status, 1);
       
        busy = (bool)(status & (1u << NORFLASH_BUSSY_BIT));
       }
   

}

static void encode_adress(norflash_t *self, uint flashMemAddr){

    assert(flashMemAddr<= self->max_addr);
    
    self->cmd_addr.items.addr[0] = (flashMemAddr&((0xFF0000u)))>>16;
    self->cmd_addr.items.addr[1] = (flashMemAddr&((0x00FF00u)))>>8;
    self->cmd_addr.items.addr[2] = flashMemAddr&((0x0000FFu));
    self->cmd_addr.items.addr[3] = 0; 
    }



void norflash_chip_erase(norflash_t *self){
    assert(self->init_ok);
    
    write_enable(self);

    self->cmd_addr.items.cmd = NORFLASH_CMD_CHIP_ERASE;
    norflash_send_cmd(self);

    printf("Erasing  chip (+-40sec) .... ");
    wait_for_ready(self);
    printf(" READY !\n");

}



int norflash_init(norflash_t *self,  uint baudrate){

    memset((void*) self,0, sizeof(self));
    self->baudrate = baudrate;
    self->validate_before_wr = true;
    self->validate_after_wr = true;


    spi_init(NORFLASH_SPI_PORT, baudrate);   
    gpio_set_function(NORFLASH_PIN_RX, GPIO_FUNC_SPI);
    gpio_set_function(NORFLASH_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(NORFLASH_PIN_TX, GPIO_FUNC_SPI);

    gpio_set_function(NORFLASH_PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_dir(NORFLASH_PIN_CS, GPIO_OUT);

    self->cmd_addr.items.cmd = NORFLASH_CMD_JEDEC_ID;
       
    norflash_get(self,self->page_buffer,3);

    uint8_t mfg_id =  self->page_buffer[0];
    self->max_addr = cmd9Fh_to_max_addr(self->page_buffer[2]);
    self->addr_len = 3;

    if (self->max_addr == 0 || mfg_id == 0 ){
        printf("failed to initialise flash, return of cmd 9Fh : %02x,%02x,%02x\n",self->page_buffer[0],self->page_buffer[1],self->page_buffer[2] );
        assert(false);
        return PICO_ERROR_GENERIC;
    }

    printf("Flash initialised, %d Bytes NOR-Flash found of Manufacturer-ID = %02x(hex)\n", self->max_addr+1, mfg_id );


    if (self->max_addr > 0xFFFFFFu ){
        printf("4byte adderssing is not supported , only 16Mb (128Mbit) is usable \n" );
        self->max_addr = 0xFFFFFFu;
        self->addr_len = 3;
    }

    self->init_ok = true;
    return PICO_OK;

}

int norflash_read(norflash_t *self, uint flashMemAddr, void* out_buffer, uint len){
    self->cmd_addr.items.cmd = NORFLASH_CMD_FAST_READ;
    encode_adress(self, flashMemAddr);
    
    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, self->cmd_addr.buffer ,1+3+1); //1 byte cmd + 3 byte addr + 1 byte dummy
    spi_read_blocking(NORFLASH_SPI_PORT, 0, out_buffer, len);
    cs_deselect();

}


int norflash_write_page(norflash_t *self, uint flashMemAddr, uint len){

//TODO intput validation, len
    encode_adress(self, flashMemAddr);

    uint8_t empty = 0xFF;
    
    if (0 > norflash_validate(self,&empty,len,false,true)){
        printf("Page not empty, aborting write operation\n");
        return PICO_ERROR_GENERIC;
    }

    write_enable(self);

    self->cmd_addr.items.cmd = NORFLASH_CMD_PAGE_PROG;  
    

    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, self->cmd_addr.buffer, 1+3); //1 byte cmd + 3 byte addr
    spi_write_blocking(NORFLASH_SPI_PORT, self->page_buffer, len);
    cs_deselect();

    wait_for_ready(self);

    if (0 > norflash_validate(self,self->page_buffer,len,true,true)){
        printf("ERROR Write unsuccesfull!, data corrupted\n");
        return PICO_ERROR_GENERIC;
    }

    printf("%d bytes written to %02x %02x %02x \n", len,self->cmd_addr.items.addr[0],self->cmd_addr.items.addr[1],self->cmd_addr.items.addr[2]);

    return PICO_OK;

}
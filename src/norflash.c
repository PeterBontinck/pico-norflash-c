#ifdef NDEBUG           /* required by ANSI standard */
# define IFDBUG(__f) ((void)0)
#else
# define IFDBUG(__f) __f
#endif

#include <stdio.h>
#include "include/norflash.h"
#include "norflash_cmd.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "string.h"

//TODO, add DMA read support.

//TODO, support 4bit adderssing support for  256Mbit chips


//Global Variables for singletons
norflash_t chip1_singleton;


typedef struct {
    uint8_t buf[5];
  } cmd_buf_t;

norflash_t* norflash_get_pt_singleton_chip1(){
    return &chip1_singleton;
}

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


static inline cmd_buf_t encode_cmd_addr(norflash_t *self, uint8_t cmd, uint flashMemAddr){

    assert(flashMemAddr<= self->max_addr);

    cmd_buf_t cmd_addr = {{0}};
    
    cmd_addr.buf[0] = cmd;
    cmd_addr.buf[1] = (flashMemAddr&0xFF0000u)>>16;
    cmd_addr.buf[2] = (flashMemAddr&0x00FF00u)>>8;
    cmd_addr.buf[3] = flashMemAddr&0x0000FFu ;
    
    return cmd_addr;
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

static void norflash_get(norflash_t *self, uint8_t cmd,  uint8_t *out_buff, size_t out_len){
    
    //uint8_t *cmd_pt = cmd; 
    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, &cmd,1);
    spi_read_blocking(NORFLASH_SPI_PORT,0,out_buff,out_len);
    cs_deselect();
}

static void norflash_send_cmd(norflash_t *self, uint8_t cmd){
    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, &cmd, 1);
    cs_deselect();
}

static int norflash_validate(norflash_t *self, uint start_add,  uint8_t *source_pt, size_t len , bool inc, bool exhaustive){
    uint inc_val = (inc)? 1 : 0; 
    uint8_t *validation_pt = source_pt;
    int exception = PICO_OK;
    
    cmd_buf_t cmd_add = encode_cmd_addr(self, NORFLASH_CMD_FAST_READ, start_add);

    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, cmd_add.buf, NORFLASH_FAST_READ_CMDBUF_LEN); //1 byte cmd + 3 byte addr + 1 byte dummy for fast read
    for(uint i=0; i<len; i++){
        uint8_t temp;
        spi_read_blocking(NORFLASH_SPI_PORT, 0, &temp, 1);
        if (temp != *validation_pt){
            exception = PICO_ERROR_GENERIC;
            printf("Validation Error data in: #%02x%02x%02x + #%02x is #%02x. #%02x was expected\n", 
                cmd_add.buf[1],
                cmd_add.buf[2],
                cmd_add.buf[3],
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
    norflash_send_cmd(self,NORFLASH_CMD_WR_EN);
}

static void wait_for_ready(norflash_t *self){
    uint8_t status;
    do{
        norflash_get(self,NORFLASH_CMD_RD_STATUS1, &status, 1);
     }while (status & (1u << NORFLASH_BUSSY_BIT));
}

void norflash_chip_erase(norflash_t *self){
    assert(self->init_ok);
    
    write_enable(self);

    norflash_send_cmd(self,NORFLASH_CMD_CHIP_ERASE);

    printf("Erasing  chip (+-40sec) .... ");
    wait_for_ready(self);
    printf(" READY !\n");
}

void print_byte_buffer(uint8_t *buf, uint len, uint rows){
    for (uint i = 0; i < len ; i++)
    {   
        printf("%02x\t", *(buf+i));
        if (!((i + 1) % rows))
            printf("\n");
    }

    printf("\n");
}

int norflash_init(norflash_t *self,  uint baudrate){

    memset((void*) self,0, sizeof(self));
    self->baudrate = baudrate;

    spi_init(NORFLASH_SPI_PORT, baudrate);   
    gpio_set_function(NORFLASH_PIN_RX, GPIO_FUNC_SPI);
    gpio_set_function(NORFLASH_PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(NORFLASH_PIN_TX, GPIO_FUNC_SPI);

    gpio_set_function(NORFLASH_PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_dir(NORFLASH_PIN_CS, GPIO_OUT);
     
    struct device_s {
        uint8_t mfg_id;
        uint8_t type;
        uint8_t density;
    } device;

    norflash_get(self,NORFLASH_CMD_JEDEC_ID, &device.mfg_id, sizeof(device));
 
    self->max_addr = cmd9Fh_to_max_addr(device.density);

    if (self->max_addr == 0 || device.mfg_id == 0 ){
        printf("failed to initialise Norflash, return of cmd 9Fh : #%02x,#%02x,#%02x\n",device.mfg_id,device.type,device.density );
        assert(false);
        return PICO_ERROR_GENERIC;
    }

    printf("Norflash initialised, %d Bytes chip found of Manufacturer-ID = #%02x\n", self->max_addr+1, device.mfg_id );


    if (self->max_addr > 0xFFFFFFu ){
        printf("4byte addressing is not supported , only 16Mb (128Mbit) is usable \n" );
        self->max_addr = 0xFFFFFFu;
        self->addr_len = 3;
    }

    self->init_ok = true;
    return PICO_OK;
}

int norflash_read_blocking(norflash_t *self, uint flash_addr, void* out_buffer, uint len){
    assert(self->init_ok);
    
    const cmd_buf_t cmd_addr = encode_cmd_addr(self, NORFLASH_CMD_FAST_READ, flash_addr);
    
    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, cmd_addr.buf, NORFLASH_FAST_READ_CMDBUF_LEN); 
    spi_read_blocking(NORFLASH_SPI_PORT, 0, out_buffer, len);
    cs_deselect();
}


int norflash_write_page(norflash_t *self, uint flash_addr, uint len){
    assert(self->init_ok);

    uint in_page_addr = flash_addr & 0xFFu;

    if (((0xFFu - in_page_addr) < len-1)){ // len -1 for zero index of adr
        printf("ERROR : Page will overflow, aborting write operation! in_page_addr: #%02x , len %d \n",in_page_addr, len);
        return PICO_ERROR_GENERIC;
    }
   
    uint8_t empty = 0xFF;
    if (0 > norflash_validate(self,flash_addr,&empty,len,false,true)){
        printf("ERROR : Page not empty, aborting write operation\n");
        return PICO_ERROR_GENERIC;
    }

    const cmd_buf_t cmd_addr = encode_cmd_addr(self, NORFLASH_CMD_PAGE_PROG, flash_addr);
    
    write_enable(self);
 
    cs_select();
    spi_write_blocking(NORFLASH_SPI_PORT, cmd_addr.buf, 1+3); //1 byte cmd + 3 byte addr
    spi_write_blocking(NORFLASH_SPI_PORT, self->page_buffer, len);
    cs_deselect();

    wait_for_ready(self);

    if (0 > norflash_validate(self, flash_addr, self->page_buffer, len, true, true)){
        printf("ERROR : Write unsuccessful!, data corrupted\n");
        return PICO_ERROR_GENERIC;
    }

    printf("%d bytes written to #%02x%02x%02x \n", len,cmd_addr.buf[1],cmd_addr.buf[2],cmd_addr.buf[3]);

    return PICO_OK;
}


int norflash_start_async_read(
    uint flash_addr,
    uint struct_size,
    void *dst,
    norflash_rd_callback_t irq_callback,
    uint num_strucs_to_read
){
    gpio_put(DEBUG_PIN_A, 1);
    
    norflash_t *self = &chip1_singleton;

    self->dma.sizeof_struct = struct_size;
    self->dma.next_dst_pt = dst;
    self->dma.callback = irq_callback;
    self->dma.reads_left = num_strucs_to_read;

    self->dma.ch_tx = dma_claim_unused_channel(true);
    self->dma.ch_rx = dma_claim_unused_channel(true);

    dma_channel_config c = dma_channel_get_default_config(self->dma.ch_tx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(spi_default, true));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false); 
    dma_channel_configure(self->dma.ch_tx, &c,
                          &spi_get_hw(spi_default)->dr, // write address
                          NULL, // read address
                          self->dma.sizeof_struct, // element count (each element is of size transfer_data_size)
                          false); // don't start yet

    //setup dma to wait for TX to finish. We wait for dummy bytes to be received in sync to TX.

    c = dma_channel_get_default_config(self->dma.ch_rx);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
    channel_config_set_dreq(&c, spi_get_dreq(NORFLASH_SPI_PORT, false));
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, false); //during cmd add dummy phase we write to dummy

    int8_t dummy;
    dma_channel_configure(self->dma.ch_rx, &c,
        &dummy, // write address
        &spi_get_hw(NORFLASH_SPI_PORT)->dr, // read address
        NORFLASH_FAST_READ_CMDBUF_LEN, 
        false); // don't start yet   

    dma_channel_set_irq0_enabled(self->dma.ch_rx, true);
    irq_set_exclusive_handler(DMA_IRQ_0, norflash_next_async_read);
    irq_set_enabled(DMA_IRQ_0, true);

    cmd_buf_t cmd_addr = encode_cmd_addr(self, NORFLASH_CMD_FAST_READ, flash_addr);
    
    // sure the RX register is empty and the SPI bus is not busy.
    while (spi_is_readable(NORFLASH_SPI_PORT))
        (void)spi_get_hw(NORFLASH_SPI_PORT)->dr;

    while (spi_is_busy(NORFLASH_SPI_PORT)) 
        tight_loop_contents();
    
    while (spi_is_readable(NORFLASH_SPI_PORT))
        (void)spi_get_hw(NORFLASH_SPI_PORT)->dr;

    // Lets go... send the Cmd + Adr + Dummy for fast read. 
    
    dma_channel_start(self->dma.ch_rx);

    for (size_t i = 0; i < NORFLASH_FAST_READ_CMDBUF_LEN; ++i) {
         spi_get_hw(NORFLASH_SPI_PORT)->dr = (uint32_t)cmd_addr.buf[i];
    }
    
    // norflash_next_async_read() will be called when so setup DMA for the first Read
    return PICO_OK;
}

void norflash_next_async_read(){
    gpio_put(DEBUG_PIN_A, 1);

    norflash_t *self = &chip1_singleton;

    if(self->dma.first_run){
        dma_channel_config c = dma_channel_get_default_config(self->dma.ch_rx);
        channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
        channel_config_set_dreq(&c, spi_get_dreq(NORFLASH_SPI_PORT, false));
        channel_config_set_read_increment(&c, false);
        channel_config_set_write_increment(&c, true);

        dma_channel_configure(self->dma.ch_rx, &c,
            self->dma.next_dst_pt, // write address
            &spi_get_hw(NORFLASH_SPI_PORT)->dr, // read address
            self->dma.sizeof_struct, 
            false); // don't start yet
        
        irq_set_exclusive_handler(DMA_IRQ_0, self->dma.callback);
        irq_set_enabled(DMA_IRQ_0, true);
        self->dma.first_run = false;
    }

    if(self->dma.reads_left == 0){
        irq_set_enabled(DMA_IRQ_0, false);
        irq_remove_handler(DMA_IRQ_0, norflash_next_async_read);
        dma_channel_cleanup(self->dma.ch_rx);
        memset(&self->dma, 0, sizeof(self->dma));
        printf("dma cleaned up");
    }
    else{
        self->dma.reads_left--;
        dma_channel_set_write_addr(self->dma.ch_rx, self->dma.next_dst_pt,false);
        dma_channel_set_trans_count(self->dma.ch_rx, self->dma.sizeof_struct, false);
        dma_channel_set_trans_count(self->dma.ch_tx, self->dma.sizeof_struct, false);
        
        //self->dma.next_dst_pt += self->dma.sizeof_struct;
      
        // start them exactly simultaneously to avoid races (in extreme cases the FIFO could overflow)
        dma_start_channel_mask((1u << self->dma.ch_tx) | (1u << self->dma.ch_rx));
    }

    gpio_put(DEBUG_PIN_A, 0);
    // Clear the interrupt request.
    dma_hw->ints0 = 1u << self->dma.ch_rx;

}
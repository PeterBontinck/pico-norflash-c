
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

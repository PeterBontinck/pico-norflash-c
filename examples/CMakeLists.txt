
add_executable(NandFlashTest
    norflashTest.c
)


target_link_libraries(NandFlashTest
    pico_stdlib
    spi_norflash
)


pico_set_program_name(NandFlashTest "NandFlashTest")
pico_set_program_version(NandFlashTest "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(NandFlashTest 1)
pico_enable_stdio_usb(NandFlashTest 0)


pico_add_extra_outputs(NandFlashTest)
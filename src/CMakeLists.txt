add_library(spi_norflash INTERFACE)

target_include_directories(spi_norflash INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(spi_norflash INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/norflash.c
)

target_link_libraries(spi_norflash INTERFACE
    hardware_spi    
    hardware_dma
)
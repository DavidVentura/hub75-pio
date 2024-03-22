cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

project(my_project CXX C ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(main
    src/hub75.c
)

pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/src/hub75.pio)
pico_enable_stdio_usb(main 0)
pico_enable_stdio_uart(main 1)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main
	pico_stdlib
	hardware_pio)

# do not create map/bin/hex/uf2 file in addition to ELF.
# pico_add_extra_outputs(main)

cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico-sdk/pico_sdk_init.cmake)

project(my_project CXX C ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
add_executable(main
    src/main.c
)

pico_generate_pio_header(main ${CMAKE_CURRENT_LIST_DIR}/hub75.pio)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(main
	pico_stdlib
	hardware_pio)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(main)

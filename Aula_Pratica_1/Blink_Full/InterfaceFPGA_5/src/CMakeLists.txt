add_executable(InterfaceFPGA_5
    main.c
    #io_rgb_tasks.c
)

# Corrige a saída para build/ em vez de build/src/
set_target_properties(InterfaceFPGA_5 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_sources(InterfaceFPGA_5 PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(InterfaceFPGA_5 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../inc

)

target_link_libraries(InterfaceFPGA_5 
    pico_stdlib 
    FreeRTOS-Kernel
    hardware_adc
    hardware_i2c
    hardware_gpio
)

pico_add_extra_outputs(InterfaceFPGA_5)

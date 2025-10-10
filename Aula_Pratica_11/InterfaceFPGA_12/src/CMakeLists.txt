add_executable(InterfaceFPGA_12
    main.c
    i2c_config.c
    oled_display.c
    ssd1306_text.c
    tarefa_word5.c
    tarefa_display_word6.c
    numeros_display.c
    oled_animacoes.c
    numeros_medios.c
    digitos_medios_utils.c
    oled_context.c
    tarefa_led_verde.c
    tarefa_joystick_eventos.c
    joystick_setup.c
    joystick_x_dma.c
    joystick_y_dma.c 
    dma_handlers.c 
    tarefa_word_to_gpio.c
)

# Corrige a saída para build/ em vez de build/src/
set_target_properties(InterfaceFPGA_12 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_sources(InterfaceFPGA_12 PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(InterfaceFPGA_12 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../inc

)

target_link_libraries(InterfaceFPGA_12 
    pico_stdlib 
    FreeRTOS-Kernel
    hardware_adc
    hardware_i2c
    hardware_gpio
    hardware_dma 
    hardware_pwm 
    hardware_pio 
    hardware_clocks
)

pico_add_extra_outputs(InterfaceFPGA_12)

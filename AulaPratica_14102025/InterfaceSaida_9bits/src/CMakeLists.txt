add_executable(InterfaceSaida_9bits
    main.c
    i2c_config.c
    oled_display.c
    ssd1306_text.c
    tarefa_botao_a.c
    tarefa_botao_b.c
    tarefa_botao_joy.c
    tarefa_word_9.c
    tarefa_display_word_9.c
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
    digitos_grandes_utils.c 
    numeros_grandes.c 

)

# Corrige a saída para build/ em vez de build/src/
set_target_properties(InterfaceSaida_9bits PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_sources(InterfaceSaida_9bits PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(InterfaceSaida_9bits PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../inc

)

target_link_libraries(InterfaceSaida_9bits 
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

pico_add_extra_outputs(InterfaceSaida_9bits)

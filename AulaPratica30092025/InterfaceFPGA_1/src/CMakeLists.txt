add_executable(InterfaceFPGA_1
    main.c
    i2c_config.c
    oled_display.c
    ssd1306_text.c
    tarefa_botao_a.c
    tarefa_botao_b.c
    tarefa_botao_joy.c
    tarefa_word6.c
    tarefa_display_word6.c
    numeros_display.c
    oled_animacoes.c
    numeros_medios.c
    digitos_medios_utils.c
    oled_context.c
    tarefa_led_verde.c
)

# Corrige a saída para build/ em vez de build/src/
set_target_properties(InterfaceFPGA_1 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_sources(InterfaceFPGA_1 PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(InterfaceFPGA_1 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../inc

)

target_link_libraries(InterfaceFPGA_1 
    pico_stdlib 
    FreeRTOS-Kernel
    hardware_adc
    hardware_i2c
    hardware_gpio
)

pico_add_extra_outputs(InterfaceFPGA_1)

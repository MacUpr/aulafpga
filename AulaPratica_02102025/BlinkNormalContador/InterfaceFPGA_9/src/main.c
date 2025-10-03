// main.c
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// ==== OLED ====
#include "oled_display.h"       // oled_init(&oled)
#include "oled_context.h"       // extern ssd1306_t oled; extern SemaphoreHandle_t mutex_oled;

// ==== Tarefas ====
#include "tarefa_word6.h"           // criar_tarefa_word6(...)
#include "tarefa_display_word6.h"   // criar_tarefa_display_word6(...)

#include "tarefa_botao_a.h"         // criar_tarefa_botao_a(...)
#include "tarefa_botao_b.h"         // criar_tarefa_botao_b(...)
#include "tarefa_botao_joy.h"       // criar_tarefa_botao_joy(...)

#include "tarefa_led_verde.h"       // criar_tarefa_led_verde(...)

// ==== Núcleos (RP2040) ====
#define CORE0_MASK  ( (UBaseType_t)(1u << 0) )
#define CORE1_MASK  ( (UBaseType_t)(1u << 1) )

// ==== Prioridades ====
// Maior -> menor: WORD6 > JOY > A > B >= OLED ~= LED_VERDE
#define PRIO_WORD6      (tskIDLE_PRIORITY + 4)
#define PRIO_BTN_JOY    (tskIDLE_PRIORITY + 3)
#define PRIO_BTN_A      (tskIDLE_PRIORITY + 2)
#define PRIO_BTN_B      (tskIDLE_PRIORITY + 1)
#define PRIO_OLED       (tskIDLE_PRIORITY + 1)
#define PRIO_LED_VERDE  (tskIDLE_PRIORITY + 1)

// ==== Saídas controladas pelos botões ====
#define GPIO_OUT_A   16
#define GPIO_OUT_B   17
#define GPIO_OUT_JOY 28

static inline void gpio_as_output_high(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 1); // alto = não pressionado
}

int main(void) {
    // STDIO USB
    stdio_init_all();
    //while (!stdio_usb_connected());

    printf("=== INICIANDO SISTEMA: WORD6 + OLED + BOTOES + LED_VERDE ===\n");

    // OLED
    if (!oled_init(&oled)) {
        printf("Falha ao inicializar OLED!\n");
        while (true) { tight_loop_contents(); }
    }
    mutex_oled = xSemaphoreCreateMutex();
    configASSERT(mutex_oled != NULL);

    // Saídas controladas pelos botões (estado inicial = alto)
    gpio_as_output_high(GPIO_OUT_A);
    gpio_as_output_high(GPIO_OUT_B);
    gpio_as_output_high(GPIO_OUT_JOY);

    // Tarefa de leitura dos 6 bits -> núcleo 0, prioridade máxima
    // use_pullup = true (estável quando FPGA estiver em tri-state)
    criar_tarefa_word6(PRIO_WORD6, CORE0_MASK, true);

    // Tarefa de exibição no OLED -> núcleo 1
    criar_tarefa_display_word6(PRIO_OLED, CORE1_MASK);

    // Tarefas dos botões -> núcleo 1, com prioridades JOY > A > B
    criar_tarefa_botao_b  (PRIO_BTN_B,   CORE1_MASK);
    criar_tarefa_botao_a  (PRIO_BTN_A,   CORE1_MASK);
    criar_tarefa_botao_joy(PRIO_BTN_JOY, CORE1_MASK);

    // Tarefa LED: piscar em VERDE continuamente -> núcleo 1
    criar_tarefa_led_verde(PRIO_LED_VERDE, CORE1_MASK);

    // Scheduler
    vTaskStartScheduler();

    // Nunca deve chegar aqui
    while (true) { tight_loop_contents(); }
}

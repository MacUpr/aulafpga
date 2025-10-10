// main.c
#include <stdio.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// ==== OLED ====
#include "oled_display.h"
#include "oled_context.h"

// ==== Tarefas ====
#include "tarefa_word6.h"
#include "tarefa_display_word6.h"
#include "tarefa_botao_a.h"
#include "tarefa_botao_b.h"
#include "tarefa_botao_joy.h"
#include "tarefa_led_verde.h"
#include "tarefa_joystick_eventos.h"  // eventos extremos (DMA->notify)

// ==== DMA IRQ handler ====
#include "hardware/irq.h"
#include "dma_handlers.h"             // dma_handler_joy_xy

// ==== Núcleos (RP2040) ====
#define CORE0_MASK  ((UBaseType_t)(1u << 0))
#define CORE1_MASK  ((UBaseType_t)(1u << 1))

// ==== Prioridades ====
// Maior -> menor: WORD6 > JOY EVT > JOY BTN > A > B >= OLED ~= LED_VERDE
#define PRIO_WORD6      (tskIDLE_PRIORITY + 4)
#define PRIO_JOY_EVT    (tskIDLE_PRIORITY + 3)
#define PRIO_BTN_JOY    (tskIDLE_PRIORITY + 3)
#define PRIO_BTN_A      (tskIDLE_PRIORITY + 2)
#define PRIO_BTN_B      (tskIDLE_PRIORITY + 1)
#define PRIO_OLED       (tskIDLE_PRIORITY + 1)
#define PRIO_LED_VERDE  (tskIDLE_PRIORITY + 1)

int main(void) {
    stdio_init_all();
    // while (!stdio_usb_connected()) { tight_loop_contents(); }

    printf("=== INICIANDO SISTEMA: WORD6 + OLED + BOTOES + LED_VERDE ===\n");

    // OLED
    if (!oled_init(&oled)) {
        printf("Falha ao inicializar OLED!\n");
        while (true) { tight_loop_contents(); }
    }
    mutex_oled = xSemaphoreCreateMutex();
    configASSERT(mutex_oled != NULL);

    // DMA IRQ1: demultiplexa X/Y e notifica a tarefa de eventos
    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler_joy_xy);
    irq_set_enabled(DMA_IRQ_1, true);

    // Tarefa dos 6 bits -> núcleo 0 (escreve nos pinos)
    criar_tarefa_word6(PRIO_WORD6, CORE0_MASK, true);

    // OLED -> núcleo 1
    criar_tarefa_display_word6(PRIO_OLED, CORE1_MASK);

    // Tarefas dos botões (núcleo 1)
    //criar_tarefa_botao_b  (PRIO_BTN_B,   CORE1_MASK);
    //criar_tarefa_botao_a  (PRIO_BTN_A,   CORE1_MASK);
    //criar_tarefa_botao_joy(PRIO_BTN_JOY, CORE1_MASK);

    // Eventos do joystick (extremos com histerese) -> núcleo 1
    criar_tarefa_joystick_eventos(PRIO_JOY_EVT, CORE1_MASK);

    // LED verde (núcleo 1)
    criar_tarefa_led_verde(PRIO_LED_VERDE, CORE1_MASK);

    vTaskStartScheduler();

    // Nunca deve chegar aqui
    while (true) { tight_loop_contents(); }
}

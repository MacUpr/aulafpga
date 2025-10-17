// main.c — WORD9 + BCD8 + DISPLAY_DUPLO + LED_VERDE

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

// ==== Tarefas usadas ====
#include "tarefa_word_9.h"            // criar_tarefa_word9(...)
#include "tarefa_bcd8.h"              // criar_tarefa_bcd8(...)
#include "tarefa_display_duplo_bcd.h" // criar_tarefa_display_duplo_bcd(...)
#include "tarefa_led_verde.h"         // criar_tarefa_led_verde(...)

// ==== Núcleos (RP2040) ====
#define CORE0_MASK  ((UBaseType_t)(1u << 0))
#define CORE1_MASK  ((UBaseType_t)(1u << 1))

// ==== Prioridades (maior é mais alta) ====
// Leitura > BCD > Display ~= LED_VERDE
#define PRIO_WORD9      (tskIDLE_PRIORITY + 4)
#define PRIO_BCD8       (tskIDLE_PRIORITY + 3)
#define PRIO_DISPLAY    (tskIDLE_PRIORITY + 2)
#define PRIO_LED_VERDE  (tskIDLE_PRIORITY + 2)

// ==== Objetos globais do OLED (definidos em outro TU) ====
extern SemaphoreHandle_t mutex_oled;
extern ssd1306_t oled;

int main(void) {
    stdio_init_all();
    // Opcional: aguardar USB para logs
    // while (!stdio_usb_connected()) { tight_loop_contents(); }

    printf("=== INICIANDO: WORD9 + BCD8 + DISPLAY_DUPLO + LED_VERDE ===\n");

    // Inicializa OLED
    if (!oled_init(&oled)) {
        printf("Falha ao inicializar OLED!\n");
        while (true) { tight_loop_contents(); }
    }
    mutex_oled = xSemaphoreCreateMutex();
    configASSERT(mutex_oled != NULL);

    // ----- Tarefas -----
    // 1) Leitura de 9 bits (B0..B8) — com pull-up interno habilitado
    criar_tarefa_word9(PRIO_WORD9, CORE0_MASK, /*use_pullup=*/true);

    // 2) Conversão dos 8 LSB (B0..B7) para BCD (dezena/unidade)
    criar_tarefa_bcd8(PRIO_BCD8, CORE0_MASK);

    // 3) Display: dois dígitos grandes centralizados (consome g_bcd_* )
    criar_tarefa_display_duplo_bcd(PRIO_DISPLAY, CORE1_MASK);

    // 4) LED verde (núcleo 1)
    criar_tarefa_led_verde(PRIO_LED_VERDE, CORE1_MASK);

    // Scheduler
    vTaskStartScheduler();

    // Nunca deve chegar aqui
    while (true) { tight_loop_contents(); }
}

// main.c
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// ==== OLED ====
#include "oled_display.h"     // oled_init(&oled)
#include "oled_context.h"     // extern ssd1306_t oled; extern SemaphoreHandle_t mutex_oled;

// ==== Tarefas ====
#include "tarefa_word6.h"           // criar_tarefa_word6(...)
#include "tarefa_display_word6.h"   // criar_tarefa_display_word6(...)
#include "tarefa_led_rgb.h"         // criar_tarefa_led_verde(...)

// ==== Núcleos (RP2040) ====
#define CORE0_MASK  ( (UBaseType_t)(1u << 0) )
#define CORE1_MASK  ( (UBaseType_t)(1u << 1) )

// ==== Prioridades ====
#define PRIO_WORD6       (tskIDLE_PRIORITY + 3)   // leitura da palavra de 6 bits
#define PRIO_OLED        (tskIDLE_PRIORITY + 2)   // exibição no OLED
#define PRIO_LED_VERDE   (tskIDLE_PRIORITY + 1)   // seta LED e se suspende

int main(void) {
    // STDIO USB
    stdio_init_all();
    // while (!stdio_usb_connected());

    printf("=== INICIANDO: WORD6 + OLED + LED_RGB ===\n");

    // OLED
    if (!oled_init(&oled)) {
        printf("Falha ao inicializar OLED!\n");
        while (true) { tight_loop_contents(); }
    }
    mutex_oled = xSemaphoreCreateMutex();
    configASSERT(mutex_oled != NULL);

    // LED RGB fixo em VERDE ao iniciar (a tarefa se suspende em seguida)
    // Usa LED_RGB_A por padrão (R=13, G=11, B=12) com ativo-baixo, conforme a tarefa.
    criar_tarefa_led_verde(PRIO_LED_VERDE, CORE0_MASK);

    // Tarefa de leitura dos 6 bits -> núcleo 0
    // Parâmetro use_pullup = true para estabilidade quando a FPGA estiver em tri-state
    criar_tarefa_word6(PRIO_WORD6, CORE0_MASK, true);

    // Tarefa de exibição no OLED -> núcleo 1
    criar_tarefa_display_word6(PRIO_OLED, CORE1_MASK);

    // Scheduler
    vTaskStartScheduler();

    // Não deve retornar
    while (true) { tight_loop_contents(); }
}

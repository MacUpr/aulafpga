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
#include "oled_context.h"          // expõe 'oled' e 'mutex_oled'

// Tela que mostra estados das 7 “chaves”
#include "tarefa_display_chaves.h"

// ==== I/O por eventos do joystick (ADC+DMA) ====
#include "tarefa_joystick_eventos.h"

// ==== LED opcional ====
#include "tarefa_led_verde.h"

// ==== Onda quadrada (duas saídas) ====
#include "tarefa_onda_quadrada.h"

// ==== DMA IRQ handler (usado pelos DMAs do joystick) ====
#include "hardware/irq.h"
#include "dma_handlers.h"          // dma_handler_joy_xy -> notifica g_hJoyEvt

// ==== Núcleos (RP2040) ====
#define CORE0_MASK  ((UBaseType_t)(1u << 0))
#define CORE1_MASK  ((UBaseType_t)(1u << 1))

// ==== Prioridades ====
#define PRIO_JOY_EVT    (tskIDLE_PRIORITY + 3)
#define PRIO_OLED       (tskIDLE_PRIORITY + 1)
#define PRIO_LED_VERDE  (tskIDLE_PRIORITY + 1)
#define PRIO_ONDA       (tskIDLE_PRIORITY + 1)

// Ponha 1 se REALMENTE estiver linkando tarefa_botao_a.c / _b.c / _joy.c
#ifndef USE_TAREFAS_BOTOES
#define USE_TAREFAS_BOTOES  1
#endif

#if USE_TAREFAS_BOTOES
  #include "tarefa_botao_a.h"
  #include "tarefa_botao_b.h"
  #include "tarefa_botao_joy.h"
  #define PRIO_BTN_A      (tskIDLE_PRIORITY + 2)
  #define PRIO_BTN_B      (tskIDLE_PRIORITY + 1)
  #define PRIO_BTN_JOY    (tskIDLE_PRIORITY + 3)
#endif

int main(void) {
    stdio_init_all();
    // while (!stdio_usb_connected()) { tight_loop_contents(); } // opcional
    printf("=== INICIANDO SISTEMA: CHAVES + JOYSTICK + OLED + ONDAS ===\n");

    // ---- OLED ----
    if (!oled_init(&oled)) {
        printf("Falha ao inicializar OLED!\n");
        while (true) { tight_loop_contents(); }
    }
    // mutex global do OLED (declarado em oled_context.h)
    mutex_oled = xSemaphoreCreateMutex();
    configASSERT(mutex_oled != NULL);

    // ---- IRQ DMA1: ISR (dma_handler_joy_xy) notifica a tarefa do joystick ----
    irq_set_exclusive_handler(DMA_IRQ_1, dma_handler_joy_xy);
    irq_set_enabled(DMA_IRQ_1, true);

    // ---- Tarefas principais ----

    // 1) OLED: mostra estados das 7 saídas (linhas “SAÍDAS”, “Chave 1..7”)
    criar_tarefa_display_chaves(PRIO_OLED, CORE0_MASK);

    // 2) Eventos do joystick (X>=3800, X<=500, Y>=3800, Y<=500) dirigem pinos dedicados
    criar_tarefa_joystick_eventos(PRIO_JOY_EVT, CORE1_MASK);

#if USE_TAREFAS_BOTOES
    // 3) Botões dedicados (se estiverem linkados no projeto)
    criar_tarefa_botao_b  (PRIO_BTN_B,   CORE1_MASK);
    criar_tarefa_botao_a  (PRIO_BTN_A,   CORE1_MASK);
    criar_tarefa_botao_joy(PRIO_BTN_JOY, CORE1_MASK);
#endif

    // 4) LED (opcional)
    criar_tarefa_led_verde(PRIO_LED_VERDE, CORE1_MASK);

    // 5) Ondas quadradas
    TaskHandle_t h_onda17 = NULL, h_onda28 = NULL;

    // GP17 @ 1 Hz
    tarefa_onda_quadrada_cfg_t cfg17 = {
        .gpio_wave = 17,
        .freq_hz   = 1.0f,
        .gpio_led_r = GPIO_UNUSED, .gpio_led_g = GPIO_UNUSED, .gpio_led_b = GPIO_UNUSED,
        .led_active_low = true,          // irrelevante sem LED
        .task_name = "onda17",
        .task_priority = PRIO_ONDA,
        .task_stack = 1024,
        .core_affinity_mask = 0          // 0 = qualquer core (mais seguro)
    };
    configASSERT(tarefa_onda_quadrada_start(&cfg17, &h_onda17) == pdPASS);

    // GP28 @ 5 Hz
    tarefa_onda_quadrada_cfg_t cfg28 = {
        .gpio_wave = 28,
        .freq_hz   = 5.0f,
        .gpio_led_r = GPIO_UNUSED, .gpio_led_g = GPIO_UNUSED, .gpio_led_b = GPIO_UNUSED,
        .led_active_low = true,
        .task_name = "onda28",
        .task_priority = PRIO_ONDA,
        .task_stack = 1024,
        .core_affinity_mask = 0          // idem
    };
    configASSERT(tarefa_onda_quadrada_start(&cfg28, &h_onda28) == pdPASS);

    // ---- RTOS ----
    vTaskStartScheduler();

    // Nunca deve chegar aqui
    while (true) { tight_loop_contents(); }
}

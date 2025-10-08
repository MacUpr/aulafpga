#include "tarefa_led_verde.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

// Palavra de 6 bits (vinda da outra tarefa)
extern volatile uint8_t g_word6_value;

// ========== CONFIGS ==========
#ifndef WORD6_GATE_BIT
#define WORD6_GATE_BIT 0       // 0..5 escolhe o bit que habilita o pisca
#endif
#ifndef WORD6_ACTIVE_HIGH
#define WORD6_ACTIVE_HIGH 1    // 1: bit=1 habilita; 0: bit=0 habilita
#endif

// >>> Escolha a base de sincronismo do pisca:
//    - SAMPLE: pisca a cada leitura da outra tarefa (PERIOD_MS)
//    - CONFIRM: pisca na cadência de confirmação (2 * PERIOD_MS)
#ifndef WORD6_PERIOD_MS
#define WORD6_PERIOD_MS 50u     // tem que bater com PERIOD_MS da tarefa word6
#endif
#ifndef LED_SYNC_WITH_CONFIRM
#define LED_SYNC_WITH_CONFIRM 1  // 1 = usa 2*PERIOD_MS; 0 = usa PERIOD_MS
#endif

// LED físico
#ifndef GPIO_LED_R
#define GPIO_LED_R   13
#endif
#ifndef GPIO_LED_G
#define GPIO_LED_G   11
#endif
#ifndef GPIO_LED_B
#define GPIO_LED_B   12
#endif

#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW 0
#endif

static inline int level_for(bool on) {
#if LED_ACTIVE_LOW
    return on ? 0 : 1;
#else
    return on ? 1 : 0;
#endif
}

static inline void rgb_init_pins(void) {
    gpio_init(GPIO_LED_R); gpio_set_dir(GPIO_LED_R, GPIO_OUT);
    gpio_init(GPIO_LED_G); gpio_set_dir(GPIO_LED_G, GPIO_OUT);
    gpio_init(GPIO_LED_B); gpio_set_dir(GPIO_LED_B, GPIO_OUT);
    gpio_put(GPIO_LED_R, level_for(false));
    gpio_put(GPIO_LED_G, level_for(false));
    gpio_put(GPIO_LED_B, level_for(false));
}

static inline void rgb_set(bool r_on, bool g_on, bool b_on) {
    gpio_put(GPIO_LED_R, level_for(r_on));
    gpio_put(GPIO_LED_G, level_for(g_on));
    gpio_put(GPIO_LED_B, level_for(b_on));
}

static inline bool gate_is_enabled(void) {
    uint8_t v = g_word6_value;
    uint8_t bit = (v >> WORD6_GATE_BIT) & 1u;
#if WORD6_ACTIVE_HIGH
    return bit != 0u;
#else
    return bit == 0u;
#endif
}

// ======== TAREFA: pisca VERDE sincronizado com a leitura/confirm. ========
static void tarefa_led_verde(void *params) {
    (void)params;

    rgb_init_pins();

    // dt_sync = período de pisca (igual ao período de leitura ou de confirmação)
    const uint32_t base_ms = WORD6_PERIOD_MS;
    const TickType_t dt_sync = pdMS_TO_TICKS(
        LED_SYNC_WITH_CONFIRM ? (2u * base_ms) : base_ms
    );

    TickType_t t0 = xTaskGetTickCount();
    bool on = false;

    for (;;) {
        if (gate_is_enabled()) {
            on = !on;                       // alterna a cada “leitura” (ou confirmação)
            rgb_set(false, on, false);      // pisca verde
        } else {
            on = false;
            rgb_set(false, false, false);   // desabilitado → apagado
        }
        vTaskDelayUntil(&t0, dt_sync);      // sincroniza com a cadência escolhida
    }
}

// Wrapper
void criar_tarefa_led_verde(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_led_verde, "LED_VERDE", 512, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

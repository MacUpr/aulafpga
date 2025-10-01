#include "tarefa_led_verde.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

// ======== PINOS DO LED RGB (BitDogLab: LED_RGB_A) ========
#define GPIO_LED_R   13   // R
#define GPIO_LED_G   11   // G
#define GPIO_LED_B   12   // B

// Seu hardware está acendendo em nível alto -> ativo-ALTO
#define LED_ACTIVE_LOW   0   // <<< ALTERADO PARA 0

// Período do pisca
#define ON_MS   250u
#define OFF_MS  250u

// Converte "ligado/desligado" -> nível lógico no pino
static inline int level_for(bool on) {
#if LED_ACTIVE_LOW
    return on ? 0 : 1;  // ativo-baixo: 0 acende
#else
    return on ? 1 : 0;  // ativo-alto: 1 acende   <<< AQUI
#endif
}

static inline void rgb_init_pins(void) {
    gpio_init(GPIO_LED_R); gpio_set_dir(GPIO_LED_R, GPIO_OUT);
    gpio_init(GPIO_LED_G); gpio_set_dir(GPIO_LED_G, GPIO_OUT);
    gpio_init(GPIO_LED_B); gpio_set_dir(GPIO_LED_B, GPIO_OUT);

    // Apaga tudo inicialmente
    gpio_put(GPIO_LED_R, level_for(false));
    gpio_put(GPIO_LED_G, level_for(false));
    gpio_put(GPIO_LED_B, level_for(false));
}

static inline void rgb_set(bool r_on, bool g_on, bool b_on) {
    gpio_put(GPIO_LED_R, level_for(r_on));
    gpio_put(GPIO_LED_G, level_for(g_on));
    gpio_put(GPIO_LED_B, level_for(b_on));
}

// ======== TAREFA: piscar verde ========
static void tarefa_led_verde(void *params) {
    (void)params;

    rgb_init_pins();

    TickType_t t0 = xTaskGetTickCount();
    const TickType_t dt_on  = pdMS_TO_TICKS(ON_MS);
    const TickType_t dt_off = pdMS_TO_TICKS(OFF_MS);

    for (;;) {
        // VERDE
        rgb_set(false, true, false);
        vTaskDelayUntil(&t0, dt_on);

        // APAGADO
        rgb_set(false, false, false);
        vTaskDelayUntil(&t0, dt_off);
    }
}

// Wrapper de criação com afinidade de núcleo
void criar_tarefa_led_verde(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_led_verde, "LED_VERDE", 512, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

// tarefa_led_verde.c — LED verde segue o MSB da palavra de 9 bits
#include "tarefa_led_verde.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#include "tarefa_word_9.h"   // word9_get()

#include <stdbool.h>

// ======== PINOS DO LED RGB (BitDogLab: LED_RGB_A) ========
// Mapeamento: R=13, G=12, B=11
#define GPIO_LED_R   13
#define GPIO_LED_G   12
#define GPIO_LED_B   11

// Polaridade do hardware
#define LED_ACTIVE_LOW   0   // 0 = ativo-alto (1 acende). Use 1 se for comum-ânodo (0 acende)

// Período de varredura do MSB (não afeta frequência do pisca; só a taxa de leitura)
#define MSB_POLL_MS  20u

static inline int level_for(bool on) {
#if LED_ACTIVE_LOW
    return on ? 0 : 1;  // ativo-baixo: 0 acende
#else
    return on ? 1 : 0;  // ativo-alto: 1 acende
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

// ======== TAREFA: LED verde segue o MSB ========
static void tarefa_led_verde(void *params) {
    (void)params;

    rgb_init_pins();

    bool last = false;
    const TickType_t dt = pdMS_TO_TICKS(MSB_POLL_MS);

    for (;;) {
        // Lê MSB da última palavra disponível
        uint16_t w  = word9_get();
        bool msb_on = ((w >> 8) & 1u) != 0;

        // Atualiza só se mudou (evita jitter desnecessário no GPIO)
        if (msb_on != last) {
            last = msb_on;
            // Verde segue o MSB; vermelho/azul desligados
            rgb_set(false, msb_on, false);
        }

        vTaskDelay(dt);
    }
}

// Wrapper de criação com afinidade de núcleo
void criar_tarefa_led_verde(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_led_verde, "LED_VERDE_MSB", 512, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

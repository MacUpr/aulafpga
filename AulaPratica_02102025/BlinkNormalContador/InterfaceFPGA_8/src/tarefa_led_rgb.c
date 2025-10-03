// tarefa_led_verde.c — LED RGB controlado por entradas GP16, GP17, GP28
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

// ======== LED RGB físico (BitDogLab: LED_RGB_A) ========
#ifndef GPIO_LED_R
#define GPIO_LED_R   13   // R
#endif
#ifndef GPIO_LED_G
#define GPIO_LED_G   11   // G
#endif
#ifndef GPIO_LED_B
#define GPIO_LED_B   12   // B
#endif

// ======== Entradas que controlam R,G,B ========
// (Se já existirem #defines no projeto, estes fallbacks não serão usados)
#ifndef GPIO_IN_R
#define GPIO_IN_R 16      // bit6
#endif
#ifndef GPIO_IN_G
#define GPIO_IN_G 17      // bit7
#endif
#ifndef GPIO_IN_B
#define GPIO_IN_B 28      // bit8
#endif

// Entradas ativas em nível alto? (1 = liga LED lógico)
#ifndef INPUT_ACTIVE_HIGH
#define INPUT_ACTIVE_HIGH 1
#endif

// LED físico ativo-baixo (comum-ânodo na BitDogLab)
#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW 1
#endif

// ======== Utilidades LED ========
static inline void rgb_init_pins(void) {
    gpio_init(GPIO_LED_R); gpio_set_dir(GPIO_LED_R, GPIO_OUT);
    gpio_init(GPIO_LED_G); gpio_set_dir(GPIO_LED_G, GPIO_OUT);
    gpio_init(GPIO_LED_B); gpio_set_dir(GPIO_LED_B, GPIO_OUT);
}

static inline int level_for(bool on) {
#if LED_ACTIVE_LOW
    return on ? 1 : 0;   // ativo-baixo: 0 acende
#else
    return on ? 0 : 1;   // ativo-alto: 1 acende
#endif
}

static inline void rgb_write(bool r_on, bool g_on, bool b_on) {
    gpio_put(GPIO_LED_R, level_for(r_on));
    gpio_put(GPIO_LED_G, level_for(g_on));
    gpio_put(GPIO_LED_B, level_for(b_on));
}

// ======== Utilidades entrada ========
static inline void input_init_pullup(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_disable_pulls(pin);
    gpio_pull_up(pin); // solicitou pull-up
}

static inline bool read_input_on(uint pin) {
    // Lê e converte para "ON lógico" conforme INPUT_ACTIVE_HIGH
    bool v = (gpio_get(pin) & 1u) != 0;
#if INPUT_ACTIVE_HIGH
    return v;           // 1 => ON
#else
    return !v;          // 0 => ON (ativo-baixo na entrada)
#endif
}

// ======== Tarefa: LED segue as entradas ========
#ifndef LED_SCAN_MS
#define LED_SCAN_MS 20u  // varredura ~50 Hz
#endif

static void tarefa_led_verde(void *params) {
    (void)params;

    // LED como saída
    rgb_init_pins();

    // Entradas com pull-up
    input_init_pullup(GPIO_IN_R);
    input_init_pullup(GPIO_IN_G);
    input_init_pullup(GPIO_IN_B);

    // Estado anterior para evitar writes desnecessários
    bool pr = false, pg = false, pb = false;
    bool fr = true; // força primeira atualização

    const TickType_t dt = pdMS_TO_TICKS(LED_SCAN_MS);

    for (;;) {
        // Amostragem dupla simples para estabilidade
        bool r1 = read_input_on(GPIO_IN_R);
        bool g1 = read_input_on(GPIO_IN_G);
        bool b1 = read_input_on(GPIO_IN_B);
        vTaskDelay(dt);
        bool r2 = read_input_on(GPIO_IN_R);
        bool g2 = read_input_on(GPIO_IN_G);
        bool b2 = read_input_on(GPIO_IN_B);

        bool r_on = (r1 == r2) ? r2 : r2; // mantém a segunda leitura
        bool g_on = (g1 == g2) ? g2 : g2;
        bool b_on = (b1 == b2) ? b2 : b2;

        if (fr || r_on != pr || g_on != pg || b_on != pb) {
            rgb_write(r_on, g_on, b_on);
            pr = r_on; pg = g_on; pb = b_on;
            fr = false;
        }

        // Pequeno intervalo antes do próximo ciclo
        vTaskDelay(dt);
    }
}

// Wrapper de criação com afinidade de núcleo
void criar_tarefa_led_verde(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_led_verde, "LED_RGB_IN", 768, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

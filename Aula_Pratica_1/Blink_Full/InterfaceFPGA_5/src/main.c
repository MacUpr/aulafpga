#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

// ======= AJUSTES DE HARDWARE =======
#define GPIO_IN_PIN       18     // entrada que será lida
#define LED_R_PIN         13     // BitDogLab: R=13, G=11, B=12
#define LED_G_PIN         11
#define LED_B_PIN         12

#define LED_ACTIVE_LOW    true   // LED comum-ânodo (nível 0 acende)
#define INPUT_USE_PULLUP  true   // mantém em 3V3 quando solto
#define INPUT_ACTIVE_HIGH true   // ativo quando nível lógico = 1 (3V3)

#define INPUT_SCAN_MS     5
#define INPUT_DEBOUNCE_MS 20

// ======= UTILIDADES DE LED =======
static inline void led_all_off(void) {
#if LED_ACTIVE_LOW
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
#else
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
#endif
}

static inline void led_green(bool on) {
#if LED_ACTIVE_LOW
    gpio_put(LED_R_PIN, 0);              // apaga R
    gpio_put(LED_B_PIN, 0);              // apaga B
    gpio_put(LED_G_PIN, on ? 0 : 1);     // G acende em 0
#else
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_B_PIN, 0);
    gpio_put(LED_G_PIN, on ? 1 : 0);
#endif
}

// ======= AUX =======
static inline bool input_ativo(void) {
    bool raw = gpio_get(GPIO_IN_PIN);
#if INPUT_ACTIVE_HIGH
    return raw;     // agora ativo quando for 1 (3V3)
#else
    return !raw;
#endif
}

// ======= TAREFA: lê GPIO18 e controla LED VERDE =======
static void tarefa_gpio_led(void *arg) {
    (void)arg;

    // LED RGB como saída
    gpio_init(LED_R_PIN); gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN); gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN); gpio_set_dir(LED_B_PIN, GPIO_OUT);
    led_all_off();

    // Entrada
    gpio_init(GPIO_IN_PIN);
    gpio_set_dir(GPIO_IN_PIN, GPIO_IN);
#if INPUT_USE_PULLUP
    gpio_disable_pulls(GPIO_IN_PIN);
    gpio_pull_up(GPIO_IN_PIN);
#else
    gpio_disable_pulls(GPIO_IN_PIN);
#endif

    bool confirmado = input_ativo();
    bool amostrado  = confirmado;
    TickType_t janela_ini = xTaskGetTickCount();
    led_green(confirmado);

    const TickType_t periodo = pdMS_TO_TICKS(INPUT_SCAN_MS);
    TickType_t wake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&wake, periodo);

        bool agora = input_ativo();

        if (agora != amostrado) {
            amostrado = agora;
            janela_ini = xTaskGetTickCount();
            continue;
        }

        if (amostrado != confirmado) {
            if ((xTaskGetTickCount() - janela_ini) >= pdMS_TO_TICKS(INPUT_DEBOUNCE_MS)) {
                confirmado = amostrado;
                led_green(confirmado);
            }
        }
    }
}

int main(void) {
    stdio_init_all();

    xTaskCreate(
        tarefa_gpio_led, "gpio_led",
        768, NULL, (tskIDLE_PRIORITY + 1), NULL);

    vTaskStartScheduler();
    while (true) { /* nunca chega aqui */ }
    return 0;
}

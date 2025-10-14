#include "tarefa_botao_a.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

static inline void cfg_in_pullup(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

static inline void cfg_out_zero(uint pin) {
    gpio_init(pin);
    gpio_disable_pulls(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0); // inicia 0
}

static void tarefa_botao_a_entry(void *param) {
    (void)param;

    cfg_in_pullup(GPIO_BOTAO_A);
    cfg_out_zero(PIN_OUT_BTN_A);

    bool stable_pressed = (gpio_get(GPIO_BOTAO_A) == 0);
    TickType_t t_mark   = xTaskGetTickCount();

    const TickType_t dt       = pdMS_TO_TICKS(BTN_PERIOD_MS);
    const TickType_t tdeb     = pdMS_TO_TICKS(BTN_DEBOUNCE_MS);

    for (;;) {
        bool raw_pressed = (gpio_get(GPIO_BOTAO_A) == 0);

        if (raw_pressed != stable_pressed) {
            TickType_t now = xTaskGetTickCount();
            if ((now - t_mark) >= tdeb) {
                // Borda real
#if BTN_A_PULSE_MODE
                // PULSE: 1 enquanto pressionado; 0 ao soltar
                gpio_put(PIN_OUT_BTN_A, raw_pressed ? 1 : 0);
#else
                // TOGGLE: alterna apenas na borda de pressão (solta não faz nada)
                if (raw_pressed && !stable_pressed) {
                    gpio_put(PIN_OUT_BTN_A, !gpio_get_out_level(PIN_OUT_BTN_A));
                }
#endif
                stable_pressed = raw_pressed;
                t_mark = now;
            }
        } else {
            t_mark = xTaskGetTickCount();
        }

        vTaskDelay(dt);
    }
}

void criar_tarefa_botao_a(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_a_entry, "btnA", 768, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

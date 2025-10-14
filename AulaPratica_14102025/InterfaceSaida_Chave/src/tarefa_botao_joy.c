#include "tarefa_botao_joy.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "task.h"
#include <stdbool.h>

static inline void cfg_in_pullup(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

static inline void cfg_out_zero(uint pin) {
    gpio_init(pin);
    gpio_disable_pulls(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0);
}

static void tarefa_botao_joy_entry(void *param) {
    (void)param;

    cfg_in_pullup(GPIO_BOTAO_JOY);
    cfg_out_zero(PIN_OUT_BTN_J);

    bool stable_pressed = (gpio_get(GPIO_BOTAO_JOY) == 0);
    TickType_t t_mark   = xTaskGetTickCount();

    const TickType_t dt   = pdMS_TO_TICKS(BTN_PERIOD_MS);
    const TickType_t tdeb = pdMS_TO_TICKS(BTN_DEBOUNCE_MS);

    for (;;) {
        bool raw_pressed = (gpio_get(GPIO_BOTAO_JOY) == 0);

        if (raw_pressed != stable_pressed) {
            TickType_t now = xTaskGetTickCount();
            if ((now - t_mark) >= tdeb) {
#if BTN_JOY_PULSE_MODE
                gpio_put(PIN_OUT_BTN_J, raw_pressed ? 1 : 0);
#else
                if (raw_pressed && !stable_pressed) {
                    gpio_put(PIN_OUT_BTN_J, !gpio_get_out_level(PIN_OUT_BTN_J));
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

void criar_tarefa_botao_joy(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_joy_entry, "btnJoy", 768, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

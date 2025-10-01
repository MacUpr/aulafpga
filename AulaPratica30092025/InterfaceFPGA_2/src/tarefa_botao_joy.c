#include "tarefa_botao_joy.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define PERIOD_MS     5u
#define DEBOUNCE_MS  20u

#ifndef GPIO_BOTAO_JOY
#define GPIO_BOTAO_JOY 22
#endif
#ifndef GPIO_OUT_JOY
#define GPIO_OUT_JOY   28
#endif

void tarefa_botao_joy(void *params) {
    (void)params;

    gpio_init(GPIO_BOTAO_JOY);
    gpio_set_dir(GPIO_BOTAO_JOY, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_JOY);

    gpio_init(GPIO_OUT_JOY);
    gpio_set_dir(GPIO_OUT_JOY, GPIO_OUT);
    gpio_put(GPIO_OUT_JOY, 1);

    bool stable = (gpio_get(GPIO_BOTAO_JOY) == 0);
    TickType_t t_mark = xTaskGetTickCount();

    for (;;) {
        bool raw = (gpio_get(GPIO_BOTAO_JOY) == 0);
        if (raw != stable) {
            TickType_t now = xTaskGetTickCount();
            if (now - t_mark >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                stable = raw;
                t_mark = now;
            }
        } else {
            t_mark = xTaskGetTickCount();
        }

        gpio_put(GPIO_OUT_JOY, stable ? 0 : 1);

        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

void criar_tarefa_botao_joy(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_joy, "BotaoJoy", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

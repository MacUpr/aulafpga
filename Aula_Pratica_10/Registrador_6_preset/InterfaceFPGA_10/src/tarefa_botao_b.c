#include "tarefa_botao_b.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define PERIOD_MS     5u
#define DEBOUNCE_MS  20u

#ifndef GPIO_BOTAO_B
#define GPIO_BOTAO_B 6
#endif
#ifndef GPIO_OUT_B
#define GPIO_OUT_B   17
#endif

void tarefa_botao_b(void *params) {
    (void)params;

    gpio_init(GPIO_BOTAO_B);
    gpio_set_dir(GPIO_BOTAO_B, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_B);

    gpio_init(GPIO_OUT_B);
    gpio_set_dir(GPIO_OUT_B, GPIO_OUT);
    gpio_put(GPIO_OUT_B, 1);

    bool stable = (gpio_get(GPIO_BOTAO_B) == 0);
    TickType_t t_mark = xTaskGetTickCount();

    for (;;) {
        bool raw = (gpio_get(GPIO_BOTAO_B) == 0);
        if (raw != stable) {
            TickType_t now = xTaskGetTickCount();
            if (now - t_mark >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                stable = raw;
                t_mark = now;
            }
        } else {
            t_mark = xTaskGetTickCount();
        }

        gpio_put(GPIO_OUT_B, stable ? 0 : 1);

        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

void criar_tarefa_botao_b(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_b, "BotaoB", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

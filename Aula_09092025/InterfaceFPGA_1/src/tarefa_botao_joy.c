#include "tarefa_botao_joy.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define PERIOD_MS        5u
#define DEBOUNCE_MS     20u

void tarefa_botao_joy(void *params) {
    (void)params;

    gpio_init(GPIO_BOTAO_JOY);
    gpio_set_dir(GPIO_BOTAO_JOY, GPIO_IN);
    gpio_disable_pulls(GPIO_BOTAO_JOY);
    gpio_pull_up(GPIO_BOTAO_JOY);

    gpio_init(GPIO_OUT_JOY);
    gpio_set_dir(GPIO_OUT_JOY, GPIO_OUT);
    gpio_put(GPIO_OUT_JOY, 1);

    bool prev = (gpio_get(GPIO_BOTAO_JOY) == 0);
    TickType_t t0 = xTaskGetTickCount();

    for (;;) {
        bool now = (gpio_get(GPIO_BOTAO_JOY) == 0);
        if (now != prev) {
            TickType_t t = xTaskGetTickCount();
            if (t - t0 >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                prev = now;
                t0   = t;
                gpio_put(GPIO_OUT_JOY, prev ? 0 : 1);
            }
        } else {
            gpio_put(GPIO_OUT_JOY, prev ? 0 : 1);
        }
        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

// --- wrapper de criação ---
void criar_tarefa_botao_joy(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_joy, "BotaoJoy", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

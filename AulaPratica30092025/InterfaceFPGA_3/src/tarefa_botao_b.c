#include "tarefa_botao_b.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define PERIOD_MS     5u
#define DEBOUNCE_MS  20u
#define PULSE_MS     50u   // largura do pulso em nível 0 (50 ms)


void tarefa_botao_b(void *params) {
    (void)params;

    gpio_init(GPIO_BOTAO_B);
    gpio_set_dir(GPIO_BOTAO_B, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_B);

    gpio_init(GPIO_OUT_B);
    gpio_set_dir(GPIO_OUT_B, GPIO_OUT);
    gpio_put(GPIO_OUT_B, 1);

    bool prev = (gpio_get(GPIO_BOTAO_B) == 0);
    TickType_t t0 = xTaskGetTickCount();

    for (;;) {
        bool now = (gpio_get(GPIO_BOTAO_B) == 0);

        if (now != prev) {
            TickType_t t = xTaskGetTickCount();
            if (t - t0 >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                if (!prev && now) {                     // borda de descida
                    gpio_put(GPIO_OUT_B, 0);
                    vTaskDelay(pdMS_TO_TICKS(PULSE_MS));
                    gpio_put(GPIO_OUT_B, 1);
                }
                prev = now;
                t0   = t;
            }
        } else {
            gpio_put(GPIO_OUT_B, 1);
        }

        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

// Wrapper
void criar_tarefa_botao_b(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_b, "BotaoB", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

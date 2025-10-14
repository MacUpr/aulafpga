#include "tarefa_botao_b.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>

#include "tarefa_word6.h"   // WORD9_MASK, word6_get, g_word6_value

#define PERIOD_MS     5u
#define DEBOUNCE_MS  20u

// Pressionar o botão B ==> coloca todos os 9 bits em 1
void tarefa_botao_b(void *params) {
    (void)params;

    gpio_init(GPIO_BOTAO_B);
    gpio_set_dir(GPIO_BOTAO_B, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_B);  // ativo em nível baixo

    bool prev = (gpio_get(GPIO_BOTAO_B) == 0);
    TickType_t t0 = xTaskGetTickCount();

    for (;;) {
        bool now = (gpio_get(GPIO_BOTAO_B) == 0);   // pressionado = 0

        if (now != prev) {
            TickType_t t = xTaskGetTickCount();
            if ((t - t0) >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                // borda de descida: solto(1) -> pressionado(0)
                if (!prev && now) {
                    taskENTER_CRITICAL();
                    g_word6_value = WORD9_MASK;     // SETA todos os 9 bits (0x1FF)
                    taskEXIT_CRITICAL();
                }
                prev = now;
                t0   = t;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

void criar_tarefa_botao_b(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_b, "BotaoB", 768, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

#include "tarefa_botao_a.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define PERIOD_MS    5u
#define DEBOUNCE_MS 20u

void tarefa_botao_a(void *params) {
    (void)params;

    // Botão ativo-baixo com pull-up
    gpio_init(GPIO_BOTAO_A);
    gpio_set_dir(GPIO_BOTAO_A, GPIO_IN);
    gpio_disable_pulls(GPIO_BOTAO_A);
    gpio_pull_up(GPIO_BOTAO_A);

    // Saída começa em 1
    gpio_init(GPIO_OUT_A);
    gpio_set_dir(GPIO_OUT_A, GPIO_OUT);
    bool out = true;
    gpio_put(GPIO_OUT_A, out);

    // Estado inicial do botão (true = pressionado, pois é ativo-baixo)
    bool prev = (gpio_get(GPIO_BOTAO_A) == 0);
    TickType_t t0 = xTaskGetTickCount();

    for (;;) {
        bool now = (gpio_get(GPIO_BOTAO_A) == 0);  // ativo-baixo: 0 no pino => pressed=true
        if (now != prev) {
            TickType_t t = xTaskGetTickCount();
            if (t - t0 >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                // Borda de DESCIDA (solto->pressionado)? Faz TOGGLE
                if (!prev && now) {
                    out = !out;
                    gpio_put(GPIO_OUT_A, out);
                }
                prev = now;
                t0   = t;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

// --- wrapper de criação ---
void criar_tarefa_botao_a(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_a, "BotaoA", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

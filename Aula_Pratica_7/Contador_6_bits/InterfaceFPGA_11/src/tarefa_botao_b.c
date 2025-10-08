#include "tarefa_botao_b.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define PERIOD_MS     5u
#define DEBOUNCE_MS  20u

void tarefa_botao_b(void *params) {
    (void)params;

    // ---- Entrada: botão ativo-baixo com pull-up ----
    gpio_init(GPIO_BOTAO_B);
    gpio_set_dir(GPIO_BOTAO_B, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_B);  // repouso em nível alto

    // ---- Saída: começa em 1 ----
    gpio_init(GPIO_OUT_B);
    gpio_set_dir(GPIO_OUT_B, GPIO_OUT);
    bool out = true;
    gpio_put(GPIO_OUT_B, out);

    // Estado processado (após debounce): true=pressionado (nível 0 no pino)
    bool prev = (gpio_get(GPIO_BOTAO_B) == 0);
    TickType_t t0 = xTaskGetTickCount();

    for (;;) {
        bool now = (gpio_get(GPIO_BOTAO_B) == 0);  // true=pressed

        if (now != prev) {
            TickType_t t = xTaskGetTickCount();
            if (t - t0 >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                // Borda de descida: solto(false) -> pressionado(true)
                if (!prev && now) {
                    out = !out;                 // alterna estado
                    gpio_put(GPIO_OUT_B, out);  // aplica na saída
                }
                prev = now;
                t0   = t;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

// --- wrapper de criação ---
void criar_tarefa_botao_b(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_b, "BotaoB", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

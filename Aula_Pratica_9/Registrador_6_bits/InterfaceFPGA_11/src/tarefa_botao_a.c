#include "tarefa_botao_a.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define PERIOD_MS     5u
#define DEBOUNCE_MS  20u
#define PULSE_MS     150u   // largura do pulso em nível 0 (50 ms)


void tarefa_botao_a(void *params) {
    (void)params;

    // Entrada: ativo-baixo com pull-up
    gpio_init(GPIO_BOTAO_A);
    gpio_set_dir(GPIO_BOTAO_A, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_A);

    // Saída: começa em nível alto
    gpio_init(GPIO_OUT_A);
    gpio_set_dir(GPIO_OUT_A, GPIO_OUT);
    gpio_put(GPIO_OUT_A, 1);

    bool prev = (gpio_get(GPIO_BOTAO_A) == 0); // true=pressionado
    TickType_t t0 = xTaskGetTickCount();

    for (;;) {
        bool now = (gpio_get(GPIO_BOTAO_A) == 0); // true=pressed (0 no pino)

        if (now != prev) {
            TickType_t t = xTaskGetTickCount();
            if (t - t0 >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                // Borda de descida: solto(false) -> pressionado(true)
                if (!prev && now) {
                    gpio_put(GPIO_OUT_A, 0);                      // pulso em 0
                    vTaskDelay(pdMS_TO_TICKS(PULSE_MS));
                    gpio_put(GPIO_OUT_A, 1);                      // retorna para 1
                }
                prev = now;
                t0   = t;
            }
        } else {
            // Garantir nível alto enquanto não há pulso
            gpio_put(GPIO_OUT_A, 1);
        }

        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

// Wrapper
void criar_tarefa_botao_a(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_a, "BotaoA", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

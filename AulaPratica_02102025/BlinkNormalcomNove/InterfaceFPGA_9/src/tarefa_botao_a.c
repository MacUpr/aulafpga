#include "tarefa_botao_a.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define PERIOD_MS     5u
#define DEBOUNCE_MS  20u

#ifndef GPIO_BOTAO_A
#define GPIO_BOTAO_A 5
#endif
#ifndef GPIO_OUT_A
#define GPIO_OUT_A   16
#endif

void tarefa_botao_a(void *params) {
    (void)params;

    // Entrada: ativo-baixo com pull-up
    gpio_init(GPIO_BOTAO_A);
    gpio_set_dir(GPIO_BOTAO_A, GPIO_IN);
    gpio_pull_up(GPIO_BOTAO_A);

    // Saída: começa em nível alto (solto = 1)
    gpio_init(GPIO_OUT_A);
    gpio_set_dir(GPIO_OUT_A, GPIO_OUT);
    gpio_put(GPIO_OUT_A, 1);

    bool stable = (gpio_get(GPIO_BOTAO_A) == 0); // estado estável (true=pressionado)
    TickType_t t_mark = xTaskGetTickCount();

    for (;;) {
        bool raw = (gpio_get(GPIO_BOTAO_A) == 0);       // leitura bruta (0 no pino => pressed)
        if (raw != stable) {
            TickType_t now = xTaskGetTickCount();
            if (now - t_mark >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                stable = raw;                            // aceita mudança após debounce
                t_mark = now;
            }
        } else {
            t_mark = xTaskGetTickCount();               // reinicia janela se permanecer igual
        }

        // Segue o estado estável do botão: pressionado => 0, solto => 1
        gpio_put(GPIO_OUT_A, stable ? 0 : 1);

        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

// Wrapper de criação
void criar_tarefa_botao_a(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_botao_a, "BotaoA", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

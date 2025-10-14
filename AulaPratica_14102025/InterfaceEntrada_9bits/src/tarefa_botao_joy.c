#include "tarefa_botao_joy.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

#define PERIOD_MS     5u
#define DEBOUNCE_MS  20u

void tarefa_botao_joy(void *params) {
    (void)params;

    // Botão do joystick: ativo-baixo com pull-up
    gpio_init(GPIO_BOTAO_JOY);
    gpio_set_dir(GPIO_BOTAO_JOY, GPIO_IN);
    gpio_disable_pulls(GPIO_BOTAO_JOY);
    gpio_pull_up(GPIO_BOTAO_JOY);

    // Saída começa em nível alto; alterna a cada pressão
    gpio_init(GPIO_OUT_JOY);
    gpio_set_dir(GPIO_OUT_JOY, GPIO_OUT);
    bool out = true;
    gpio_put(GPIO_OUT_JOY, out);

    // Estado inicial do botão (true = pressionado, por ser ativo-baixo)
    bool prev = (gpio_get(GPIO_BOTAO_JOY) == 0);
    TickType_t t0 = xTaskGetTickCount();

    for (;;) {
        bool now = (gpio_get(GPIO_BOTAO_JOY) == 0);  // 0 no pino => pressionado
        if (now != prev) {
            TickType_t t = xTaskGetTickCount();
            if (t - t0 >= pdMS_TO_TICKS(DEBOUNCE_MS)) {
                // Borda de DESCIDA (solto -> pressionado) => TOGGLE
                if (!prev && now) {
                    out = !out;
                    gpio_put(GPIO_OUT_JOY, out);
                }
                prev = now;
                t0   = t;
            }
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

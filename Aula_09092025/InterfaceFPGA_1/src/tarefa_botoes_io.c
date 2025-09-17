#include "tarefa_botoes_io.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>

// === Função genérica de task para cada botão ===
static void tarefa_botao(void *params) {
    const uint entrada = ((uint*)params)[0];
    const uint saida  = ((uint*)params)[1];

    printf("[tarefa_botao] Iniciada (IN=%u, OUT=%u)\n", entrada, saida);

    for (;;) {
        bool pressed = (gpio_get(entrada) == 0);  // pull-up → pressionado = 0
        gpio_put(saida, pressed ? 0 : 1);
        vTaskDelay(pdMS_TO_TICKS(20)); // debounce simples
    }
}

// === Função de criação das 3 tarefas ===
void criar_tarefas_botoes_io(UBaseType_t prio, UBaseType_t core_mask) {
    // Inicializa GPIOs de entrada
    gpio_init(GPIO_BOTAO_A); gpio_set_dir(GPIO_BOTAO_A, GPIO_IN); gpio_pull_up(GPIO_BOTAO_A);
    gpio_init(GPIO_BOTAO_B); gpio_set_dir(GPIO_BOTAO_B, GPIO_IN); gpio_pull_up(GPIO_BOTAO_B);
    gpio_init(GPIO_BOTAO_JOY); gpio_set_dir(GPIO_BOTAO_JOY, GPIO_IN); gpio_pull_up(GPIO_BOTAO_JOY);

    // Inicializa GPIOs de saída
    gpio_init(GPIO_OUT_A); gpio_set_dir(GPIO_OUT_A, GPIO_OUT); gpio_put(GPIO_OUT_A, 1);
    gpio_init(GPIO_OUT_B); gpio_set_dir(GPIO_OUT_B, GPIO_OUT); gpio_put(GPIO_OUT_B, 1);
    gpio_init(GPIO_OUT_JOY); gpio_set_dir(GPIO_OUT_JOY, GPIO_OUT); gpio_put(GPIO_OUT_JOY, 1);

    // Parametros (entrada, saída) para cada tarefa
    static uint params_a[2] = { GPIO_BOTAO_A, GPIO_OUT_A };
    static uint params_b[2] = { GPIO_BOTAO_B, GPIO_OUT_B };
    static uint params_j[2] = { GPIO_BOTAO_JOY, GPIO_OUT_JOY };

    TaskHandle_t th;

    if (xTaskCreate(tarefa_botao, "btnA", 512, params_a, prio, &th) == pdPASS) {
        vTaskCoreAffinitySet(th, core_mask);
    }
    if (xTaskCreate(tarefa_botao, "btnB", 512, params_b, prio, &th) == pdPASS) {
        vTaskCoreAffinitySet(th, core_mask);
    }
    if (xTaskCreate(tarefa_botao, "btnJoy", 512, params_j, prio, &th) == pdPASS) {
        vTaskCoreAffinitySet(th, core_mask);
    }
}

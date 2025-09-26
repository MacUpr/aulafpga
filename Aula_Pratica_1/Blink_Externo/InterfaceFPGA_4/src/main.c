#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "tarefa_onda_quadrada.h"

int main(void) {
    stdio_init_all();

    tarefa_onda_quadrada_cfg_t cfg = {
        .gpio_wave          = 18,
        .gpio_led_r         = 13,
        .gpio_led_g         = 11,
        .gpio_led_b         = 12,
        .led_active_low     = false,

        .task_name          = "onda_sq",
        .task_priority      = (tskIDLE_PRIORITY + 1),
        .task_stack         = 1024,
        .core_affinity_mask = (1u << 1)   // core 1; use 0 para sem afinidade
    };

    tarefa_onda_quadrada_start(&cfg, NULL);
    vTaskStartScheduler();
    for(;;) {}
}

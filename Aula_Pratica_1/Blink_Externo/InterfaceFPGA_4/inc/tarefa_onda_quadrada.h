#ifndef TAREFA_ONDA_QUADRADA_H
#define TAREFA_ONDA_QUADRADA_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Ajuste a frequência desejada da onda quadrada aqui (em Hz)
#define ONDA_FREQUENCIA_HZ   2   // Ex.: 2 Hz -> período total = 0,5 s
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// Configuração da tarefa
typedef struct {
    uint gpio_wave;         // GPIO para onda quadrada (ex.: 18)
    uint gpio_led_r;        // GPIO LED R (ex.: 13)
    uint gpio_led_g;        // GPIO LED G (ex.: 11)
    uint gpio_led_b;        // GPIO LED B (ex.: 12)
    bool led_active_low;    // LED ativo-baixo?

    const char *task_name;
    UBaseType_t task_priority;
    uint16_t    task_stack;
    uint32_t core_affinity_mask; // afinidade opcional
} tarefa_onda_quadrada_cfg_t;

BaseType_t tarefa_onda_quadrada_start(const tarefa_onda_quadrada_cfg_t *cfg, TaskHandle_t *handle_out);
void tarefa_onda_quadrada_stop(TaskHandle_t handle);
void tarefa_onda_quadrada_suspend(TaskHandle_t handle);
void tarefa_onda_quadrada_resume(TaskHandle_t handle);

#endif // TAREFA_ONDA_QUADRADA_H

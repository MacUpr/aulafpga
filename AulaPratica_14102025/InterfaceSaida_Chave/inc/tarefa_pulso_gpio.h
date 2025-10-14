// tarefa_pulso_gpio.h
#ifndef TAREFA_PULSO_GPIO_H
#define TAREFA_PULSO_GPIO_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Ajuste a frequência desejada aqui (em Hz)
#define PULSO_FREQUENCIA_HZ  5.0f
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

typedef struct {
    uint        gpio;               // GPIO que vai gerar o pulso
    const char *task_name;          // nome da task (pode ser NULL)
    UBaseType_t task_priority;      // prioridade
    uint16_t    task_stack;         // stack em words (ex.: 1024)
    uint32_t    core_affinity_mask; // afinidade opcional (0 = default)
} tarefa_pulso_gpio_cfg_t2;

BaseType_t tarefa_pulso_gpio_start (const tarefa_pulso_gpio_cfg_t2 *cfg, TaskHandle_t *handle_out);
void        tarefa_pulso_gpio_stop  (TaskHandle_t handle);
void        tarefa_pulso_gpio_suspend(TaskHandle_t handle);
void        tarefa_pulso_gpio_resume (TaskHandle_t handle);

#endif // TAREFA_PULSO_GPIO_H

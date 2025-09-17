#ifndef TAREFA_BOTOES_IO_H
#define TAREFA_BOTOES_IO_H

#include "FreeRTOS.h"
#include "task.h"

// Entradas (BitDogLab)
#define GPIO_BOTAO_A     5
#define GPIO_BOTAO_B     6
#define GPIO_BOTAO_JOY   22

// Saídas controladas
#define GPIO_OUT_A       16
#define GPIO_OUT_B       17
#define GPIO_OUT_JOY     28

// Cria as três tarefas (núcleo via core_mask)
void criar_tarefas_botoes_io(UBaseType_t prio, UBaseType_t core_mask);

#endif // TAREFA_BOTOES_IO_H

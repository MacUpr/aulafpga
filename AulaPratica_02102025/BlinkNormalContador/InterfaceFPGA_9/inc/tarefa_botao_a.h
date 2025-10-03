#ifndef TAREFA_BOTAO_A_H
#define TAREFA_BOTAO_A_H

#include "FreeRTOS.h"
#include "task.h"

// Entrada/saída
#define GPIO_BOTAO_A   5
#define GPIO_OUT_A    16

void tarefa_botao_a(void *params);
void criar_tarefa_botao_a(UBaseType_t prio, UBaseType_t core_mask);

#endif

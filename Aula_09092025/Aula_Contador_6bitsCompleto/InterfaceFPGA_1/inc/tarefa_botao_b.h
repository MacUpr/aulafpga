#ifndef TAREFA_BOTAO_B_H
#define TAREFA_BOTAO_B_H

#include "FreeRTOS.h"
#include "task.h"

#define GPIO_BOTAO_B   6
#define GPIO_OUT_B    17

void tarefa_botao_b(void *params);
void criar_tarefa_botao_b(UBaseType_t prio, UBaseType_t core_mask);

#endif

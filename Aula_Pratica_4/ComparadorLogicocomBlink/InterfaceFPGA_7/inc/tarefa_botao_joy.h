#ifndef TAREFA_BOTAO_JOY_H
#define TAREFA_BOTAO_JOY_H

#include "FreeRTOS.h"
#include "task.h"

#define GPIO_BOTAO_JOY 22
#define GPIO_OUT_JOY   28

void tarefa_botao_joy(void *params);
void criar_tarefa_botao_joy(UBaseType_t prio, UBaseType_t core_mask);

#endif

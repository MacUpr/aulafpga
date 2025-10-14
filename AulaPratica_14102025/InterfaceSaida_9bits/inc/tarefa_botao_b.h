#ifndef TAREFA_BOTAO_B_H
#define TAREFA_BOTAO_B_H

#include "FreeRTOS.h"
#include "task.h"

// Entrada do botão B (ajuste se necessário)
#define GPIO_BOTAO_B   6

#ifdef __cplusplus
extern "C" {
#endif

void tarefa_botao_b(void *params);
void criar_tarefa_botao_b(UBaseType_t prio, UBaseType_t core_mask);

#ifdef __cplusplus
}
#endif
#endif

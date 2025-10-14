#ifndef TAREFA_BOTAO_A_H
#define TAREFA_BOTAO_A_H

#include "FreeRTOS.h"
#include "task.h"

// Entrada do botão A (ajuste se necessário)
#define GPIO_BOTAO_A   5

#ifdef __cplusplus
extern "C" {
#endif

void tarefa_botao_a(void *params);
void criar_tarefa_botao_a(UBaseType_t prio, UBaseType_t core_mask);

#ifdef __cplusplus
}
#endif
#endif

#ifndef TAREFA_DISPLAY_CHAVES_H
#define TAREFA_DISPLAY_CHAVES_H

#include "FreeRTOS.h"

// Cria a tarefa que mostra no OLED o estado das 7 saídas:
//  GP18 (Botão A), GP19 (Botão B), GP20 (Botão Joy),
//  GP4 (X>=3800), GP9 (X<=500), GP8 (Y>=3800), GP16 (Y<=500).
// A tarefa usa mutex_oled (definido em outro módulo) para desenhar.
#ifdef __cplusplus
extern "C" {
#endif

void criar_tarefa_display_chaves(UBaseType_t prio, UBaseType_t core_mask);

#ifdef __cplusplus
}
#endif

#endif /* TAREFA_DISPLAY_CHAVES_H */

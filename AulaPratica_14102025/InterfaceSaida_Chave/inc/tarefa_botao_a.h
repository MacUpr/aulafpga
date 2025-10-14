#ifndef TAREFA_BOTAO_A_H
#define TAREFA_BOTAO_A_H

#include "FreeRTOS.h"

// ========== Pinos (override se quiser antes do include) ==========
#ifndef GPIO_BOTAO_A
#define GPIO_BOTAO_A   5
#endif
#ifndef PIN_OUT_BTN_A
#define PIN_OUT_BTN_A 18   // SAÍDA do Botão A
#endif

// ========== Configuração de modo ==========
// 0 = TOGGLE (inverte na borda de pressão)
// 1 = PULSE  (1 enquanto pressionado; volta 0 ao soltar)
#ifndef BTN_A_PULSE_MODE
#define BTN_A_PULSE_MODE  0
#endif

// Debounce (ms)
#ifndef BTN_DEBOUNCE_MS
#define BTN_DEBOUNCE_MS   20u
#endif
#ifndef BTN_PERIOD_MS
#define BTN_PERIOD_MS      5u
#endif

#ifdef __cplusplus
extern "C" {
#endif

void criar_tarefa_botao_a(UBaseType_t prio, UBaseType_t core_mask);

#ifdef __cplusplus
}
#endif

#endif /* TAREFA_BOTAO_A_H */

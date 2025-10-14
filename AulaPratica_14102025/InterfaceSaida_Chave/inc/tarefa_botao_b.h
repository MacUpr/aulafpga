#ifndef TAREFA_BOTAO_B_H
#define TAREFA_BOTAO_B_H

#include "FreeRTOS.h"

// Pinos
#ifndef GPIO_BOTAO_B
#define GPIO_BOTAO_B   6
#endif
#ifndef PIN_OUT_BTN_B
#define PIN_OUT_BTN_B 19   // SAÍDA do Botão B
#endif

// Modo: 0=TOGGLE, 1=PULSE
#ifndef BTN_B_PULSE_MODE
#define BTN_B_PULSE_MODE  0
#endif

// Debounce
#ifndef BTN_DEBOUNCE_MS
#define BTN_DEBOUNCE_MS   20u
#endif
#ifndef BTN_PERIOD_MS
#define BTN_PERIOD_MS      5u
#endif

#ifdef __cplusplus
extern "C" {
#endif

void criar_tarefa_botao_b(UBaseType_t prio, UBaseType_t core_mask);

#ifdef __cplusplus
}
#endif

#endif /* TAREFA_BOTAO_B_H */

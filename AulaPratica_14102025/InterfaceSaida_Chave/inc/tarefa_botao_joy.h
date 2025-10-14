#ifndef TAREFA_BOTAO_JOY_H
#define TAREFA_BOTAO_JOY_H

#include "FreeRTOS.h"

// Pinos
#ifndef GPIO_BOTAO_JOY
#define GPIO_BOTAO_JOY 22   // ajuste conforme seu hardware
#endif
#ifndef PIN_OUT_BTN_J
#define PIN_OUT_BTN_J  20   // SAÍDA do Botão do Joystick
#endif

// Modo: 0=TOGGLE, 1=PULSE
#ifndef BTN_JOY_PULSE_MODE
#define BTN_JOY_PULSE_MODE  0
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

void criar_tarefa_botao_joy(UBaseType_t prio, UBaseType_t core_mask);

#ifdef __cplusplus
}
#endif

#endif /* TAREFA_BOTAO_JOY_H */

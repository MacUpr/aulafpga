#ifndef TAREFA_JOYSTICK_EVENTOS_H
#define TAREFA_JOYSTICK_EVENTOS_H

#include "FreeRTOS.h"
#include "task.h"

// Mapeamento das SAÍDAS associadas aos extremos do joystick
#ifndef PIN_OUT_JX_MAX
#define PIN_OUT_JX_MAX   4   // X >= 3800
#endif
#ifndef PIN_OUT_JX_MIN
#define PIN_OUT_JX_MIN   9   // X <= 500
#endif
#ifndef PIN_OUT_JY_MAX
#define PIN_OUT_JY_MAX   8   // Y >= 3800
#endif
#ifndef PIN_OUT_JY_MIN
#define PIN_OUT_JY_MIN  16   // Y <= 500
#endif

// Modos de ação nos extremos: 0=TOGGLE, 1=PULSE
#ifndef JX_MAX_PULSE_MODE
#define JX_MAX_PULSE_MODE  1   // por padrão pulso (ex.: 1 curto)
#endif
#ifndef JX_MIN_PULSE_MODE
#define JX_MIN_PULSE_MODE  1   // por padrão toggle
#endif
#ifndef JY_MAX_PULSE_MODE
#define JY_MAX_PULSE_MODE  1   // pulso
#endif
#ifndef JY_MIN_PULSE_MODE
#define JY_MIN_PULSE_MODE  1  // pulso
#endif

// Duração do pulso (ms) quando *_PULSE_MODE==1
#ifndef JOY_PULSE_MS
#define JOY_PULSE_MS      150u
#endif

// Para o ISR de DMA notificar esta tarefa:
#ifdef __cplusplus
extern "C" {
#endif

extern TaskHandle_t g_hJoyEvt;  // usado em dma_handlers.c

void criar_tarefa_joystick_eventos(UBaseType_t prio, UBaseType_t core_mask);

#ifdef __cplusplus
}
#endif

#endif /* TAREFA_JOYSTICK_EVENTOS_H */

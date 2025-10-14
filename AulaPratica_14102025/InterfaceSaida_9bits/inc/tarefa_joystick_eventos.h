#ifndef TAREFA_JOYSTICK_EVENTOS_H
#define TAREFA_JOYSTICK_EVENTOS_H

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

void criar_tarefa_joystick_eventos(UBaseType_t prio, UBaseType_t core_mask);

// Getter do índice selecionado (0..8) para o display
unsigned joystick_get_sel_bit(void);

#ifdef __cplusplus
}
#endif
#endif

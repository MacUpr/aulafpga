#ifndef TAREFA_JOYSTICK_EVENTOS_H
#define TAREFA_JOYSTICK_EVENTOS_H

#include "FreeRTOS.h"

// Cria a tarefa que processa eventos do joystick (extremos com histerese).
// Ela é acordada pelo ISR do DMA (dma_handler_joy_xy) via xTaskNotifyFromISR.
void criar_tarefa_joystick_eventos(UBaseType_t prio, UBaseType_t core_mask);

#endif

#ifndef JOYSTICK_X_DMA_H
#define JOYSTICK_X_DMA_H

#include <stdint.h>

// Quantas amostras por disparo (mantivemos 3 como no seu processamento)
#define NUM_SAMPLES  3

// Buffer de destino preenchido pelo DMA (lido pela tarefa)
extern uint16_t buffer_joy_x[NUM_SAMPLES];

// Canal DMA usado para X (referenciado pelo ISR)
extern int dma_joy_x;

// Inicializações/arranque
void init_dma_joystick_x(void);
void iniciar_dma_joystick_x(void);

#endif // JOYSTICK_X_DMA_H

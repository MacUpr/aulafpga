#ifndef JOYSTICK_Y_DMA_H
#define JOYSTICK_Y_DMA_H

#include <stdint.h>

#define NUM_SAMPLES  3

extern uint16_t buffer_joy_y[NUM_SAMPLES];
extern int dma_joy_y;

void init_dma_joystick_y(void);
void iniciar_dma_joystick_y(void);

#endif // JOYSTICK_Y_DMA_H

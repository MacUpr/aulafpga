#ifndef JOYSTICK_SETUP_H
#define JOYSTICK_SETUP_H

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa ADC (GPIOs analógicas GP26/GP27) e os canais DMA do joystick.
// Idempotente: se já tiver sido chamado, não faz nada.
void inicializa_joystick_adc_dma(void);

#ifdef __cplusplus
}
#endif

#endif // JOYSTICK_SETUP_H

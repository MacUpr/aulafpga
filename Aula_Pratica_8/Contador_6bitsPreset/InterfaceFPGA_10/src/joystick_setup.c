#include "joystick_setup.h"

#include <stdbool.h>
#include <stdio.h>

#include "hardware/adc.h"
#include "pico/stdlib.h"

// DMAs dos eixos
#include "joystick_x_dma.h"
#include "joystick_y_dma.h"

// RP2040 padrão: X=ADC0->GP26, Y=ADC1->GP27
#define GPIO_ADC_X   26
#define GPIO_ADC_Y   27

void inicializa_joystick_adc_dma(void) {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    // --- ADC base ---
    adc_init();
    adc_gpio_init(GPIO_ADC_X);
    adc_gpio_init(GPIO_ADC_Y);

    // Deixa ADC parado e FIFO drenado; cada início de captura reconfigura
    adc_run(false);
    adc_fifo_drain();
    adc_fifo_setup(false, false, 1, false, false);

    // --- Canais DMA para X e Y ---
    init_dma_joystick_x();
    init_dma_joystick_y();

    printf("[JOY SETUP] ADC em GP26/GP27 e DMAs X/Y inicializados.\n");
}

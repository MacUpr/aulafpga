#include "hardware/adc.h"
#include "hardware/dma.h"
#include "joystick_y_dma.h"

// Buffer de destino (visível para a tarefa)
uint16_t buffer_joy_y[NUM_SAMPLES];

// Canal e config do DMA para Y
int dma_joy_y = 2;
static dma_channel_config cfg_joy_y;

void init_dma_joystick_y(void) {
    // Seleciona o canal analógico Y (ADC1) para garantir estado inicial
    adc_select_input(1);

    // Configuração base do canal DMA
    cfg_joy_y = dma_channel_get_default_config(dma_joy_y);
    channel_config_set_transfer_data_size(&cfg_joy_y, DMA_SIZE_16);
    channel_config_set_read_increment(&cfg_joy_y, false);
    channel_config_set_write_increment(&cfg_joy_y, true);
    channel_config_set_dreq(&cfg_joy_y, DREQ_ADC);

    // Habilita IRQ do canal no grupo 1 (usamos DMA_IRQ_1)
    dma_channel_set_irq1_enabled(dma_joy_y, true);
}

void iniciar_dma_joystick_y(void) {
    // 1) Garanta que o ADC/FIFO estejam estáveis
    adc_run(false);
    adc_fifo_drain();

    adc_fifo_setup(true, true, 1, false, false);

    // Seleciona entrada do Y (ADC1) para esta aquisição
    adc_select_input(1);

    // 2) Arma o DMA SEM iniciar imediatamente
    dma_channel_configure(
        dma_joy_y,
        &cfg_joy_y,
        buffer_joy_y,        // destino
        &adc_hw->fifo,       // origem
        NUM_SAMPLES,         // quantas amostras
        false                // start agora? não
    );

    // 3) Liga o ADC e só então dá start no DMA
    adc_run(true);
    dma_start_channel_mask(1u << dma_joy_y);
}

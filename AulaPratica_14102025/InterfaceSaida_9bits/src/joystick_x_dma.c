#include "hardware/adc.h"
#include "hardware/dma.h"
#include "joystick_x_dma.h"

// Buffer de destino (visível para a tarefa)
uint16_t buffer_joy_x[NUM_SAMPLES];

// Canal e config do DMA para X
int dma_joy_x = 1;
static dma_channel_config cfg_joy_x;

void init_dma_joystick_x(void) {
    // Seleciona o canal analógico X (ADC0) para garantir estado inicial
    adc_select_input(0);

    // Configuração base do canal DMA
    cfg_joy_x = dma_channel_get_default_config(dma_joy_x);
    channel_config_set_transfer_data_size(&cfg_joy_x, DMA_SIZE_16); // 16 bits do ADC
    channel_config_set_read_increment(&cfg_joy_x, false);           // lê sempre do FIFO
    channel_config_set_write_increment(&cfg_joy_x, true);           // escreve ++ no buffer
    channel_config_set_dreq(&cfg_joy_x, DREQ_ADC);                  // throttle pelo ADC

    // Habilita IRQ do canal no grupo 1 (usamos DMA_IRQ_1)
    dma_channel_set_irq1_enabled(dma_joy_x, true);
}

void iniciar_dma_joystick_x(void) {
    // 1) Garanta que o ADC/FIFO estejam estáveis
    adc_run(false);
    adc_fifo_drain();

    // FIFO do ADC: enable=1, dreq=1, thresh=1, err_in_fifo=0, byte_shift=0
    adc_fifo_setup(true, true, 1, false, false);

    // Seleciona entrada do X (ADC0) para esta aquisição
    adc_select_input(0);

    // 2) Arma o DMA SEM iniciar imediatamente
    dma_channel_configure(
        dma_joy_x,
        &cfg_joy_x,
        buffer_joy_x,        // destino
        &adc_hw->fifo,       // origem
        NUM_SAMPLES,         // quantas amostras
        false                // start agora? não
    );

    // 3) Liga o ADC e só então dá start no DMA
    adc_run(true);
    dma_start_channel_mask(1u << dma_joy_x);
}

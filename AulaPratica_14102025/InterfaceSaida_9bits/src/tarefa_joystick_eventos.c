// tarefa_joystick_eventos.c — seleção 0..8 por Y, valor por X (9 bits)
#include "tarefa_joystick_eventos.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Joystick + DMA
#include "joystick_setup.h"
#include "joystick_x_dma.h"
#include "joystick_y_dma.h"

// Palavra de saída (9 bits no projeto)
#include "tarefa_word6.h"

// =================== Integração com o ISR ===================
TaskHandle_t g_hJoyEvt = NULL;      // usado pelo ISR para notificar esta tarefa

// Bits de notificação (devem bater com dma_handlers.c)
#define NOTIF_X_DONE  (1u << 0)
#define NOTIF_Y_DONE  (1u << 1)

// =================== Limiares ===================
// Faixa ADC 12 bits: 0..4095
#define X_SET1_ENTER   3800    // X >= 3800  -> bit(sel) = 1
#define X_SET0_ENTER    500    // X  <  500  -> bit(sel) = 0

// Histerese para Y (navegação)
#define Y_MAX_ENTER    3800
#define Y_MAX_EXIT     3600
#define Y_MIN_ENTER     500
#define Y_MIN_EXIT      700

// Frequência do laço
#define PERIOD_MS      30u     // ~33 Hz

// Quantidade de bits (0..8)
#define SEL_BITS       9u

// =================== Estado/export ===================
// Seleção atual 0..8 (visível para o getter e para evitar otimização agressiva)
static volatile uint8_t s_sel = 0;

// Getter exposto no header (usado pelo display)
unsigned joystick_get_sel_bit(void) {
    return (unsigned)s_sel;
}

// =================== Helpers ===================
static inline uint16_t media3_u16(const uint16_t *buf) {
    return (uint16_t)(((uint32_t)buf[0] + buf[1] + buf[2]) / 3u);
}

// Define APENAS o bit idx (0..8) sem afetar os demais (atomicamente)
static inline void word_set_bit(uint8_t idx, bool one) {
    taskENTER_CRITICAL();
    uint16_t v = (uint16_t)(word6_get() & WORD9_MASK); // WORD9_MASK vem do header
    if (one) v = (uint16_t)(v |  (1u << idx));
    else     v = (uint16_t)(v & ~(1u << idx));
    g_word6_value = v;
    taskEXIT_CRITICAL();
}

// =================== Tarefa ===================
static void tarefa_joystick_eventos_entry(void *arg) {
    (void)arg;
    printf("[JOY EVT] 9 bits: Y navega sel(0..8), X define valor (>=%d →1, <%d →0)\n",
           X_SET1_ENTER, X_SET0_ENTER);

    // Histerese de navegação Y
    bool y_up=false, y_down=false;

    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        // ====== NAVEGAÇÃO (Y) ======
        iniciar_dma_joystick_y();
        uint32_t notif_bits = 0;
        xTaskNotifyWait(0, 0xFFFFFFFFu, &notif_bits, portMAX_DELAY);

        if (notif_bits & NOTIF_Y_DONE) {
            uint16_t y = media3_u16(buffer_joy_y);

            // Avança seleção (0->1->...->8->0) ao entrar no máximo
            if (!y_up && y >= Y_MAX_ENTER) {
                y_up = true;
                s_sel = (uint8_t)((s_sel + 1u) % SEL_BITS);
            } else if (y_up && y < Y_MAX_EXIT) {
                y_up = false;
            }

            // Retrocede seleção (0<-1<-...<-8<-0) ao entrar no mínimo
            if (!y_down && y <= Y_MIN_ENTER) {
                y_down = true;
                s_sel = (uint8_t)((s_sel + SEL_BITS - 1u) % SEL_BITS);
            } else if (y_down && y > Y_MIN_EXIT) {
                y_down = false;
            }
        }

        // ====== VALOR (X) ======
        iniciar_dma_joystick_x();
        notif_bits = 0;
        xTaskNotifyWait(0, 0xFFFFFFFFu, &notif_bits, portMAX_DELAY);

        if (notif_bits & NOTIF_X_DONE) {
            uint16_t x = media3_u16(buffer_joy_x);

            if (x >= X_SET1_ENTER) {
                word_set_bit((uint8_t)s_sel, true);
            } else if (x < X_SET0_ENTER) {
                word_set_bit((uint8_t)s_sel, false);
            }
        }

        vTaskDelay(dt);
    }
}

void criar_tarefa_joystick_eventos(UBaseType_t prio, UBaseType_t core_mask) {
    inicializa_joystick_adc_dma();   // ADC + GPIOs analógicas + inits dos DMAs

    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_joystick_eventos_entry, "JoyEvt_XY_9bits", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);

    // Registra para o ISR de DMA notificar esta tarefa
    g_hJoyEvt = th;
    vTaskCoreAffinitySet(th, core_mask);
}

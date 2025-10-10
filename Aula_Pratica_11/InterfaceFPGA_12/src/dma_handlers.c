#include "hardware/dma.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

// Canais definidos nos módulos X/Y
extern int dma_joy_x;
extern int dma_joy_y;

// Handle da tarefa de eventos (definido em tarefa_joystick_eventos.c)
extern TaskHandle_t g_hJoyEvt;

// Bits de notificação (devem bater com tarefa_joystick_eventos.c)
#define NOTIF_X_DONE  (1u << 0)
#define NOTIF_Y_DONE  (1u << 1)

void dma_handler_joy_xy(void) {
    uint32_t status = dma_hw->ints1;           // IRQ group 1
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t notify_bits = 0;

    if (status & (1u << dma_joy_x)) {
        dma_hw->ints1 = (1u << dma_joy_x);     // limpa IRQ do canal X
        notify_bits |= NOTIF_X_DONE;
    }
    if (status & (1u << dma_joy_y)) {
        dma_hw->ints1 = (1u << dma_joy_y);     // limpa IRQ do canal Y
        notify_bits |= NOTIF_Y_DONE;
    }

    if (notify_bits && g_hJoyEvt) {
        xTaskNotifyFromISR(g_hJoyEvt, notify_bits, eSetBits, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

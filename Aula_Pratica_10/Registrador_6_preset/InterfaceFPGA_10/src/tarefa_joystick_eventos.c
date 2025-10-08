#include "tarefa_joystick_eventos.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Setup/DMAs do joystick
#include "joystick_setup.h"     // inicializa_joystick_adc_dma()
#include "joystick_x_dma.h"     // buffer_joy_x, iniciar_dma_joystick_x()
#include "joystick_y_dma.h"     // buffer_joy_y, iniciar_dma_joystick_y()

// Palavra de 6 bits a ser exibida/escrita nos pinos
#include "tarefa_word6.h"

// =================== Integração com o ISR ===================
TaskHandle_t g_hJoyEvt = NULL;      // usado pelo ISR para notificar esta tarefa

// Bits de notificação (devem bater com dma_handlers.c)
#define NOTIF_X_DONE  (1u << 0)
#define NOTIF_Y_DONE  (1u << 1)

// =================== Parâmetros de histerese =================
// Faixa ADC 12 bits: 0..4095
#define THRESH_MAX_ENTER   3800
#define THRESH_MAX_EXIT    3600
#define THRESH_MIN_ENTER    300
#define THRESH_MIN_EXIT     500

// Frequências/tempos
#define PERIOD_MS   30u         // ~33 Hz entre ciclos de decisão
#define WAIT_MS     50u         // tempo máx para receber cada eixo

// =================== Helpers simétricos ===================
typedef void (*start_dma_fn)(void);

static inline uint16_t media3_u16(const uint16_t *buf) {
    return (uint16_t)(((uint32_t)buf[0] + buf[1] + buf[2]) / 3u);
}

static BaseType_t wait_notify(uint32_t need_bits, TickType_t tmo, uint32_t *got_out) {
    uint32_t got = 0, bits;
    TickType_t t0 = xTaskGetTickCount();
    do {
        TickType_t elapsed = xTaskGetTickCount() - t0;
        if (elapsed >= tmo) break;
        TickType_t rem = tmo - elapsed;
        if (xTaskNotifyWait(0, 0xFFFFFFFFu, &bits, rem) == pdTRUE) got |= bits;
    } while ((got & need_bits) != need_bits);
    if (got_out) *got_out = got;
    return ((got & need_bits) == need_bits) ? pdTRUE : pdFALSE;
}

typedef struct {
    const char     *name;        // "X" ou "Y"
    start_dma_fn    start_dma;   // iniciar_dma_joystick_x/y
    const uint16_t *buf;         // buffer_joy_x/y
    uint32_t        notif_bit;   // NOTIF_X_DONE / NOTIF_Y_DONE
    bool           *st_max;      // histerese máximo
    bool           *st_min;      // histerese mínimo
    uint8_t         bit_max;     // X->4, Y->2
    uint8_t         bit_min;     // X->3, Y->1
} AxisCtx;

static inline void toggle_bit(uint8_t bit) {
    uint8_t v = (uint8_t)(word6_get() & 0x3F);
    v ^= (1u << bit);
    g_word6_value = v;
}

static inline void process_axis(const AxisCtx *a, TickType_t tmo_ms) {
    a->start_dma();                        // dispara captura
    uint32_t got=0;
    if (!wait_notify(a->notif_bit, pdMS_TO_TICKS(tmo_ms), &got)) {
        printf("[JOY %s] TIMEOUT notif=0x%08lx\n", a->name, (unsigned long)got);
        return;
    }
    uint16_t v = media3_u16(a->buf);      // média simples (3 samples)

    // Lógica simétrica de histerese
    if (!*a->st_max && v >= THRESH_MAX_ENTER) { *a->st_max = true;  toggle_bit(a->bit_max); }
    else if (*a->st_max && v < THRESH_MAX_EXIT) { *a->st_max = false; }

    if (!*a->st_min && v <= THRESH_MIN_ENTER) { *a->st_min = true;  toggle_bit(a->bit_min); }
    else if (*a->st_min && v > THRESH_MIN_EXIT) { *a->st_min = false; }
}

// =================== Tarefa ===================
static void tarefa_joystick_eventos_entry(void *arg) {
    (void)arg;
    printf("[JOY EVT] iniciada (IRQ->Notify)  MAX>=%d EXIT<%d | MIN<=%d EXIT>%d\n",
           THRESH_MAX_ENTER, THRESH_MAX_EXIT, THRESH_MIN_ENTER, THRESH_MIN_EXIT);

    // estados persistentes de histerese
    static bool x_max=false, x_min=false, y_max=false, y_min=false;

    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        // Perfis simétricos de X e Y (executados em sequência)
        const AxisCtx AX = {
            .name="X",
            .start_dma = iniciar_dma_joystick_x,
            .buf = buffer_joy_x,
            .notif_bit = NOTIF_X_DONE,
            .st_max = &x_max, .st_min = &x_min,
            .bit_max = 4, .bit_min = 3
        };
        const AxisCtx AY = {
            .name="Y",
            .start_dma = iniciar_dma_joystick_y,
            .buf = buffer_joy_y,
            .notif_bit = NOTIF_Y_DONE,
            .st_max = &y_max, .st_min = &y_min,
            .bit_max = 2, .bit_min = 1
        };

        process_axis(&AX, WAIT_MS);
        process_axis(&AY, WAIT_MS);

        // log 1x/s (snapshot)
        static TickType_t t_last = 0;
        TickType_t now = xTaskGetTickCount();
        if ((now - t_last) >= pdMS_TO_TICKS(1000)) {
            t_last = now;
            printf("[JOY] word=0x%02X  X[max=%d min=%d] Y[max=%d min=%d]\n",
                   (unsigned)word6_get(), x_max, x_min, y_max, y_min);
        }

        vTaskDelay(dt);
    }
}

void criar_tarefa_joystick_eventos(UBaseType_t prio, UBaseType_t core_mask) {
    inicializa_joystick_adc_dma();                 // ADC + GPIOs analógicas + inits dos DMAs

    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_joystick_eventos_entry, "JoyEvt", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    g_hJoyEvt = th;                                // registra p/ ISR
    vTaskCoreAffinitySet(th, core_mask);
}

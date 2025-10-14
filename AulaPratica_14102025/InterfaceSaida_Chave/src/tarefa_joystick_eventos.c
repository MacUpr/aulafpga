#include "tarefa_joystick_eventos.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "task.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Joystick ADC+DMA (X/Y)
#include "joystick_setup.h"     // inicializa_joystick_adc_dma()
#include "joystick_x_dma.h"     // buffer_joy_x, iniciar_dma_joystick_x()
#include "joystick_y_dma.h"     // buffer_joy_y, iniciar_dma_joystick_y()

// Notificações do ISR (devem bater com dma_handlers.c)
TaskHandle_t g_hJoyEvt = NULL;
#define NOTIF_X_DONE  (1u << 0)
#define NOTIF_Y_DONE  (1u << 1)

// Histerese (0..4095)
#ifndef THRESH_MAX_ENTER
#define THRESH_MAX_ENTER   3800
#endif
#ifndef THRESH_MAX_EXIT
#define THRESH_MAX_EXIT    3600
#endif
#ifndef THRESH_MIN_ENTER
#define THRESH_MIN_ENTER    300
#endif
#ifndef THRESH_MIN_EXIT
#define THRESH_MIN_EXIT     500
#endif

#ifndef PERIOD_MS
#define PERIOD_MS           30u
#endif
#ifndef WAIT_MS
#define WAIT_MS             50u
#endif

#ifndef JOY_PULSE_MS
#define JOY_PULSE_MS       120u
#endif

// ===== Helpers =====
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

static inline void cfg_out_zero(uint pin) {
    gpio_init(pin);
    gpio_disable_pulls(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0); // inicia 0
}

static inline void pulse_pin(uint pin, TickType_t ms) {
    gpio_put(pin, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    gpio_put(pin, 0);
}

static inline void toggle_pin(uint pin) {
    gpio_put(pin, !gpio_get_out_level(pin));
}

typedef struct {
    const char     *name;        // "X" / "Y"
    start_dma_fn    start_dma;   // função para disparar DMA
    const uint16_t *buf;         // buffer resultante
    uint32_t        notif_bit;   // bit de notificação
    bool           *st_max;      // estado histerese MAX
    bool           *st_min;      // estado histerese MIN
    uint            pin_max;     // pino associado ao extremo superior
    uint            pin_min;     // pino associado ao extremo inferior
    bool            pulse_max;   // true=pulse, false=toggle
    bool            pulse_min;   // true=pulse, false=toggle
} axis_ctx_t;

static inline void process_axis(const axis_ctx_t *a, TickType_t wait_ms) {
    a->start_dma();
    uint32_t got = 0;
    if (!wait_notify(a->notif_bit, pdMS_TO_TICKS(wait_ms), &got)) {
        // timeout de DMA
        return;
    }

    uint16_t v = media3_u16(a->buf);

    // MAX
    if (!*a->st_max && v >= THRESH_MAX_ENTER) {
        *a->st_max = true;
        if (a->pulse_max) pulse_pin(a->pin_max, JOY_PULSE_MS);
        else              toggle_pin(a->pin_max);
    } else if (*a->st_max && v < THRESH_MAX_EXIT) {
        *a->st_max = false;
    }

    // MIN
    if (!*a->st_min && v <= THRESH_MIN_ENTER) {
        *a->st_min = true;
        if (a->pulse_min) pulse_pin(a->pin_min, JOY_PULSE_MS);
        else              toggle_pin(a->pin_min);
    } else if (*a->st_min && v > THRESH_MIN_EXIT) {
        *a->st_min = false;
    }
}

static void tarefa_joystick_eventos_entry(void *arg) {
    (void)arg;

    // Inicializa ADC/DMA do joystick
    inicializa_joystick_adc_dma();

    // Configura saídas e zera
    cfg_out_zero(PIN_OUT_JX_MAX);
    cfg_out_zero(PIN_OUT_JX_MIN);
    cfg_out_zero(PIN_OUT_JY_MAX);
    cfg_out_zero(PIN_OUT_JY_MIN);

    static bool x_max=false, x_min=false, y_max=false, y_min=false;
    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        const axis_ctx_t AX = {
            .name     = "X",
            .start_dma= iniciar_dma_joystick_x,
            .buf      = buffer_joy_x,
            .notif_bit= NOTIF_X_DONE,
            .st_max   = &x_max, .st_min = &x_min,
            .pin_max  = PIN_OUT_JX_MAX, .pin_min = PIN_OUT_JX_MIN,
            .pulse_max= (JX_MAX_PULSE_MODE ? true : false),
            .pulse_min= (JX_MIN_PULSE_MODE ? true : false),
        };
        const axis_ctx_t AY = {
            .name     = "Y",
            .start_dma= iniciar_dma_joystick_y,
            .buf      = buffer_joy_y,
            .notif_bit= NOTIF_Y_DONE,
            .st_max   = &y_max, .st_min = &y_min,
            .pin_max  = PIN_OUT_JY_MAX, .pin_min = PIN_OUT_JY_MIN,
            .pulse_max= (JY_MAX_PULSE_MODE ? true : false),
            .pulse_min= (JY_MIN_PULSE_MODE ? true : false),
        };

        process_axis(&AX, WAIT_MS);
        process_axis(&AY, WAIT_MS);

        vTaskDelay(dt);
    }
}

void criar_tarefa_joystick_eventos(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_joystick_eventos_entry, "JoyEvt", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    g_hJoyEvt = th; // para o ISR de DMA notificar
    vTaskCoreAffinitySet(th, core_mask);
}

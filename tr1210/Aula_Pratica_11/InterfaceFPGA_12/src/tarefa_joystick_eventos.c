#include "tarefa_joystick_eventos.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

// Setup/DMAs do joystick (ADC+DMA)
#include "joystick_setup.h"     // inicializa_joystick_adc_dma()
#include "joystick_x_dma.h"     // buffer_joy_x, iniciar_dma_joystick_x()
#include "joystick_y_dma.h"     // buffer_joy_y, iniciar_dma_joystick_y()

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

// Duração do pulso (em ms) para B2/B1/B0
#ifndef PULSE_MS
#define PULSE_MS    120u
#endif

// ============ Mapeamento FÍSICO das SAÍDAS do joystick ============
// B2, B1, B0 = pulso; B3 = toggle
#define JOY_OUT_B2   16   // X max  -> pulso (1 por PULSE_MS)
#define JOY_OUT_B1   17   // Y max  -> pulso
#define JOY_OUT_B0   28   // Y min  -> pulso
#define JOY_OUT_B3    8   // X min  -> toggle

// =================== Helpers ===================
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

// ===== GPIO helpers (saídas) =====
static inline void cfg_out(uint pin) {
    gpio_init(pin);
    gpio_disable_pulls(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, 0); // começa em 0
}
static inline void set_pin(uint pin) { gpio_put(pin, 1); }
static inline void clr_pin(uint pin) { gpio_put(pin, 0); }
static inline void tog_pin(uint pin) { gpio_put(pin, !gpio_get_out_level(pin)); }

// Pulso síncrono: segura em 1 por PULSE_MS e volta a 0 (bloqueia só esta tarefa)
static inline void pulse_pin(uint pin, TickType_t pulse_ms) {
    set_pin(pin);
    vTaskDelay(pdMS_TO_TICKS(pulse_ms));
    clr_pin(pin);
}

// Contexto por eixo + modos de ação (pulso/toggle)
typedef struct {
    const char     *name;        // "X" ou "Y"
    start_dma_fn    start_dma;   // iniciar_dma_joystick_x/y
    const uint16_t *buf;         // buffer_joy_x/y
    uint32_t        notif_bit;   // NOTIF_X_DONE / NOTIF_Y_DONE
    bool           *st_max;      // histerese máximo (estado)
    bool           *st_min;      // histerese mínimo (estado)
    uint           pin_max;      // pino associado ao extremo superior
    uint           pin_min;      // pino associado ao extremo inferior
    bool           pulse_max;    // true = pulso; false = toggle
    bool           pulse_min;    // true = pulso; false = toggle
} AxisCtx;

// Lida com um eixo: dispara DMA, espera notificação e aplica histerese + ação
static inline void process_axis(const AxisCtx *a, TickType_t tmo_ms) {
    a->start_dma();                        // dispara captura
    uint32_t got=0;
    if (!wait_notify(a->notif_bit, pdMS_TO_TICKS(tmo_ms), &got)) {
        printf("[JOY %s] TIMEOUT notif=0x%08lx\n", a->name, (unsigned long)got);
        return;
    }
    uint16_t v = media3_u16(a->buf);      // média simples (3 samples)

    // Extremo superior (entra => aciona; sai => apenas desarma)
    if (!*a->st_max && v >= THRESH_MAX_ENTER) {
        *a->st_max = true;
        if (a->pulse_max) pulse_pin(a->pin_max, PULSE_MS);
        else              tog_pin(a->pin_max);
    } else if (*a->st_max && v < THRESH_MAX_EXIT) {
        *a->st_max = false;
    }

    // Extremo inferior (entra => aciona; sai => apenas desarma)
    if (!*a->st_min && v <= THRESH_MIN_ENTER) {
        *a->st_min = true;
        if (a->pulse_min) pulse_pin(a->pin_min, PULSE_MS);
        else              tog_pin(a->pin_min);
    } else if (*a->st_min && v > THRESH_MIN_EXIT) {
        *a->st_min = false;
    }
}

// =================== Tarefa ===================
static void tarefa_joystick_eventos_entry(void *arg) {
    (void)arg;
    printf("[JOY EVT] iniciada  MAX>=%d EXIT<%d | MIN<=%d EXIT>%d | PULSE=%ums\n",
           THRESH_MAX_ENTER, THRESH_MAX_EXIT, THRESH_MIN_ENTER, THRESH_MIN_EXIT, (unsigned)PULSE_MS);

    // Configura SAÍDAS físicas controladas por esta tarefa
    cfg_out(JOY_OUT_B2);
    cfg_out(JOY_OUT_B1);
    cfg_out(JOY_OUT_B0);
    cfg_out(JOY_OUT_B3);

    // estados persistentes de histerese
    static bool x_max=false, x_min=false, y_max=false, y_min=false;

    // Garante nível inicial 0 (pulsos iniciam em 0; toggle também em 0)
    clr_pin(JOY_OUT_B2);
    clr_pin(JOY_OUT_B1);
    clr_pin(JOY_OUT_B0);
    clr_pin(JOY_OUT_B3);

    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        // Perfis: X(direita=max → B2 pulso; esquerda=min → B3 toggle)
        const AxisCtx AX = {
            .name="X",
            .start_dma = iniciar_dma_joystick_x,
            .buf = buffer_joy_x,
            .notif_bit = NOTIF_X_DONE,
            .st_max = &x_max, .st_min = &x_min,
            .pin_max = JOY_OUT_B2, .pin_min = JOY_OUT_B3,
            .pulse_max = true,   // B2 = pulso
            .pulse_min = false   // B3 = toggle
        };
        // Perfis: Y(cima=max → B1 pulso; baixo=min → B0 pulso)
        const AxisCtx AY = {
            .name="Y",
            .start_dma = iniciar_dma_joystick_y,
            .buf = buffer_joy_y,
            .notif_bit = NOTIF_Y_DONE,
            .st_max = &y_max, .st_min = &y_min,
            .pin_max = JOY_OUT_B1, .pin_min = JOY_OUT_B0,
            .pulse_max = true,   // B1 = pulso
            .pulse_min = true    // B0 = pulso
        };

        process_axis(&AX, WAIT_MS);
        process_axis(&AY, WAIT_MS);

        // log 1x/s (snapshot básico dos níveis de saída)
        static TickType_t t_last = 0;
        TickType_t now = xTaskGetTickCount();
        if ((now - t_last) >= pdMS_TO_TICKS(1000)) {
            t_last = now;
            printf("[JOY] OUTS  B3(GP8)=%d B2(GP16)=%d B1(GP17)=%d B0(GP28)=%d\n",
                   gpio_get_out_level(JOY_OUT_B3),
                   gpio_get_out_level(JOY_OUT_B2),
                   gpio_get_out_level(JOY_OUT_B1),
                   gpio_get_out_level(JOY_OUT_B0));
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

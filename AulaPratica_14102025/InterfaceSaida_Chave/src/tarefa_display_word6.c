#include "tarefa_display_word6.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "oled_display.h"
#include "oled_context.h"
#include "ssd1306_text.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// ======= Entradas (word5) =======
#define PIN_M1     18  // linha 0  -> Motor M1 ON/OFF
#define PIN_M2     19  // linha 2  -> Motor M2 ON/OFF
#define PIN_CRONO  20  // linha 4  -> "Cronômetro" piscando
#define PIN_SYS     4  // linha 6  -> Sistema ON/OFF
#define PIN_HB      9  // linha 8  -> "Heartbeat" piscando

// (opcional) configurar como entrada com pull-up aqui
#ifndef DISPLAY_INPUT_PULLUP
#define DISPLAY_INPUT_PULLUP  1
#endif

// período de varredura do display
#ifndef PERIOD_MS
#define PERIOD_MS  40u
#endif

// extern (definido em outro módulo)
extern SemaphoreHandle_t mutex_oled;

// ---------- helpers ----------
static inline void cfg_input_pull(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
#if DISPLAY_INPUT_PULLUP
    gpio_pull_up(pin);
#else
    gpio_disable_pulls(pin);
#endif
}

static inline uint8_t rd(uint pin) {
    return (uint8_t)(gpio_get(pin) & 1u);
}

static inline void safe_draw_line(uint8_t line_idx, const char *txt) {
    // cada "linha" é 8px de altura
    const uint8_t y = (uint8_t)(line_idx * 8);
    if (y + 7u < oled.height) {
        ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, y, txt,
                                    (uint8_t)oled.width, (uint8_t)oled.height);
    }
}

// ---------- tela ----------
static void render_tela(bool m1, bool m2, bool sys,
                        bool show_crono, bool show_hb)
{
    oled_clear(&oled);

    // linha 0: M1
    safe_draw_line(0, m1 ? "Motor M1 ON" : "Motor M1 OFF");

    // linha 2: M2
    safe_draw_line(1, m2 ? "Motor M2 ON" : "Motor M2 OFF");

    // linha 4: Cronômetro (pisca)
    if (show_crono) safe_draw_line(3, "Cron\xC3\xB4metro Ativo"); // "Cronômetro" UTF-8

    // linha 6: Sistema
    safe_draw_line(5, sys ? "Sistema ON" : "Sistema OFF");

    // linha 8: Heartbeat (pisca)
    if (show_hb) safe_draw_line(7, "Heartbeat");

    oled_render(&oled);
}

static void task_display_word6(void *arg)
{
    (void)arg;
    printf("[OLED] tarefa_display_word6 iniciada\n");

    // configura entradas
    cfg_input_pull(PIN_M1);
    cfg_input_pull(PIN_M2);
    cfg_input_pull(PIN_CRONO);
    cfg_input_pull(PIN_SYS);
    cfg_input_pull(PIN_HB);

    // estados anteriores e visibilidades dos “blinks”
    uint8_t last_m1 = rd(PIN_M1);
    uint8_t last_m2 = rd(PIN_M2);
    uint8_t last_sys = rd(PIN_SYS);
    uint8_t last_cr = rd(PIN_CRONO);
    uint8_t last_hb = rd(PIN_HB);

    bool show_crono = true; // começam visíveis
    bool show_hbeat = true;

    // força 1a renderização
    bool dirty = true;

    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        // lê pinos
        uint8_t m1  = rd(PIN_M1);
        uint8_t m2  = rd(PIN_M2);
        uint8_t sys = rd(PIN_SYS);
        uint8_t cr  = rd(PIN_CRONO);
        uint8_t hb  = rd(PIN_HB);

        // detecta bordas para piscar (sincronizado ao sinal)
        if (cr != last_cr) {
            last_cr = cr;
            show_crono = !show_crono;
            dirty = true;
        }
        if (hb != last_hb) {
            last_hb = hb;
            show_hbeat = !show_hbeat;
            dirty = true;
        }

        // mudanças de estado ON/OFF dos motores/sistema
        if (m1 != last_m1) { last_m1 = m1; dirty = true; }
        if (m2 != last_m2) { last_m2 = m2; dirty = true; }
        if (sys != last_sys){ last_sys = sys; dirty = true; }

        if (dirty) {
            if (xSemaphoreTake(mutex_oled, pdMS_TO_TICKS(100))) {
                render_tela(m1, m2, sys, show_crono, show_hbeat);
                xSemaphoreGive(mutex_oled);
            }
            dirty = false;
        }

        vTaskDelay(dt);
    }
}

void criar_tarefa_display_word6(UBaseType_t prio, UBaseType_t core_mask)
{
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_display_word6, "disp6", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

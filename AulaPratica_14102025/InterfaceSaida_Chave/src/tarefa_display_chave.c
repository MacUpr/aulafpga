// tarefa_display_chaves.c — mostra no OLED o ESTADO (0/1) das 7 saídas
#include "tarefa_display_chaves.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "oled_display.h"
#include "oled_context.h"
#include "ssd1306_text.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// ====== Mapeamento das 7 saídas (ajuste se necessário) ======
#ifndef PIN_OUT_BTN_A
#define PIN_OUT_BTN_A   18   // Botão A
#endif
#ifndef PIN_OUT_BTN_B
#define PIN_OUT_BTN_B   19   // Botão B
#endif
#ifndef PIN_OUT_BTN_J
#define PIN_OUT_BTN_J   20   // Botão Joy (push)
#endif
#ifndef PIN_OUT_JX_MAX
#define PIN_OUT_JX_MAX   4   // Joystick X >= 3800
#endif
#ifndef PIN_OUT_JX_MIN
#define PIN_OUT_JX_MIN   9   // Joystick X <= 500
#endif
#ifndef PIN_OUT_JY_MAX
#define PIN_OUT_JY_MAX   8   // Joystick Y >= 3800
#endif
#ifndef PIN_OUT_JY_MIN
#define PIN_OUT_JY_MIN  16   // Joystick Y <= 500
#endif

// Período de varredura do display
#ifndef DISP_PERIOD_MS
#define DISP_PERIOD_MS  80u
#endif

// Vem de oled_context.c (ou onde você criou o mutex no main)
extern SemaphoreHandle_t mutex_oled;

// ---- helpers de texto/linha ----
static inline void draw_line(uint8_t line_idx, const char *txt) {
    const uint8_t y = (uint8_t)(line_idx * 8);
    if (y + 7u < oled.height) {
        ssd1306_draw_utf8_multiline(
            oled.ram_buffer, 0, y, txt,
            (uint8_t)oled.width, (uint8_t)oled.height
        );
    }
}

static inline const char* onoff(uint8_t lvl) { return lvl ? "1" : "0"; }

// Render completo da tela
static void render_status(const uint8_t lv[7]) {
    oled_clear(&oled);

    // Linha 0: título
    draw_line(0, "SELECIONE"); // "SAÍDAS" em UTF-8 (I acentuado)

    char buf[24];

    // Linhas 1..7: Chave 1..7
    // 1: Botão A
    snprintf(buf, sizeof(buf), "Chave 1: %s", onoff(lv[0]));
    draw_line(1, buf);

    // 2: Botão B
    snprintf(buf, sizeof(buf), "Chave 2: %s", onoff(lv[1]));
    draw_line(2, buf);

    // 3: Botão Joy
    snprintf(buf, sizeof(buf), "Chave 3: %s", onoff(lv[2]));
    draw_line(3, buf);

    // 4: X Alto
    snprintf(buf, sizeof(buf), "Chave 4: %s", onoff(lv[3]));
    draw_line(4, buf);

    // 5: X Baixo
    snprintf(buf, sizeof(buf), "Chave 5: %s", onoff(lv[4]));
    draw_line(5, buf);

    // 6: Y Alto
    snprintf(buf, sizeof(buf), "Chave 6: %s", onoff(lv[5]));
    draw_line(6, buf);

    // 7: Y Baixo
    snprintf(buf, sizeof(buf), "Chave 7: %s", onoff(lv[6]));
    draw_line(7, buf);

    oled_render(&oled);
}

static void task_display_chaves(void *arg) {
    (void)arg;
    printf("[OLED] Display estados das 7 chaves\n");

    // snapshot inicial (força 1a render)
    uint8_t last[7];
    for (int i = 0; i < 7; ++i) last[i] = 0xFF;

    const TickType_t dt = pdMS_TO_TICKS(DISP_PERIOD_MS);

    for (;;) {
        uint8_t cur[7];
        cur[0] = (uint8_t)gpio_get_out_level(PIN_OUT_BTN_A);  // Chave 1
        cur[1] = (uint8_t)gpio_get_out_level(PIN_OUT_BTN_B);  // Chave 2
        cur[2] = (uint8_t)gpio_get_out_level(PIN_OUT_BTN_J);  // Chave 3
        cur[3] = (uint8_t)gpio_get_out_level(PIN_OUT_JX_MAX); // Chave 4
        cur[4] = (uint8_t)gpio_get_out_level(PIN_OUT_JX_MIN); // Chave 5
        cur[5] = (uint8_t)gpio_get_out_level(PIN_OUT_JY_MAX); // Chave 6
        cur[6] = (uint8_t)gpio_get_out_level(PIN_OUT_JY_MIN); // Chave 7

        bool changed = false;
        for (int i = 0; i < 7; ++i) {
            if (cur[i] != last[i]) { changed = true; break; }
        }

        if (changed) {
            for (int i = 0; i < 7; ++i) last[i] = cur[i];
            if (xSemaphoreTake(mutex_oled, pdMS_TO_TICKS(100))) {
                render_status(cur);
                xSemaphoreGive(mutex_oled);
            }
        }

        vTaskDelay(dt);
    }
}

void criar_tarefa_display_chaves(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_display_chaves, "disp_chaves", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

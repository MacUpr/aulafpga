#include "tarefa_display_word6.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "oled_display.h"
#include "oled_context.h"
#include "numeros_medios.h"
#include "ssd1306_text.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef GPIO_OUT_B
#define GPIO_OUT_B 17   // único pino de saída mostrado (LOAD via Botão B)
#endif

extern volatile uint8_t g_word6_value;

// ====== Render de dígitos MEDIOS (16x24) nas páginas 5..7 ======
#define LARGURA_MED       16
#define ALTURA_POR_PAG     8
#define PAGINAS_USADAS     3
#define PAGINA_DESTINO     5

static inline const char* bit_txt(bool level) { return level ? "1" : "0"; }

static void desenhar_cabecalho(void)
{
    uint8_t *buf  = oled.ram_buffer;
    uint8_t w_pix = (uint8_t)oled.width;
    uint8_t h_pix = (uint8_t)oled.height;
    ssd1306_draw_utf8_multiline(buf, 0, 0*8, "WORD6 (B5..B0)", w_pix, h_pix);
}

static void desenhar_estado_load(void)
{
    const bool outB = gpio_get_out_level(GPIO_OUT_B);
    uint8_t *buf  = oled.ram_buffer;
    uint8_t w_pix = (uint8_t)oled.width;
    uint8_t h_pix = (uint8_t)oled.height;

    ssd1306_draw_utf8_multiline(buf, 0, 2*8, "LOAD:", w_pix, h_pix);
    ssd1306_draw_utf8_multiline(buf, 48, 2*8, bit_txt(outB), w_pix, h_pix);
}

static void exibir_bitmap_medio(ssd1306_t *d, uint8_t x, const uint8_t *bmp)
{
    for (uint8_t col = 0; col < LARGURA_MED; col++) {
        for (uint8_t page = 0; page < PAGINAS_USADAS; page++) {
            size_t index = (size_t)(col * ALTURA_POR_PAG + (PAGINA_DESTINO + page));
            size_t pos   = (size_t)((PAGINA_DESTINO + page) * d->width + (x + col));
            if (index < (LARGURA_MED * ALTURA_POR_PAG) && pos < d->bufsize) {
                d->ram_buffer[pos] = bmp[index];
            }
        }
    }
}

static inline void exibir_digito_medio_01(ssd1306_t *d, char c, uint8_t x)
{
    const uint8_t *bmp = NULL;
    if (c == '0') bmp = numeros_medios[0];
    else if (c == '1') bmp = numeros_medios[1];
    if (bmp) exibir_bitmap_medio(d, x, bmp);
}

// MSB→LSB (B5..B0)
static void montar_bits_msb2lsb(uint8_t v, char out6[7])
{
    out6[0] = (v & (1u<<5)) ? '1' : '0';
    out6[1] = (v & (1u<<4)) ? '1' : '0';
    out6[2] = (v & (1u<<3)) ? '1' : '0';
    out6[3] = (v & (1u<<2)) ? '1' : '0';
    out6[4] = (v & (1u<<1)) ? '1' : '0';
    out6[5] = (v & (1u<<0)) ? '1' : '0';
    out6[6] = '\0';
}

static void desenhar_bits_medios(uint8_t v)
{
    char bits[7];
    montar_bits_msb2lsb(v, bits);

    oled_clear(&oled);
    desenhar_cabecalho();
    desenhar_estado_load();

    const uint8_t largura = LARGURA_MED;
    const uint8_t gap     = 2;
    const uint8_t total_w = (uint8_t)(6 * largura + 5 * gap);
    uint8_t x0 = (oled.width > total_w) ? (uint8_t)((oled.width - total_w) / 2) : 0;

    uint8_t x = x0;
    for (int i = 0; i < 6; i++) {
        exibir_digito_medio_01(&oled, bits[i], x);  // bits[0]=B5 ... bits[5]=B0
        x = (uint8_t)(x + largura + gap);
    }
    oled_render(&oled);
}

#define PERIOD_MS  80u

static inline uint16_t snapshot_word_load(void) {
    uint16_t snap = (uint16_t)(g_word6_value & 0x3Fu); // bits 0..5
    const bool outB  = gpio_get_out_level(GPIO_OUT_B);
    snap |= (uint16_t)(outB ? 1u : 0u) << 12;
    return snap;
}

static void task_display_word6(void *arg)
{
    (void)arg;
    printf("[OLED] tarefa_display_word6 iniciada\n");

    uint16_t ultimo = 0xFFFF;
    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        uint16_t snap = snapshot_word_load();
        if (snap != ultimo) {
            ultimo = snap;
            if (xSemaphoreTake(mutex_oled, pdMS_TO_TICKS(100))) {
                desenhar_bits_medios((uint8_t)(snap & 0x3Fu));
                xSemaphoreGive(mutex_oled);
            }
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

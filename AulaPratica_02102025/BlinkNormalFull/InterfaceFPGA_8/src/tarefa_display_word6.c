// src/tarefa_display_word6.c
#include "tarefa_display_word6.h"

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "oled_display.h"
#include "oled_context.h"
#include "numeros_medios.h"       // numeros_medios[10][128]
#include "ssd1306_text.h"         // buffer*, x, y, str, width, height
#include "hardware/gpio.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// ===== Valores globais vindos da tarefa de leitura =====
extern volatile uint8_t  g_word6_value;  // bits 0..5 (compat)
extern volatile uint16_t g_word9_value;  // bits 0..8

// ======= Render de dígitos MEDIOS (16x24) nas páginas 5..7 =======
#define LARGURA_MED     16
#define ALTURA_POR_PAG   8
#define PAGINAS_USADAS   3
#define PAGINA_DESTINO   5   // desenha nas páginas 5,6,7 (parte inferior)

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

// Constrói string de 6 bits na ordem LSB→MSB (b0..b5)
static void montar_bits_lsb2msb(uint8_t v, char out6[7])
{
    out6[0] = (v & (1u<<0)) ? '1' : '0';
    out6[1] = (v & (1u<<1)) ? '1' : '0';
    out6[2] = (v & (1u<<2)) ? '1' : '0';
    out6[3] = (v & (1u<<3)) ? '1' : '0';
    out6[4] = (v & (1u<<4)) ? '1' : '0';
    out6[5] = (v & (1u<<5)) ? '1' : '0';
    out6[6] = '\0';
}

static inline const char* bit_txt(bool level) { return level ? "1" : "0"; }

// ---- Cabeçalhos nas páginas 0,1,2: mostram Out_A/B/C vindos dos bits 6,7,8 ----
static void desenhar_cabecalhos_out_from_bits(uint16_t v9)
{
    const bool outA = ((v9 >> 6) & 1u) != 0; // bit6
    const bool outB = ((v9 >> 7) & 1u) != 0; // bit7
    const bool outC = ((v9 >> 8) & 1u) != 0; // bit8

    uint8_t *buf  = oled.ram_buffer;
    uint8_t w_pix = (uint8_t)oled.width;
    uint8_t h_pix = (uint8_t)oled.height;

    // páginas 0,1,2 (y = página*8)
    ssd1306_draw_utf8_multiline(buf, 0, 0*8, "Red: ", w_pix, h_pix);
    ssd1306_draw_utf8_multiline(buf, 72,0*8, bit_txt(outA), w_pix, h_pix);

    ssd1306_draw_utf8_multiline(buf, 0, 1*8, "Green: ", w_pix, h_pix);
    ssd1306_draw_utf8_multiline(buf, 72,1*8, bit_txt(outB), w_pix, h_pix);

    ssd1306_draw_utf8_multiline(buf, 0, 2*8, "Blue: ", w_pix, h_pix);
    ssd1306_draw_utf8_multiline(buf, 72,2*8, bit_txt(outC), w_pix, h_pix);

    ssd1306_draw_utf8_multiline(buf, 0, 4*8, "BitDogLab_NOVE", w_pix, h_pix);
}

static void desenhar_bits_medios(uint8_t v6, uint16_t v9)
{
    char bits[7];
    montar_bits_lsb2msb(v6, bits); // bits[0]=b0(LSB) ... bits[5]=b5(MSB)

    oled_clear(&oled);

    // Linhas com estados de Out_A/B/C (vindos de b6..b8)
    desenhar_cabecalhos_out_from_bits(v9);

    // Centraliza os 6 dígitos (páginas 5..7)
    const uint8_t largura = LARGURA_MED;     // 16
    const uint8_t gap     = 2;               // 2 px
    const uint8_t total_w = (6 * largura) + (5 * gap);
    uint8_t x0 = (oled.width > total_w) ? (uint8_t)((oled.width - total_w) / 2) : 0;

    uint8_t x = x0;
    for (int i = 0; i < 6; i++) {
        // Desenhar MSB→LSB da esquerda para a direita
        exibir_digito_medio_01(&oled, bits[5 - i], x);
        x = (uint8_t)(x + largura + gap);
    }

    oled_render(&oled);
}

// ======= Tarefa =======
#define PERIOD_MS  80u

// Snapshot: junta os 6 bits de entrada (0..5) e os três "Out_*" (6..8)
static inline uint32_t snapshot_word_outs(void) {
    // leitura atômica simples (volátil); para RP2040 é suficiente aqui
    uint16_t v9 = g_word9_value;
    uint8_t  v6 = (uint8_t)(v9 & 0x3Fu);
    return ((uint32_t)v9 << 8) | v6; // empacota para comparar mudanças (v9|v6)
}

static void task_display_word6(void *arg)
{
    (void)arg;
    printf("[OLED] tarefa_display_word6 iniciada\n");

    uint32_t ultimo = 0xFFFFFFFFu;
    TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        uint16_t cur9 = g_word9_value;            // bits 0..8
        uint8_t  cur6 = (uint8_t)(cur9 & 0x3Fu);  // bits 0..5

        uint32_t snap = ((uint32_t)cur9 << 8) | cur6;

        if (snap != ultimo) {
            ultimo = snap;
            if (xSemaphoreTake(mutex_oled, pdMS_TO_TICKS(100))) {
                desenhar_bits_medios(cur6, cur9);
                xSemaphoreGive(mutex_oled);
            }
        }
        vTaskDelay(dt);
    }
}

void criar_tarefa_display_word6(UBaseType_t prio, UBaseType_t core_mask)
{
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_display_word6, "disp6",
                                1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

// src/tarefa_display_word6.c
#include "tarefa_display_word6.h"

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "oled_display.h"
#include "oled_context.h"
#include "numeros_medios.h"       // numeros_medios[10][128]
#include "ssd1306_text.h"         // ssd1306_draw_utf8_multiline(...)
#include "hardware/gpio.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// -------- Fallback de pinos (só usados se não estiverem definidos em headers do projeto) --------
#ifndef GPIO_BOTAO_A
#define GPIO_BOTAO_A 5
#endif
#ifndef GPIO_BOTAO_B
#define GPIO_BOTAO_B 6
#endif
#ifndef GPIO_BOTAO_JOY
#define GPIO_BOTAO_JOY 22
#endif

#ifndef GPIO_OUT_A
#define GPIO_OUT_A  16
#endif
#ifndef GPIO_OUT_B
#define GPIO_OUT_B  17
#endif
#ifndef GPIO_OUT_JOY
#define GPIO_OUT_JOY 28
#endif

// Valor global lido pela tarefa de entrada (definido em tarefa_word6.c)
extern volatile uint8_t g_word6_value;

// ======= Render de dígitos MEDIOS (16x24) nas páginas 5..7 =======
#define LARGURA_MED     16
#define ALTURA_POR_PAG   8
#define PAGINAS_USADAS   3
#define PAGINA_DESTINO   5   // desenha nas páginas 5,6,7 (parte inferior)

// ---- utilidade: texto do bit ----
static inline const char* bit_txt(bool level) {
    return level ? "1" : "0";
}

// ---- Cabeçalho superior informando o modo TOGGLE ----
static void desenhar_cabecalho_toggle(void)
{
    uint8_t *buf  = oled.ram_buffer;
    uint8_t w_pix = (uint8_t)oled.width;
    uint8_t h_pix = (uint8_t)oled.height;

    // Página 0 (y = 0). Mensagem com acento (UTF-8).
    ssd1306_draw_utf8_multiline(buf, 0, 0, "BOTÕES A/B/JOY: TOGGLE", w_pix, h_pix);
}

// ---- Linhas mostrando o ESTADO das GPIOs de SAÍDA (0/1) ----
// Colocadas a partir da página 1 para não sobrepor o cabeçalho.
static void desenhar_cabecalhos_saidas(void)
{
    const bool outA  = gpio_get_out_level(GPIO_OUT_A);
    const bool outB  = gpio_get_out_level(GPIO_OUT_B);
    const bool outJ  = gpio_get_out_level(GPIO_OUT_JOY);

    uint8_t *buf  = oled.ram_buffer;
    uint8_t w_pix = (uint8_t)oled.width;
    uint8_t h_pix = (uint8_t)oled.height;

    // páginas 1,2,3 (y = página*8); ASCII/UTF-8 simples
    ssd1306_draw_utf8_multiline(buf, 0, 1*8, "GPIO_16:", w_pix, h_pix);
    ssd1306_draw_utf8_multiline(buf, 70,1*8, bit_txt(outA), w_pix, h_pix);

    ssd1306_draw_utf8_multiline(buf, 0, 2*8, "GPIO_17:", w_pix, h_pix);
    ssd1306_draw_utf8_multiline(buf, 70,2*8, bit_txt(outB), w_pix, h_pix);

    ssd1306_draw_utf8_multiline(buf, 0, 3*8, "GPIO_28:", w_pix, h_pix);
    ssd1306_draw_utf8_multiline(buf, 70,3*8, bit_txt(outJ), w_pix, h_pix);

    // Rodapé (opcional) na página 4
    ssd1306_draw_utf8_multiline(buf, 0, 4*8, "BitDogLab_TOGGLE", w_pix, h_pix);
}

static void exibir_bitmap_medio(ssd1306_t *d, uint8_t x, const uint8_t *bmp)
{
    // bmp tem 16 colunas * 8 bytes/coluna = 128 bytes (páginas 0..7)
    // Copiamos apenas as páginas 5..7 para o buffer do display.
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

static void desenhar_bits_medios(uint8_t v)
{
    char bits[7];
    montar_bits_lsb2msb(v, bits); // bits[0]=b0(LSB) ... bits[5]=b5(MSB)

    oled_clear(&oled);

    // Cabeçalho superior
    desenhar_cabecalho_toggle();

    // Linhas com estados das SAÍDAS (páginas 1..3) + rodapé (página 4)
    desenhar_cabecalhos_saidas();

    // Centraliza os 6 dígitos (páginas 5..7)
    const uint8_t largura = LARGURA_MED;     // 16
    const uint8_t gap     = 2;               // 2 px
    const uint8_t total_w = (uint8_t)(6 * largura + 5 * gap);
    uint8_t x0 = (oled.width > total_w) ? (uint8_t)((oled.width - total_w) / 2) : 0;

    uint8_t x = x0;
    for (int i = 0; i < 6; i++) {
        // Desenhar MSB→LSB da esquerda para a direita (bits[5] ... bits[0])
        exibir_digito_medio_01(&oled, bits[5 - i], x);
        x = (uint8_t)(x + largura + gap);
    }

    oled_render(&oled);
}

// ======= Tarefa =======
#define PERIOD_MS  80u

// Snapshot combinado: word6 (bits 0..5) + ESTADOS DAS SAÍDAS (bits 12..14)
// Mantém refresh mesmo se apenas as saídas mudarem.
static inline uint16_t snapshot_word_saidas(void) {
    uint16_t snap = (uint16_t)g_word6_value; // 0..5
    const bool outA  = gpio_get_out_level(GPIO_OUT_A);
    const bool outB  = gpio_get_out_level(GPIO_OUT_B);
    const bool outJ  = gpio_get_out_level(GPIO_OUT_JOY);
    snap |= (uint16_t)(outA ? 1u : 0u) << 12;
    snap |= (uint16_t)(outB ? 1u : 0u) << 13;
    snap |= (uint16_t)(outJ ? 1u : 0u) << 14;
    return snap;
}

static void task_display_word6(void *arg)
{
    (void)arg;
    printf("[OLED] tarefa_display_word6 iniciada\n");

    uint16_t ultimo = 0xFFFF;
    TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        uint16_t snap = snapshot_word_saidas();
        uint8_t v = (uint8_t)(snap & 0xFF); // apenas os 6 bits para os dígitos

        if (snap != ultimo) {
            ultimo = snap;
            if (xSemaphoreTake(mutex_oled, pdMS_TO_TICKS(100))) {
                desenhar_bits_medios(v);
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

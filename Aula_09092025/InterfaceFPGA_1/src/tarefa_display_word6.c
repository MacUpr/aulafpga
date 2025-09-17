// src/tarefa_display_word6.c
#include "tarefa_display_word6.h"

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "oled_display.h"
#include "oled_context.h"
#include "numeros_medios.h"       // numeros_medios[10][128]
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Valor global lido pela tarefa de entrada (definido em tarefa_word6.c)
extern volatile uint8_t g_word6_value;

// ======= Render de dígitos MEDIOS (16x24) nas páginas 5..7 =======
#define LARGURA_MED     16
#define ALTURA_POR_PAG   8
#define PAGINAS_USADAS   3
#define PAGINA_DESTINO   5   // desenha nas páginas 5,6,7 (parte inferior)

static void exibir_bitmap_medio(ssd1306_t *d, uint8_t x, const uint8_t *bmp)
{
    // bmp contém 16 colunas * 8 bytes/coluna = 128 bytes
    // Vamos “colar” essas 16 colunas nas 3 páginas alvo (5..7),
    // pegando das fatias correspondentes: bytes [5], [6], [7] de cada coluna.
    for (uint8_t col = 0; col < LARGURA_MED; col++) {
        for (uint8_t page = 0; page < PAGINAS_USADAS; page++) {
            // byte dentro do bitmap fonte
            size_t index = (size_t)(col * ALTURA_POR_PAG + (PAGINA_DESTINO + page));
            // posição dentro do buffer do OLED
            size_t pos   = (size_t)((PAGINA_DESTINO + page) * d->width + (x + col));
            if (index < (LARGURA_MED * ALTURA_POR_PAG) && pos < d->bufsize) {
                d->ram_buffer[pos] = bmp[index];
            }
        }
    }
}

static inline void exibir_digito_medio_01(ssd1306_t *d, char c, uint8_t x)
{
    // Apenas '0' e '1' (mas mantemos a possibilidade de ampliar depois)
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
    montar_bits_lsb2msb(v, bits);

    oled_clear(&oled);

    // Centralização simples horizontal:
    // 6 dígitos * 16px = 96px. Display 128px → margem de 16px de cada lado (separação 16px/2)
    // Vamos dar um pequeno espaçamento de 2px entre dígitos.
    const uint8_t largura = LARGURA_MED;    // 16
    const uint8_t gap     = 2;              // 2 px entre dígitos
    const uint8_t total_w = (6 * largura) + (5 * gap); // 6 dígitos + 5 gaps
    uint8_t x0 = (oled.width > total_w) ? (uint8_t)((oled.width - total_w) / 2) : 0;

    uint8_t x = x0;
    for (int i = 0; i < 6; i++) {
        exibir_digito_medio_01(&oled, bits[i], x);
        x = (uint8_t)(x + largura + gap);
    }

    oled_render(&oled);
}

// ======= Tarefa =======
#define PERIOD_MS  80u

static void task_display_word6(void *arg)
{
    (void)arg;
    printf("[OLED] tarefa_display_word6 iniciada\n");

    uint8_t ultimo = 0xFF;
    TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        // snapshot atômico (oito bits cabem num acesso atômico no RP2040)
        uint8_t v = g_word6_value;

        if (v != ultimo) {
            ultimo = v;
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

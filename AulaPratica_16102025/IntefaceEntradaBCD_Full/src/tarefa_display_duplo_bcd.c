// tarefa_display_duplo_bcd.c — Exibe "MC:DU" com 4 dígitos grandes
// Agora lê diretamente os globais da palavra separada por MSB:
//   g_hh_tens(=M), g_hh_units(=C), g_mm_tens(=D), g_mm_units(=U), g_hhmm_valid

#include "tarefa_display_duplo_bcd.h"

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "oled_display.h"
#include "oled_context.h"
#include "ssd1306_text.h"
#include "numeros_grandes.h"
#include "digitos_grandes_utils.h"

// <<< Removido: #include "tarefa_bcd8.h"
// Use os globais publicados pela tarefa_word_9.c:
extern volatile uint8_t g_hh_tens;   // M (0..2)
extern volatile uint8_t g_hh_units;  // C (0..3)
extern volatile uint8_t g_mm_tens;   // D (0..5)
extern volatile uint8_t g_mm_units;  // U (0..9)
extern volatile bool    g_hhmm_valid;

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Disponibilizados pelo seu projeto
extern SemaphoreHandle_t mutex_oled;
extern ssd1306_t oled;

#ifndef DISP_DUPLO_PERIOD_MS
#define DISP_DUPLO_PERIOD_MS 10u
#endif

// Largura do glyph "grande" (ajuste se sua fonte tiver outra largura)
#ifndef DIGITO_GRANDE_W
#define DIGITO_GRANDE_W 25u
#endif

// Espaço mínimo entre elementos (dígitos e ':')
#ifndef DIGITO_GRANDE_GAP
#define DIGITO_GRANDE_GAP 4u
#endif

// Largura reservada ao ':' quando desenhado como caractere 6x8 (escala 1x)
#ifndef COLON_W
#define COLON_W 6u
#endif

// Desenha ':' com a fonte de texto; usa ssd1306_draw_char(buffer, x, y, ch, w, h)
static void draw_colon_text(uint8_t x)
{
    // Altura típica de char ~8 px; centraliza aproximadamente
    uint8_t y = 0;
    if (oled.height > 8) {
        y = (uint8_t)((oled.height - 8) / 2);
    }
    // largura=1, altura=1 (escala 1x); 1 = “cor acesa” não é parâmetro aqui
    ssd1306_draw_char(oled.ram_buffer, (int16_t)x, (int16_t)y, (uint8_t)':', 1, 1);
}

static void desenhar_mc_du(uint8_t M, uint8_t C,
                           uint8_t D, uint8_t U,
                           bool valido)
{
    oled_clear(&oled);

    // Cabeçalho opcional: mostra validade da leitura
    ssd1306_draw_utf8_multiline(
        oled.ram_buffer, 0, 0,
        valido ? "HH:MM" : "INV",
        oled.width, oled.height
    );

    const uint8_t W   = (uint8_t)DIGITO_GRANDE_W;
    const uint8_t GAP = (uint8_t)DIGITO_GRANDE_GAP;
    const uint8_t CW  = (uint8_t)COLON_W;

    // Layout horizontal: M |G| C |G| ':' |G| D |G| U
    const uint16_t total_w = (uint16_t)(W + GAP + W + GAP + CW + GAP + W + GAP + W);

    uint8_t x0 = 0;
    if (oled.width > total_w) x0 = (uint8_t)((oled.width - total_w) / 2);

    const uint8_t xM   = x0;
    const uint8_t xC   = (uint8_t)(xM + W + GAP);
    const uint8_t xCol = (uint8_t)(xC + W + GAP);
    const uint8_t xD   = (uint8_t)(xCol + CW + GAP);
    const uint8_t xU   = (uint8_t)(xD + W + GAP);

    // Fallback simples se algo vier fora de 0..9
    const bool ok_bmps = (M < 10u) && (C < 10u) && (D < 10u) && (U < 10u);

    const uint8_t *bmpM = (valido && ok_bmps) ? numeros_grandes[M] : numeros_grandes[0];
    const uint8_t *bmpC = (valido && ok_bmps) ? numeros_grandes[C] : numeros_grandes[0];
    const uint8_t *bmpD = (valido && ok_bmps) ? numeros_grandes[D] : numeros_grandes[0];
    const uint8_t *bmpU = (valido && ok_bmps) ? numeros_grandes[U] : numeros_grandes[0];

    exibir_digito_grande(&oled, xM,  bmpM);
    exibir_digito_grande(&oled, xC,  bmpC);
    draw_colon_text(xCol);
    exibir_digito_grande(&oled, xD,  bmpD);
    exibir_digito_grande(&oled, xU,  bmpU);

    oled_render(&oled);
}

static void task_display_duplo_bcd(void *arg)
{
    (void)arg;
    printf("[OLED] Exibindo MC:DU (4 dígitos grandes)\n");

    // Estado para evitar redesenho desnecessário
    uint8_t uM=0xFF, uC=0xFF, uD=0xFF, uU=0xFF;
    bool    uOK=false;

    const TickType_t dt = pdMS_TO_TICKS(DISP_DUPLO_PERIOD_MS);

    for (;;) {
        vTaskDelay(dt);

        // Snapshot dos globais publicados pela leitura de 9b separada por MSB
        uint8_t M = g_hh_tens;   // milhar (0..2)
        uint8_t C = g_hh_units;  // centena (0..3)
        uint8_t D = g_mm_tens;   // dezena  (0..5)
        uint8_t U = g_mm_units;  // unidade (0..9)
        bool    ok = g_hhmm_valid;

        if (M!=uM || C!=uC || D!=uD || U!=uU || ok!=uOK) {
            uM=M; uC=C; uD=D; uU=U; uOK=ok;

            if (xSemaphoreTake(mutex_oled, pdMS_TO_TICKS(100))) {
                desenhar_mc_du(M, C, D, U, ok);
                xSemaphoreGive(mutex_oled);
            }

            printf("[OLED] %s  HH:MM = %u%u:%u%u\n",
                   ok ? "OK" : "INV", M, C, D, U);
        }
    }
}

void criar_tarefa_display_duplo_bcd(UBaseType_t prio, UBaseType_t core_mask)
{
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_display_duplo_bcd, "disp_duplo_bcd", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

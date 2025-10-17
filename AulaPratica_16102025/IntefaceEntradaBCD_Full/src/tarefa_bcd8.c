// tarefa_bcd8.c — Decodifica 8 LSB de UMA das palavras 9b explícitas
// Escolha a FONTE: MINUTOS (MSB=0) ou HORAS (MSB=1)
#define BCD8_FROM_MINUTES   0
#define BCD8_FROM_HOURS     1

#ifndef BCD8_FROM
#define BCD8_FROM BCD8_FROM_MINUTES   // padrão: minutos (MSB=0)
#endif

#include "tarefa_bcd8.h"
#include "tarefa_word_9.h"   // expõe g_word9_lo, g_word9_hi (e word9_get/WORD9_MASK se quiser)

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef BCD8_PERIOD_MS
#define BCD8_PERIOD_MS 50u  // taxa de verificação/atualização
#endif

// Publicações globais (LEGADO — mantidas para compatibilidade)
volatile uint8_t g_bcd_tens    = 0;    // 0..9 quando válido
volatile uint8_t g_bcd_units   = 0;    // 0..9 quando válido
volatile uint8_t g_bcd_decimal = 0xFF; // 0..99 se válido; 0xFF se inválido
volatile bool    g_bcd_valid   = false;

static inline bool bcd_range_ok(uint8_t dez, uint8_t und) {
#if (BCD8_FROM == BCD8_FROM_MINUTES)
    // MINUTOS: 00..59
    return (dez <= 5u) && (und <= 9u);
#elif (BCD8_FROM == BCD8_FROM_HOURS)
    // HORAS “HH” do seu protocolo (milhar: 0..2, centena: 0..3) → 0..23
    // Aqui tratamos dezena/unidade “à moda antiga”: limite 0..23
    // (dezena <= 2 e, se dezena == 2, unidade <= 3)
    if (dez > 2u) return false;
    if ((dez == 2u) && (und > 3u)) return false;
    return true;
#else
# error "BCD8_FROM inválido"
#endif
}

static inline uint8_t read_source_byte(void) {
#if (BCD8_FROM == BCD8_FROM_MINUTES)
    extern volatile uint16_t g_word9_lo; // MSB=0
    return (uint8_t)(g_word9_lo & 0xFFu);
#elif (BCD8_FROM == BCD8_FROM_HOURS)
    extern volatile uint16_t g_word9_hi; // MSB=1
    return (uint8_t)(g_word9_hi & 0xFFu);
#endif
}

static void task_bcd8(void *arg) {
    (void)arg;

    // (Opcional) Espera USB para logs
    for (int i = 0; i < 50 && !stdio_usb_connected(); ++i) {
        printf("[BCD8] aguardando USB...\n");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
#if (BCD8_FROM == BCD8_FROM_MINUTES)
    printf("[BCD8] fonte = MINUTOS (MSB=0) — range esperado: 00..59\n");
#else
    printf("[BCD8] fonte = HORAS   (MSB=1) — range esperado: 00..23\n");
#endif

    uint8_t ultimo_b = 0xFF;
    const TickType_t dt = pdMS_TO_TICKS(BCD8_PERIOD_MS);

    for (;;) {
        vTaskDelay(dt);

        // Snapshot da FONTE explícita (alinhado/atômico em 16b no M0+)
        uint8_t b = read_source_byte();

        if (b != ultimo_b) {
            ultimo_b = b;

            uint8_t dez = (uint8_t)((b >> 4) & 0x0Fu);
            uint8_t und = (uint8_t)( b       & 0x0Fu);

            bool valido = bcd_range_ok(dez, und);

            // Publicações (legado)
            g_bcd_tens    = dez;
            g_bcd_units   = und;
            g_bcd_valid   = valido;
            g_bcd_decimal = valido ? (uint8_t)(dez * 10u + und) : 0xFFu;

            // Log de depuração
            printf("[BCD8] byte=0x%02X  dez=%u  und=%u  %s  dec=%s\n",
                   b, dez, und, valido ? "OK" : "INV",
                   valido ? "atualizado" : "—");
        }
    }
}

void criar_tarefa_bcd8(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_bcd8, "bcd8", 768, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

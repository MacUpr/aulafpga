// tarefa_word_9.c — LÊ 9 bits (B0..B8) das GPIOs e publica palavras separadas por MSB
// Mapa LSB→MSB: B0=GP18, B1=GP19, B2=GP20, B3=GP4, B4=GP9, B5=GP8, B6=GP16, B7=GP17, B8=GP28
//
// Regras:
//  - MSB=0  -> word_lo  = {MSB=0, D(=mm_tens)[3:0], U(=mm_units)[3:0]}
//  - MSB=1  -> word_hi  = {MSB=1, M(=hh_tens)[3:0], C(=hh_units)[3:0]}
//  - g_hhmm_valid = (HH e MM dentro dos ranges)
// g_word9_value mantém a última palavra lida (lo ou hi), por compatibilidade.

#include "tarefa_word_9.h"

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/gpio.h"
#include "hardware/regs/sio.h"
#include "hardware/structs/sio.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define PERIOD_MS        10u      // período entre varreduras (mais rápido)
#define SCAN_WINDOW_MS    5u      // janela curta para capturar lo/hi multiplexados

#ifndef WORD9_FAST_SIO
#define WORD9_FAST_SIO 1
#endif

// ---------------------- Publicações globais ----------------------
volatile uint16_t g_word9_value = 0;   // última palavra (lo/hi)
volatile uint16_t g_word9_lo    = 0;   // última palavra MSB=0
volatile uint16_t g_word9_hi    = 0;   // última palavra MSB=1

volatile uint8_t  g_mm_units = 0;      // 0..9
volatile uint8_t  g_mm_tens  = 0;      // 0..5
volatile uint8_t  g_hh_units = 0;      // 0..3
volatile uint8_t  g_hh_tens  = 0;      // 0..2

volatile bool     g_hhmm_valid = false;

// Configuração
static bool s_word9_pullup_enabled = true;

// Pinos B0..B8 (LSB->MSB)
static const uint8_t GPIO_WORD_PINS[9] = {
    GPIO_WORD_B0, GPIO_WORD_B1, GPIO_WORD_B2, GPIO_WORD_B3, GPIO_WORD_B4,
    GPIO_WORD_B5, GPIO_WORD_B6, GPIO_WORD_B7, GPIO_WORD_B8
};

// --- API compat ---
uint16_t word9_get(void) {
    return (uint16_t)(g_word9_value & WORD9_MASK);
}

// --- Utilidades ---
static inline void config_input(uint pin, bool pullup) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_disable_pulls(pin);
    if (pullup) gpio_pull_up(pin);
}

static void print_bits9(uint16_t v) {
    printf("BIN: %c%c%c%c%c%c%c%c%c  (MSB..LSB)\n",
           (v&(1u<<8))?'1':'0',
           (v&(1u<<7))?'1':'0',
           (v&(1u<<6))?'1':'0',
           (v&(1u<<5))?'1':'0',
           (v&(1u<<4))?'1':'0',
           (v&(1u<<3))?'1':'0',
           (v&(1u<<2))?'1':'0',
           (v&(1u<<1))?'1':'0',
           (v&(1u<<0))?'1':'0');
}

static inline uint16_t read_word9_now(void) {
#if WORD9_FAST_SIO
    uint32_t in = sio_hw->gpio_in;
    uint16_t v  = 0;
    v |= (uint16_t)(((in >> GPIO_WORD_B0) & 1u) << 0);
    v |= (uint16_t)(((in >> GPIO_WORD_B1) & 1u) << 1);
    v |= (uint16_t)(((in >> GPIO_WORD_B2) & 1u) << 2);
    v |= (uint16_t)(((in >> GPIO_WORD_B3) & 1u) << 3);
    v |= (uint16_t)(((in >> GPIO_WORD_B4) & 1u) << 4);
    v |= (uint16_t)(((in >> GPIO_WORD_B5) & 1u) << 5);
    v |= (uint16_t)(((in >> GPIO_WORD_B6) & 1u) << 6);
    v |= (uint16_t)(((in >> GPIO_WORD_B7) & 1u) << 7);
    v |= (uint16_t)(((in >> GPIO_WORD_B8) & 1u) << 8);
    return v;
#else
    uint16_t v = 0;
    for (uint8_t i = 0; i < 9; i++) {
        v |= (uint16_t)((gpio_get(GPIO_WORD_PINS[i]) & 1u) << i);
    }
    return v;
#endif
}

static inline bool hhmm_ranges_ok(void) {
    bool mm_ok = (g_mm_tens <= 5u) && (g_mm_units <= 9u);
    bool hh_ok = (g_hh_tens <= 2u) && (g_hh_units <= 3u);
    return (mm_ok && hh_ok);
}

// --- Tarefa ---
static void task_word9_in(void *pv) {
    (void)pv;

    for (int i = 0; i < 50 && !stdio_usb_connected(); ++i) {
        printf("[Word9-IN] aguardando USB...\n");
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    printf("[Word9-IN] pull-up %s\n", s_word9_pullup_enabled ? "ON" : "OFF");
    printf("[Word9-IN] Pinos LSB->MSB: B0=GP%u B1=GP%u B2=GP%u B3=GP%u B4=GP%u B5=GP%u B6=GP%u B7=GP%u B8=GP%u\n",
           GPIO_WORD_PINS[0], GPIO_WORD_PINS[1], GPIO_WORD_PINS[2], GPIO_WORD_PINS[3], GPIO_WORD_PINS[4],
           GPIO_WORD_PINS[5], GPIO_WORD_PINS[6], GPIO_WORD_PINS[7], GPIO_WORD_PINS[8]);

    for (uint8_t i = 0; i < 9; i++) {
        config_input(GPIO_WORD_PINS[i], s_word9_pullup_enabled);
    }

    // Snapshot inicial
    g_word9_value = (uint16_t)(read_word9_now() & WORD9_MASK);
    printf("[Word9-IN] inicial: 0x%03X  ", g_word9_value);
    print_bits9(g_word9_value);

    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);
    TickType_t lastBeat = xTaskGetTickCount();
    const TickType_t beat = pdMS_TO_TICKS(1000);

    for (;;) {
        // === Varredura curta para capturar lo/hi mesmo se multiplexado rapidamente ===
        absolute_time_t until = make_timeout_time_ms(SCAN_WINDOW_MS);

        bool     got_lo = false, got_hi = false;
        uint16_t last_lo = g_word9_lo;
        uint16_t last_hi = g_word9_hi;

        while (!time_reached(until)) {
            uint16_t v = (uint16_t)(read_word9_now() & WORD9_MASK);
            uint8_t msb = (uint8_t)((v >> 8) & 1u);

            if (msb == 0u) {
                last_lo = v;
                got_lo  = true;
            } else {
                last_hi = v;
                got_hi  = true;
            }
        }

        // Publica o que foi visto nesta janela
        if (got_lo) {
            g_word9_lo  = last_lo;
            uint8_t u   = (uint8_t)( last_lo        & 0x0Fu);
            uint8_t d   = (uint8_t)((last_lo >> 4)  & 0x0Fu);
            g_mm_units  = u;     // 0..9
            g_mm_tens   = d;     // 0..5
        }

        if (got_hi) {
            g_word9_hi  = last_hi;
            uint8_t c   = (uint8_t)( last_hi        & 0x0Fu);
            uint8_t m   = (uint8_t)((last_hi >> 4)  & 0x0Fu);
            g_hh_units  = c;     // 0..3
            g_hh_tens   = m;     // 0..2
        }

        // Mantém g_word9_value como "última palavra vista" (prioriza hi se apareceu)
        if (got_hi)      g_word9_value = last_hi;
        else if (got_lo) g_word9_value = last_lo;

        // Atualiza validade
        g_hhmm_valid = hhmm_ranges_ok();

        // Logs mínimos (mostra se capturou lo/hi)
        if (got_lo || got_hi) {
            printf("[Word9-IN] janela %ums: %s%s  HH=%u%u  MM=%u%u  %s\n",
                   (unsigned)SCAN_WINDOW_MS,
                   got_hi ? "HI " : "",
                   got_lo ? "LO"  : "",
                   (unsigned)g_hh_tens, (unsigned)g_hh_units,
                   (unsigned)g_mm_tens, (unsigned)g_mm_units,
                   g_hhmm_valid ? "OK" : "INV");
        } else {
            printf("[Word9-IN] janela %ums: (nada visto)\n", (unsigned)SCAN_WINDOW_MS);
        }

        // heartbeat
        TickType_t now = xTaskGetTickCount();
        if ((now - lastBeat) >= beat) {
            lastBeat = now;
            uint16_t cur = word9_get();
            printf("[Word9-IN] atual: 0x%03X  ", cur);
            print_bits9(cur);
            printf("          HH=%u%u  MM=%u%u  %s\n",
                   (unsigned)g_hh_tens, (unsigned)g_hh_units,
                   (unsigned)g_mm_tens, (unsigned)g_mm_units,
                   g_hhmm_valid ? "OK" : "INV");
        }

        vTaskDelay(dt);
    }
}

// Criador
void criar_tarefa_word9(UBaseType_t prio, UBaseType_t core_mask, bool use_pullup) {
    s_word9_pullup_enabled = use_pullup;
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_word9_in, "word9_in", 768, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

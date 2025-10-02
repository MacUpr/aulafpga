// tarefa_word6.c — agora lê 9 bits (B0..B8), com B6..B8 = GP16,GP17,GP18 (pull-up)

#include "tarefa_word6.h"
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

// ~20 Hz -> 50 ms por amostra; confirmação com 2 amostras => ~100 ms
#define PERIOD_MS 10u

// ====== Mapeamento de pinos ======
// B0..B5 devem vir do seu header (GPIO_WORD_B0..GPIO_WORD_B5).
// Se ainda NÃO houver mapeamento para B6..B8, define aqui:
#ifndef GPIO_WORD_B6
#define GPIO_WORD_B6 16
#endif
#ifndef GPIO_WORD_B7
#define GPIO_WORD_B7 17
#endif
#ifndef GPIO_WORD_B8
#define GPIO_WORD_B8 28
#endif

// Valor “histórico” de 6 bits (compatível com módulos existentes)
volatile uint8_t  g_word6_value = 0;
// Novo: valor completo de 9 bits (armazenado em 16 bits)
volatile uint16_t g_word9_value = 0;

static inline void config_input(uint pin, bool use_pullup) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_disable_pulls(pin);
    if (use_pullup) gpio_pull_up(pin);
}

// Lê imediatamente os 9 bits B0..B8
static inline uint16_t read_word9_now(void) {
    uint16_t b0 = gpio_get(GPIO_WORD_B0) & 1u;
    uint16_t b1 = gpio_get(GPIO_WORD_B1) & 1u;
    uint16_t b2 = gpio_get(GPIO_WORD_B2) & 1u;
    uint16_t b3 = gpio_get(GPIO_WORD_B3) & 1u;
    uint16_t b4 = gpio_get(GPIO_WORD_B4) & 1u;
    uint16_t b5 = gpio_get(GPIO_WORD_B5) & 1u;
    uint16_t b6 = gpio_get(GPIO_WORD_B6) & 1u;
    uint16_t b7 = gpio_get(GPIO_WORD_B7) & 1u;
    uint16_t b8 = gpio_get(GPIO_WORD_B8) & 1u;

    return (uint16_t)((b0<<0) | (b1<<1) | (b2<<2) | (b3<<3) | (b4<<4) | (b5<<5)
                    | (b6<<6) | (b7<<7) | (b8<<8));
}

typedef struct { bool use_pullup; } word6_cfg_t; // mantém nome/assinatura

// Impressão dos 9 bits (MSB..LSB)
static void print_bits9(uint16_t v9) {
    // B8..B0
    printf("BIN9: %c%c%c %c%c%c %c%c%c  (B8..B0)\n",
           (v9&(1u<<8))?'1':'0',
           (v9&(1u<<7))?'1':'0',
           (v9&(1u<<6))?'1':'0',
           (v9&(1u<<5))?'1':'0',
           (v9&(1u<<4))?'1':'0',
           (v9&(1u<<3))?'1':'0',
           (v9&(1u<<2))?'1':'0',
           (v9&(1u<<1))?'1':'0',
           (v9&(1u<<0))?'1':'0');
}

// Impressão dos 6 LSB (compatível com logs antigos)
static void print_bits6(uint8_t v) {
    printf("BIN6: %c%c%c%c%c%c  (B5..B0)\n",
           (v&(1u<<5))?'1':'0',
           (v&(1u<<4))?'1':'0',
           (v&(1u<<3))?'1':'0',
           (v&(1u<<2))?'1':'0',
           (v&(1u<<1))?'1':'0',
           (v&(1u<<0))?'1':'0');
}

static void task_word6(void *pv) {
    word6_cfg_t *cfg = (word6_cfg_t*)pv;

    // *** Força PULL-UP, mantendo a API original ***
    cfg->use_pullup = true;

    printf("[Word9] Tarefa iniciando... (pullup=%s)\n", cfg->use_pullup ? "ON" : "OFF (Alta-Z)");
    printf("[Word9] Mapeamento pinos LSB->MSB:\n");
    printf("       B0=GP%d  B1=GP%d  B2=GP%d  B3=GP%d  B4=GP%d  B5=GP%d  B6=GP%d  B7=GP%d  B8=GP%d\n",
           GPIO_WORD_B0, GPIO_WORD_B1, GPIO_WORD_B2, GPIO_WORD_B3, GPIO_WORD_B4, GPIO_WORD_B5,
           GPIO_WORD_B6, GPIO_WORD_B7, GPIO_WORD_B8);

    for (int i = 0; i < 50 && !stdio_usb_connected(); ++i) {  // ~0,5 s
        printf("[Word9] Aguardando USB CDC conectar...\n");
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Entradas com pull-up (forçado)
    config_input(GPIO_WORD_B0, cfg->use_pullup);
    config_input(GPIO_WORD_B1, cfg->use_pullup);
    config_input(GPIO_WORD_B2, cfg->use_pullup);
    config_input(GPIO_WORD_B3, cfg->use_pullup);
    config_input(GPIO_WORD_B4, cfg->use_pullup);
    config_input(GPIO_WORD_B5, cfg->use_pullup);
    config_input(GPIO_WORD_B6, cfg->use_pullup); // <<< novos
    config_input(GPIO_WORD_B7, cfg->use_pullup);
    config_input(GPIO_WORD_B8, cfg->use_pullup);

    uint16_t last9 = read_word9_now();
    g_word9_value = last9;
    g_word6_value = (uint8_t)(last9 & 0x3Fu);  // mantém compatibilidade com 6 bits

    printf("[Word9] Leitura inicial: 9b=0x%03X  6b=0x%02X\n", g_word9_value, g_word6_value);
    print_bits9(g_word9_value);
    print_bits6(g_word6_value);

    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);
    TickType_t t_last_beat = xTaskGetTickCount();
    const TickType_t beat  = pdMS_TO_TICKS(1000); // 1 s

    for (;;) {
        vTaskDelay(dt);
        uint16_t v1 = read_word9_now();
        vTaskDelay(dt);
        uint16_t v2 = read_word9_now();

        if (v1 == v2 && v2 != last9) {
            last9 = v2;
            __asm volatile ("" ::: "memory");
            g_word9_value = last9;
            g_word6_value = (uint8_t)(last9 & 0x3Fu);

            printf("[Word9] MUDOU: 9b=0x%03X  6b=0x%02X  ", g_word9_value, g_word6_value);
            print_bits9(g_word9_value);
        }

        TickType_t now = xTaskGetTickCount();
        if ((now - t_last_beat) >= beat) {
            t_last_beat = now;

            uint16_t cur9 = g_word9_value;
            uint8_t  cur6 = g_word6_value;

            printf("[Word9] Atual: 9b=0x%03X  6b=0x%02X  ", cur9, cur6);
            print_bits9(cur9);

            printf("[Word9] Raw pinos: "
                   "B0(GP%d)=%d  B1(GP%d)=%d  B2(GP%d)=%d  B3(GP%d)=%d  B4(GP%d)=%d  B5(GP%d)=%d  "
                   "B6(GP%d)=%d  B7(GP%d)=%d  B8(GP%d)=%d\n",
                   GPIO_WORD_B0, gpio_get(GPIO_WORD_B0)&1,
                   GPIO_WORD_B1, gpio_get(GPIO_WORD_B1)&1,
                   GPIO_WORD_B2, gpio_get(GPIO_WORD_B2)&1,
                   GPIO_WORD_B3, gpio_get(GPIO_WORD_B3)&1,
                   GPIO_WORD_B4, gpio_get(GPIO_WORD_B4)&1,
                   GPIO_WORD_B5, gpio_get(GPIO_WORD_B5)&1,
                   GPIO_WORD_B6, gpio_get(GPIO_WORD_B6)&1,
                   GPIO_WORD_B7, gpio_get(GPIO_WORD_B7)&1,
                   GPIO_WORD_B8, gpio_get(GPIO_WORD_B8)&1);
        }
    }
}

void criar_tarefa_word6(UBaseType_t prio, UBaseType_t core_mask, bool use_pullup) {
    static word6_cfg_t cfg;
    cfg.use_pullup = use_pullup;   // mantém compatibilidade com chamadas antigas

    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_word6, "word9", 896, &cfg, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

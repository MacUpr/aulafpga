#include "tarefa_word6.h"
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>

// ~20 Hz -> 50 ms por amostra; confirmação com 2 amostras => ~100 ms
#define PERIOD_MS 50u

volatile uint8_t g_word6_value = 0;

static inline void config_input(uint pin, bool use_pullup) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_disable_pulls(pin);
    if (use_pullup) gpio_pull_up(pin);  // senão, fica alta-Z
}

static inline uint8_t read_word6_now(void) {
    uint8_t b0 = gpio_get(GPIO_WORD_B0) & 1u;
    uint8_t b1 = gpio_get(GPIO_WORD_B1) & 1u;
    uint8_t b2 = gpio_get(GPIO_WORD_B2) & 1u;
    uint8_t b3 = gpio_get(GPIO_WORD_B3) & 1u;
    uint8_t b4 = gpio_get(GPIO_WORD_B4) & 1u;
    uint8_t b5 = gpio_get(GPIO_WORD_B5) & 1u;
    return (uint8_t)((b0<<0) | (b1<<1) | (b2<<2) | (b3<<3) | (b4<<4) | (b5<<5));
}

typedef struct { bool use_pullup; } word6_cfg_t;

static void print_bits(uint8_t v) {
    printf("BIN: %c%c%c%c%c%c  (MSB..LSB)\n",
           (v&(1u<<5))?'1':'0',
           (v&(1u<<4))?'1':'0',
           (v&(1u<<3))?'1':'0',
           (v&(1u<<2))?'1':'0',
           (v&(1u<<1))?'1':'0',
           (v&(1u<<0))?'1':'0');
}

static void task_word6(void *pv) {
    word6_cfg_t *cfg = (word6_cfg_t*)pv;

    // Mensagem de “vida”
    printf("[Word6] Tarefa iniciando... (pullup=%s)\n", cfg->use_pullup ? "ON" : "OFF (Alta-Z)");
    printf("[Word6] Mapeamento pinos LSB->MSB: B0=GP%d  B1=GP%d  B2=GP%d  B3=GP%d  B4=GP%d  B5=GP%d\n",
           GPIO_WORD_B0, GPIO_WORD_B1, GPIO_WORD_B2, GPIO_WORD_B3, GPIO_WORD_B4, GPIO_WORD_B5);

    // Se USB ainda não conectou, avisa periodicamente (evita “silêncio” confuso)
    for (int i = 0; i < 50 && !stdio_usb_connected(); ++i) {  // ~0,5 s
        printf("[Word6] Aguardando USB CDC conectar...\n");
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Configura entradas
    config_input(GPIO_WORD_B0, cfg->use_pullup);
    config_input(GPIO_WORD_B1, cfg->use_pullup);
    config_input(GPIO_WORD_B2, cfg->use_pullup);
    config_input(GPIO_WORD_B3, cfg->use_pullup);
    config_input(GPIO_WORD_B4, cfg->use_pullup);
    config_input(GPIO_WORD_B5, cfg->use_pullup);

    uint8_t last = read_word6_now();
    g_word6_value = last;

    // Log inicial detalhado
    printf("[Word6] Leitura inicial: 0x%02X\n", g_word6_value);
    print_bits(g_word6_value);

    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);
    TickType_t t_last_beat = xTaskGetTickCount();
    const TickType_t beat  = pdMS_TO_TICKS(1000); // 1 s

    for (;;) {
        vTaskDelay(dt);
        uint8_t v1 = read_word6_now();
        vTaskDelay(dt);
        uint8_t v2 = read_word6_now();

        // Atualiza só quando 2 amostras iguais e diferente do last
        if (v1 == v2 && v2 != last) {
            last = v2;
            __asm volatile ("" ::: "memory");
            g_word6_value = last;

            printf("[Word6] MUDOU: 0x%02X  ", g_word6_value);
            print_bits(g_word6_value);
        }

        // Batimento: imprime 1x por segundo mesmo sem mudanças
        TickType_t now = xTaskGetTickCount();
        if ((now - t_last_beat) >= beat) {
            t_last_beat = now;
            uint8_t cur = g_word6_value;
            printf("[Word6] Atual: 0x%02X  ", cur);
            print_bits(cur);

            // Também imprime os níveis brutos por pino (útil p/ diagnosticar flutuação/ligação)
            printf("[Word6] Raw pinos: B0(GP%d)=%d  B1(GP%d)=%d  B2(GP%d)=%d  B3(GP%d)=%d  B4(GP%d)=%d  B5(GP%d)=%d\n",
                   GPIO_WORD_B0, gpio_get(GPIO_WORD_B0)&1,
                   GPIO_WORD_B1, gpio_get(GPIO_WORD_B1)&1,
                   GPIO_WORD_B2, gpio_get(GPIO_WORD_B2)&1,
                   GPIO_WORD_B3, gpio_get(GPIO_WORD_B3)&1,
                   GPIO_WORD_B4, gpio_get(GPIO_WORD_B4)&1,
                   GPIO_WORD_B5, gpio_get(GPIO_WORD_B5)&1);
        }
    }
}

void criar_tarefa_word6(UBaseType_t prio, UBaseType_t core_mask, bool use_pullup) {
    static word6_cfg_t cfg;  // vida útil estática
    cfg.use_pullup = use_pullup;

    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_word6, "word6", 768, &cfg, prio, &th); // stack um pouco maior
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

// tarefa_word6.c — agora espelha 9 bits (B0..B8) nas GPIOs:
// Mapa LSB→MSB: B0=GP18, B1=GP19, B2=GP20, B3=GP4, B4=GP9, B5=GP8, B6=GP16, B7=GP17, B8=GP28
#include "tarefa_word_9.h"

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define PERIOD_MS 50u

// Mantemos o nome para compatibilidade com o restante do código.
// Agora usamos os 9 bits menos significativos (0..8).
volatile uint16_t g_word6_value = 0;  // 0..0x1FF (B0..B8)

// Tabela de pinos na ordem B0..B8
static const uint8_t GPIO_WORD_PINS[9] = {
    18, // B0
    19, // B1
    20, // B2
     4, // B3
     9, // B4
     8, // B5
    16, // B6
    17, // B7
    28  // B8
};

static inline void config_output(uint pin) {
    gpio_init(pin);
    gpio_disable_pulls(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

static inline void write_word9_now(uint16_t v) {
    // v usa apenas bits 0..8
    for (uint8_t i = 0; i < 9; i++) {
        const uint8_t bit_on = (uint8_t)((v >> i) & 1u);
        gpio_put(GPIO_WORD_PINS[i], bit_on ? 1 : 0); // ativo-alto
    }
}

static void print_bits9(uint16_t v) {
    // imprime MSB..LSB (B8..B0)
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

static void task_word6_out(void *pv) {
    (void)pv;

    for (int i = 0; i < 50 && !stdio_usb_connected(); ++i) {
        printf("[Word9-OUT] aguardando USB...\n");
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    printf("[Word9-OUT] iniciando escrita ativo-alto\n");
    printf("[Word9-OUT] Pinos LSB->MSB:\n");
    printf("  B0=GP%u  B1=GP%u  B2=GP%u  B3=GP%u  B4=GP%u\n",
           GPIO_WORD_PINS[0], GPIO_WORD_PINS[1], GPIO_WORD_PINS[2], GPIO_WORD_PINS[3], GPIO_WORD_PINS[4]);
    printf("  B5=GP%u  B6=GP%u  B7=GP%u  B8=GP%u\n",
           GPIO_WORD_PINS[5], GPIO_WORD_PINS[6], GPIO_WORD_PINS[7], GPIO_WORD_PINS[8]);

    // Configura as 9 GPIOs como SAÍDA
    for (uint8_t i = 0; i < 9; i++) config_output(GPIO_WORD_PINS[i]);

    // Snapshot inicial (9 bits)
    uint16_t last = (uint16_t)(word6_get() & 0x1FFu);
    write_word9_now(last);
    printf("[Word9-OUT] inicial: 0x%03X  ", last);
    print_bits9(last);

    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);
    TickType_t lastBeat = xTaskGetTickCount();
    const TickType_t beat = pdMS_TO_TICKS(1000);

    for (;;) {
        vTaskDelay(dt);

        uint16_t cur = (uint16_t)(word6_get() & 0x1FFu);
        if (cur != last) {
            last = cur;
            write_word9_now(cur);
            printf("[Word9-OUT] mudou -> 0x%03X  ", cur);
            print_bits9(cur);
        }

        TickType_t now = xTaskGetTickCount();
        if ((now - lastBeat) >= beat) {
            lastBeat = now;
            printf("[Word9-OUT] atual: 0x%03X  ", last);
            print_bits9(last);
        }
    }
}

// Mantém a assinatura do header (parâmetro use_pullup continua ignorado em modo saída)
void criar_tarefa_word6(UBaseType_t prio, UBaseType_t core_mask, bool use_pullup) {
    (void)use_pullup;
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_word6_out, "word9_out", 768, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

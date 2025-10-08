// tarefa_word6.c — espelha g_word6_value (0..63) nas GPIOs:
// Mapa LSB→MSB: B0=GP18, B1=GP19, B2=GP20, B3=GP4, B4=GP9, B5=GP8
#include "tarefa_word6.h"

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define PERIOD_MS 50u

volatile uint8_t g_word6_value = 0;  // 0..63

static inline void config_output(uint pin) {
    gpio_init(pin);
    gpio_disable_pulls(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

static inline void put_bit(uint pin, uint8_t bit_on) {
    gpio_put(pin, bit_on ? 1 : 0);
}

static inline void write_word6_now(uint8_t v) {
    put_bit(GPIO_WORD_B0, (v >> 0) & 1u); // GP18
    put_bit(GPIO_WORD_B1, (v >> 1) & 1u); // GP19
    put_bit(GPIO_WORD_B2, (v >> 2) & 1u); // GP20
    put_bit(GPIO_WORD_B3, (v >> 3) & 1u); // GP4
    put_bit(GPIO_WORD_B4, (v >> 4) & 1u); // GP9
    put_bit(GPIO_WORD_B5, (v >> 5) & 1u); // GP8
}

static void print_bits(uint8_t v) {
    printf("BIN: %c%c%c%c%c%c  (MSB..LSB)\n",
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
        printf("[Word6-OUT] aguardando USB...\n");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    printf("[Word6-OUT] iniciando escrita ativo-alto\n");
    printf("[Word6-OUT] Pinos LSB->MSB: B0=GP%d  B1=GP%d  B2=GP%d  B3=GP%d  B4=GP%d  B5=GP%d\n",
           GPIO_WORD_B0, GPIO_WORD_B1, GPIO_WORD_B2, GPIO_WORD_B3, GPIO_WORD_B4, GPIO_WORD_B5);

    // Configura as 6 GPIOs como SAÍDA
    config_output(GPIO_WORD_B0);
    config_output(GPIO_WORD_B1);
    config_output(GPIO_WORD_B2);
    config_output(GPIO_WORD_B3);
    config_output(GPIO_WORD_B4);
    config_output(GPIO_WORD_B5);

    uint8_t last = (uint8_t)(word6_get() & 0x3Fu);
    write_word6_now(last);
    printf("[Word6-OUT] inicial: 0x%02X  ", last);
    print_bits(last);

    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);
    TickType_t lastBeat = xTaskGetTickCount();
    const TickType_t beat = pdMS_TO_TICKS(1000);

    for (;;) {
        vTaskDelay(dt);

        uint8_t cur = (uint8_t)(word6_get() & 0x3Fu);
        if (cur != last) {
            last = cur;
            write_word6_now(cur);
            printf("[Word6-OUT] mudou -> 0x%02X  ", cur);
            print_bits(cur);
        }

        TickType_t now = xTaskGetTickCount();
        if ((now - lastBeat) >= beat) {
            lastBeat = now;
            printf("[Word6-OUT] atual: 0x%02X  ", last);
            print_bits(last);
        }
    }
}

// Mantém a assinatura do header (parâmetro use_pullup é ignorado em modo saída)
void criar_tarefa_word6(UBaseType_t prio, UBaseType_t core_mask, bool use_pullup) {
    (void)use_pullup;
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_word6_out, "word6_out", 768, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

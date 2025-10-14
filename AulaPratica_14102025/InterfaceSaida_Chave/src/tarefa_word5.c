// tarefa_word5.c — LÊ g_word5_value (0..31) das GPIOs de ENTRADA com pull-up:
// Mapa LSB→MSB: B0=GP18, B1=GP19, B2=GP20, B3=GP4, B4=GP9

#include "tarefa_word5.h"

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Taxa de varredura das entradas
#ifndef PERIOD_MS
#define PERIOD_MS 10u
#endif

volatile uint8_t g_word5_value = 0;  // 0..31 (atualizado pela leitura dos pinos)

static inline void config_input(uint pin, bool use_pullup) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    if (use_pullup) {
        gpio_pull_up(pin);
    } else {
        gpio_disable_pulls(pin); // alta-Z
    }
}

static inline uint8_t read_word5_now(void) {
    uint8_t v = 0;
    v |= (uint8_t)(gpio_get(GPIO_WORD_B0) & 1u) << 0; // GP18
    v |= (uint8_t)(gpio_get(GPIO_WORD_B1) & 1u) << 1; // GP19
    v |= (uint8_t)(gpio_get(GPIO_WORD_B2) & 1u) << 2; // GP20
    v |= (uint8_t)(gpio_get(GPIO_WORD_B3) & 1u) << 3; // GP4
    v |= (uint8_t)(gpio_get(GPIO_WORD_B4) & 1u) << 4; // GP9
    return (uint8_t)(v & 0x1Fu);
}

static void print_bits5(uint8_t v) {
    v &= 0x1Fu;
    printf("BIN: %c%c%c%c%c  (MSB..LSB)\n",
           (v&(1u<<4))?'1':'0',  // B4 (GP9)
           (v&(1u<<3))?'1':'0',  // B3 (GP4)
           (v&(1u<<2))?'1':'0',  // B2 (GP20)
           (v&(1u<<1))?'1':'0',  // B1 (GP19)
           (v&(1u<<0))?'1':'0'); // B0 (GP18)
}

static void task_word5_in(void *pv) {
    (void)pv;

    // (opcional) aguardar USB para logs
    for (int i = 0; i < 50 && !stdio_usb_connected(); ++i) {
        printf("[Word5-IN] aguardando USB...\n");
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    printf("[Word5-IN] iniciando LEITURA com pull-up configurável\n");
    printf("[Word5-IN] Entradas LSB->MSB: B0=GP%d  B1=GP%d  B2=GP%d  B3=GP%d  B4=GP%d\n",
           GPIO_WORD_B0, GPIO_WORD_B1, GPIO_WORD_B2, GPIO_WORD_B3, GPIO_WORD_B4);

    // OBS: use_pullup é passado via pvParameter (bool*) para permitir escolha em runtime
    bool use_pullup = true;
    if (pv) { use_pullup = *(bool*)pv; }

    // Configura as 5 GPIOs como ENTRADA (pull-up se desejado)
    config_input(GPIO_WORD_B0, use_pullup);
    config_input(GPIO_WORD_B1, use_pullup);
    config_input(GPIO_WORD_B2, use_pullup);
    config_input(GPIO_WORD_B3, use_pullup);
    config_input(GPIO_WORD_B4, use_pullup);

    // Leitura inicial
    uint8_t cur = read_word5_now();
    g_word5_value = cur;
    printf("[Word5-IN] inicial: 0x%02X  ", cur);
    print_bits5(cur);

    const TickType_t dt   = pdMS_TO_TICKS(PERIOD_MS);
    TickType_t lastBeat   = xTaskGetTickCount();
    const TickType_t beat = pdMS_TO_TICKS(1000);

    for (;;) {
        vTaskDelay(dt);

        uint8_t v = read_word5_now();
        if (v != g_word5_value) {
            g_word5_value = v;  // atualiza snapshot global
            printf("[Word5-IN] mudou -> 0x%02X  ", v);
            print_bits5(v);
        }

        // log 1x/s do valor atual
        TickType_t now = xTaskGetTickCount();
        if ((now - lastBeat) >= beat) {
            lastBeat = now;
            printf("[Word5-IN] atual: 0x%02X  ", g_word5_value);
            print_bits5(g_word5_value);
        }
    }
}

// Cria a tarefa leitora: param use_pullup define se ativa pull-up nas entradas
void criar_tarefa_word5(UBaseType_t prio, UBaseType_t core_mask, bool use_pullup) {
    // Passa use_pullup via pvParameter
    static bool s_use_pullup;        // estático para vida longa
    s_use_pullup = use_pullup;

    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_word5_in, "word5_in", 768, &s_use_pullup, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

#include "tarefa_word_to_gpio.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "task.h"

#include <stdint.h>
#include <stdio.h>

/* ==== De onde vem a PALAVRA? ==========================================
 * Preferimos word5 (0..31). Se seu projeto ainda usa word6, basta ter
 * um stub que mapeia word6->word5, ou ajuste o include abaixo.
 */
#if __has_include("tarefa_word5.h")
  #include "tarefa_word5.h"
  #define WORD_GET()      (word5_get() & 0x1Fu)   // B4..B0
#elif __has_include("tarefa_word6.h")
  #include "tarefa_word6.h"
  #define WORD_GET()      (word6_get() & 0x3Fu)   // B5..B0 (usaremos B3..B0)
#else
  #error "Inclua tarefa_word5.h (ou tarefa_word6.h) no projeto para fornecer WORD_GET()."
#endif

/* ==== Mapeamento FÍSICO das SAÍDAS ==================================== */
#ifndef OUT_B0_PIN
#define OUT_B0_PIN   28
#endif
#ifndef OUT_B1_PIN
#define OUT_B1_PIN   17
#endif
#ifndef OUT_B2_PIN
#define OUT_B2_PIN   16
#endif
#ifndef OUT_B3_PIN
#define OUT_B3_PIN    8
#endif

/* ==== Taxa de espelhamento ============================================ */
#ifndef WORD_OUT_PERIOD_MS
#define WORD_OUT_PERIOD_MS  2u   // baixo para não perder pulsos curtos
#endif

/* ==== Helpers de GPIO ================================================== */
static inline void cfg_out(uint pin) {
    gpio_init(pin);
    gpio_disable_pulls(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

static inline void drive_pin(uint pin, uint8_t logical_level, bool active_high) {
    // logical_level: 0/1 do bit da PALAVRA
    // active_high==true => 1->3V3; false => invertido
    uint8_t phys = active_high ? logical_level : (uint8_t)!logical_level;
    gpio_put(pin, phys);
}

/* ==== Tarefa =========================================================== */
typedef struct {
    bool active_high;
} word_to_gpio_cfg_t;

static void tarefa_word_to_gpio_entry(void *arg) {
    const word_to_gpio_cfg_t *cfg = (const word_to_gpio_cfg_t*)arg;
    const bool active_high = cfg ? cfg->active_high : true;

    // Configura pinos como SAÍDA e inicia em nível lógico 0
    cfg_out(OUT_B0_PIN);
    cfg_out(OUT_B1_PIN);
    cfg_out(OUT_B2_PIN);
    cfg_out(OUT_B3_PIN);

    drive_pin(OUT_B0_PIN, 0, active_high);
    drive_pin(OUT_B1_PIN, 0, active_high);
    drive_pin(OUT_B2_PIN, 0, active_high);
    drive_pin(OUT_B3_PIN, 0, active_high);

    printf("[WORD->GPIO] ON  B3->GP%d  B2->GP%d  B1->GP%d  B0->GP%d  (active_high=%d)\n",
           OUT_B3_PIN, OUT_B2_PIN, OUT_B1_PIN, OUT_B0_PIN, (int)active_high);

    const TickType_t dt = pdMS_TO_TICKS(WORD_OUT_PERIOD_MS);

    // Heartbeat de log (opcional)
    TickType_t t_last = xTaskGetTickCount();

    for (;;) {
        // 1) Lê a palavra global (ex.: g_word5_value)
        uint8_t w = (uint8_t)WORD_GET();

        // 2) Extrai B3..B0
        uint8_t b0 = (w >> 0) & 1u;
        uint8_t b1 = (w >> 1) & 1u;
        uint8_t b2 = (w >> 2) & 1u;
        uint8_t b3 = (w >> 3) & 1u;
        // B4 é ignorado aqui

        // 3) Espelha nos pinos (sempre, para não perder pulsos curtos)
        drive_pin(OUT_B0_PIN, b0, active_high);
        drive_pin(OUT_B1_PIN, b1, active_high);
        drive_pin(OUT_B2_PIN, b2, active_high);
        drive_pin(OUT_B3_PIN, b3, active_high);

        // (Opcional) log a cada 1s
        TickType_t now = xTaskGetTickCount();
        if ((now - t_last) >= pdMS_TO_TICKS(1000)) {
            t_last = now;
            printf("[WORD->GPIO] word=0x%02X  B3=%u B2=%u B1=%u B0=%u\n",
                   w, b3, b2, b1, b0);
        }

        vTaskDelay(dt);
    }
}

/* ==== API ============================================================== */
void criar_tarefa_word_to_gpio(UBaseType_t prio, UBaseType_t core_mask, bool active_high) {
    static word_to_gpio_cfg_t cfg;  // estático: vida inteira
    cfg.active_high = active_high;

    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(
        tarefa_word_to_gpio_entry,
        "word_to_gpio",
        512,               // stack enxuto; aumente se precisar de mais log
        &cfg,
        prio,
        &th
    );
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

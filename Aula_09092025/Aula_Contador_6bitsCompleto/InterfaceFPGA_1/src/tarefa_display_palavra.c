#include "tarefa_display_palavra.h"
#include "tarefa_word6.h"

#include "oled_display.h"
#include "oled_context.h"
#include "ssd1306_text.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>

#define OLED_REFRESH_MS     80u
#define STACK_WORDS_OLED   768u

static void desenhar_palavra(uint8_t v) {
    char linha[32];

    oled_clear(&oled);
    oled_centralizar_texto(&oled, "PALAVRA DIG 6b", 0);
    ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 8, "ENTR: PULL-UP", oled.width, oled.height);

    // BIN (MSB..LSB)
    snprintf(linha, sizeof(linha), "BIN: 0b%c%c%c%c%c%c",
        (v&(1u<<5))?'1':'0',
        (v&(1u<<4))?'1':'0',
        (v&(1u<<3))?'1':'0',
        (v&(1u<<2))?'1':'0',
        (v&(1u<<1))?'1':'0',
        (v&(1u<<0))?'1':'0');
    ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 20, linha, oled.width, oled.height);

    snprintf(linha, sizeof(linha), "DEC: %u", v);
    ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 30, linha, oled.width, oled.height);

    snprintf(linha, sizeof(linha), "HEX: 0x%02X", v);
    ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 40, linha, oled.width, oled.height);

    oled_render(&oled);
}

static void task_display_palavra(void *pv) {
    (void)pv;
    uint8_t ultimo = 0xFF;  // força primeira atualização
    uint8_t rx     = 0;

    const TickType_t refresh = pdMS_TO_TICKS(OLED_REFRESH_MS);

    for (;;) {
        // pega valor mais recente (não bloqueante)
        if (fila_word6 && xQueueReceive(fila_word6, &rx, 0) == pdPASS) {
            if (rx != ultimo) {
                ultimo = rx;
                if (xSemaphoreTake(mutex_oled, pdMS_TO_TICKS(50))) {
                    desenhar_palavra(ultimo);
                    xSemaphoreGive(mutex_oled);
                }
            }
        }
        vTaskDelay(refresh);
    }
}

void criar_tarefa_display_palavra(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_display_palavra, "oled_pal", STACK_WORDS_OLED, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

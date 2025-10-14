// tarefa_display_word6.c — mostra "BIT N" (linha 0) e o dígito grande 0/1 do bit N
#include "tarefa_display_word6.h"

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "oled_display.h"
#include "oled_context.h"
#include "ssd1306_text.h"
#include "numeros_grandes.h"
#include "digitos_grandes_utils.h"

#include "tarefa_joystick_eventos.h"   // joystick_get_sel_bit()
#include "tarefa_word6.h"              // WORD9_MASK / word6_get()

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

extern SemaphoreHandle_t mutex_oled;
extern ssd1306_t oled;
extern volatile uint16_t g_word6_value;   // 9 bits em uso

#define PERIOD_MS  80u

static void desenhar_bitN_grande_com_header(uint8_t sel, uint8_t v01)
{
    // 1) limpa quadro
    oled_clear(&oled);

    // 2) cabeçalho "BIT N"
    char header[16];
    snprintf(header, sizeof(header), "BIT %u", (unsigned)sel);
    ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 0, header, oled.width, oled.height);

    // 3) dígito grande centralizado (0/1)
    const uint8_t W = 25;
    const uint8_t x = (oled.width > W) ? (uint8_t)((oled.width - W)/2) : 0;
    const uint8_t *bmp = numeros_grandes[v01 ? 1 : 0];
    exibir_digito_grande(&oled, x, bmp);

    // 4) envia ao display
    oled_render(&oled);
}

static void task_display_word6(void *arg)
{
    (void)arg;
    printf("[OLED] BIT N (grande) seguindo selecao do joystick\n");

    uint8_t ultimo_sel = 0xFF, ultimo_v01 = 0xFF;
    const TickType_t dt = pdMS_TO_TICKS(PERIOD_MS);

    for (;;) {
        // bit selecionado 0..8
        uint8_t sel = (uint8_t)(joystick_get_sel_bit() % 9u);

        // snapshot do valor daquele bit
        uint16_t w  = (uint16_t)(g_word6_value & WORD9_MASK);
        uint8_t v01 = (uint8_t)((w >> sel) & 1u);

        if (sel != ultimo_sel || v01 != ultimo_v01) {
            ultimo_sel = sel;
            ultimo_v01 = v01;

            if (xSemaphoreTake(mutex_oled, pdMS_TO_TICKS(100))) {
                desenhar_bitN_grande_com_header(sel, v01);
                xSemaphoreGive(mutex_oled);
            }
        }

        vTaskDelay(dt);
    }
}

void criar_tarefa_display_word6(UBaseType_t prio, UBaseType_t core_mask)
{
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(task_display_word6, "disp_bitN", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

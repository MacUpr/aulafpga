#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "display_bin6.h"
#include "oled_display.h"
#include "oled_context.h"
#include "ssd1306_text.h"
#include "numeros_display.h"   // <- declara oled_centralizar_texto()

void exibir_tela_bin6(uint8_t v, bool pullup_on) {
    char linha[32];

    oled_clear(&oled);

    // Título (linha 0)
    oled_centralizar_texto(&oled, "PALAVRA BINARIA", 0);

    // Indicador de modo de entrada (pull-up ON) na linha 1 (y=8 px)
    if (pullup_on) {
        ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 8,  "ENTR: PULL-UP",     oled.width, oled.height);
    } else {
        ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 8,  "ENTR: (sem pull)",  oled.width, oled.height);
    }

    // BIN (MSB..LSB) na linha ~2.5 (y=20 px)
    // Mostra bit5 .. bit0
    snprintf(linha, sizeof(linha), "BIN: 0b%c%c%c%c%c%c",
        (v & (1u << 5)) ? '1' : '0',
        (v & (1u << 4)) ? '1' : '0',
        (v & (1u << 3)) ? '1' : '0',
        (v & (1u << 2)) ? '1' : '0',
        (v & (1u << 1)) ? '1' : '0',
        (v & (1u << 0)) ? '1' : '0');
    ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 20, linha, oled.width, oled.height);

    // DEC na linha 3.75 (y=30 px)
    snprintf(linha, sizeof(linha), "DEC: %u", (unsigned)v);
    ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 30, linha, oled.width, oled.height);

    // HEX na linha 5 (y=40 px)
    snprintf(linha, sizeof(linha), "HEX: 0x%02X", (unsigned)v);
    ssd1306_draw_utf8_multiline(oled.ram_buffer, 0, 40, linha, oled.width, oled.height);

    oled_render(&oled);
}

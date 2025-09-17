#include "FreeRTOS.h"
#include "task.h"
#include "mpu_calibracao.h"
#include "oled_display.h"
#include "app_display.h"
#include "tarefa_calibracao.h"
#include "handles_tarefas.h"
#include "oled_context.h"   

#include <stdio.h>

void tarefa_calibracao(void *param) {
    i2c_inst_t *i2c = (i2c_inst_t *)param;

    if (xSemaphoreTake(mutex_oled, portMAX_DELAY)) {
        oled_clear(&oled);
        oled_render(&oled);
        xSemaphoreGive(mutex_oled);
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    const uint32_t duracao_ms = 120000;
    const uint32_t intervalo_ms = 5;
    const TickType_t total_ticks = pdMS_TO_TICKS(duracao_ms);
    const TickType_t delay_ticks = pdMS_TO_TICKS(intervalo_ms);
    const TickType_t inicio = xTaskGetTickCount();

    float soma_ax = 0, soma_ay = 0, soma_az = 0;
    float soma_gx = 0, soma_gy = 0, soma_gz = 0;
    uint32_t amostras_validas = 0;
    bool visivel = true;

    sensor_dataf_t dado;
    char texto_tempo[20];

    while ((xTaskGetTickCount() - inicio) < total_ticks) {
        if (mpu_ler_dados(i2c, &dado)) {
            soma_ax += dado.ax;
            soma_ay += dado.ay;
            soma_az += (dado.az - 1.0f);
            soma_gx += dado.gx;
            soma_gy += dado.gy;
            soma_gz += dado.gz;
            amostras_validas++;
        }

        TickType_t decorrido = xTaskGetTickCount() - inicio;
        uint32_t restante = (duracao_ms - pdTICKS_TO_MS(decorrido)) / 1000;

        if (xSemaphoreTake(mutex_oled, portMAX_DELAY)) {
            oled_clear(&oled);
            if (visivel) {
                oled_centralizar_texto(&oled, "CALIBRAÇÃO...", 2);
                snprintf(texto_tempo, sizeof(texto_tempo), "Tempo: %lus", restante);
                oled_centralizar_texto(&oled, texto_tempo, 4);
            }
            oled_render(&oled);
            xSemaphoreGive(mutex_oled);
        }

        visivel = !visivel;
        vTaskDelay(delay_ticks);
    }

    if (amostras_validas == 0) {
        printf("Nenhuma leitura válida durante a calibração!\n");
    } else {
        offset_ax = soma_ax / amostras_validas;
        offset_ay = soma_ay / amostras_validas;
        offset_az = soma_az / amostras_validas;
        offset_gx = soma_gx / amostras_validas;
        offset_gy = soma_gy / amostras_validas;
        offset_gz = soma_gz / amostras_validas;

        printf("\nCalibração finalizada (%lu amostras válidas):\n", amostras_validas);
        printf("Ax: %.4f  Ay: %.4f  Az: %.4f\n", offset_ax, offset_ay, offset_az);
        printf("Gx: %.4f  Gy: %.4f  Gz: %.4f\n", offset_gx, offset_gy, offset_gz);
    }

    if (xSemaphoreTake(mutex_oled, portMAX_DELAY)) {
        oled_clear(&oled);
        oled_centralizar_texto(&oled, "Pronto!", 3);
        oled_render(&oled);
        xSemaphoreGive(mutex_oled);
    }

    vTaskDelay(pdMS_TO_TICKS(1500));

    if (xSemaphoreTake(mutex_oled, portMAX_DELAY)) {
        oled_clear(&oled);
        oled_render(&oled);
        xSemaphoreGive(mutex_oled);
    }

    vTaskResume(handle_display);
    vTaskResume(handle_botao_a);
    xTaskNotifyGive(handle_leitura_mpu);

    vTaskDelete(NULL);
}

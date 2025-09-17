#include "FreeRTOS.h"
#include "task.h"
#include "pico/stdlib.h"
#include "mpu.h"
#include "buffer_sensor.h"
#include "oled_display.h"
#include <stdio.h>

#define INTERVALO_MS 1

void tarefa_leitura_mpu(void *params) {
    sensor_dataf_t dados_locais;

    // Aguarda notificação da calibração
    printf("[LeituraMPU] Aguardando término da calibração...\n");
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    printf("[LeituraMPU] Calibração finalizada. Iniciando leituras...\n");

    while (true) {
        printf("[LeituraMPU] Iniciando loop de leitura...\n");

        if (mpu_ler_dados(i2c0, &dados_locais)) {
            buffer_sensor_adicionar(&dados_locais);

            printf("%s: ax=%.2f ay=%.2f az=%.2f gx=%.2f gy=%.2f gz=%.2f T=%.2f C\n", 
                nome_sensor_global,
                dados_locais.ax, dados_locais.ay, dados_locais.az,
                dados_locais.gx, dados_locais.gy, dados_locais.gz,
                dados_locais.temp_c);
        } else {
            printf("Falha na leitura do sensor\n");
        }

        vTaskDelay(pdMS_TO_TICKS(INTERVALO_MS));
    }
}

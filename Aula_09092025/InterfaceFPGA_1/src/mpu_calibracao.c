#include "mpu_calibracao.h"
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MAX_AMOSTRAS 2400

float offset_ax = 0.0f;
float offset_ay = 0.0f;
float offset_az = 0.0f;
float offset_gx = 0.0f;
float offset_gy = 0.0f;
float offset_gz = 0.0f;

static int comparar_float(const void *a, const void *b) {
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
}

void mpu_calibrar_gyro(i2c_inst_t *i2c, mpu_gyro_offset_t *offset, int amostras) {
    float soma_gx = 0, soma_gy = 0, soma_gz = 0;
    sensor_dataf_t dado;
    int amostras_validas = 0;

    printf("\nIniciando calibracao do giroscopio (%d amostras)...\n", amostras);

    for (int i = 0; i < amostras; i++) {
        if (mpu_ler_dados(i2c, &dado)) {
            soma_gx += dado.gx;
            soma_gy += dado.gy;
            soma_gz += dado.gz;
            amostras_validas++;
        }
        sleep_ms(5);
    }

    if (amostras_validas == 0) {
        printf("Nenhuma leitura válida no giroscópio!\n");
        return;
    }

    offset->gx_offset = soma_gx / amostras_validas;
    offset->gy_offset = soma_gy / amostras_validas;
    offset->gz_offset = soma_gz / amostras_validas;

    printf("Offset GX=%.2f, GY=%.2f, GZ=%.2f\n", offset->gx_offset, offset->gy_offset, offset->gz_offset);
}

void mpu_calibrar_completo(i2c_inst_t *i2c) {
    const uint32_t amostras = 200;
    float ax[amostras], ay[amostras], az[amostras];
    float gx[amostras], gy[amostras], gz[amostras];
    uint32_t idx = 0;

    sensor_dataf_t dado;

    printf("\nIniciando calibração completa robusta (%u amostras)...\n", amostras);

    while (idx < amostras) {
        if (mpu_ler_dados(i2c, &dado)) {
            ax[idx] = dado.ax;
            ay[idx] = dado.ay;
            az[idx] = dado.az - 1.0f;
            gx[idx] = dado.gx;
            gy[idx] = dado.gy;
            gz[idx] = dado.gz;
            idx++;
        }
        sleep_ms(5);
    }

    qsort(ax, amostras, sizeof(float), comparar_float);
    qsort(ay, amostras, sizeof(float), comparar_float);
    qsort(az, amostras, sizeof(float), comparar_float);
    qsort(gx, amostras, sizeof(float), comparar_float);
    qsort(gy, amostras, sizeof(float), comparar_float);
    qsort(gz, amostras, sizeof(float), comparar_float);

    uint32_t ini = amostras / 10;
    uint32_t fim = amostras - ini;
    uint32_t usados = fim - ini;

    float soma_ax = 0, soma_ay = 0, soma_az = 0;
    float soma_gx = 0, soma_gy = 0, soma_gz = 0;

    for (uint32_t i = ini; i < fim; i++) {
        soma_ax += ax[i];
        soma_ay += ay[i];
        soma_az += az[i];
        soma_gx += gx[i];
        soma_gy += gy[i];
        soma_gz += gz[i];
    }

    offset_ax = soma_ax / usados;
    offset_ay = soma_ay / usados;
    offset_az = soma_az / usados;
    offset_gx = soma_gx / usados;
    offset_gy = soma_gy / usados;
    offset_gz = soma_gz / usados;

    printf("\n📏 Calibração completa robusta concluída:\n");
    printf("Ax: %.4f  Ay: %.4f  Az: %.4f\n", offset_ax, offset_ay, offset_az);
    printf("Gx: %.4f  Gy: %.4f  Gz: %.4f\n", offset_gx, offset_gy, offset_gz);
}

void mpu_calibrar_movel(i2c_inst_t *i2c, uint32_t duracao_ms) {
    const uint32_t intervalo_ms = 5;
    const uint32_t total_amostras = duracao_ms / intervalo_ms;
    uint32_t amostras = (total_amostras > MAX_AMOSTRAS) ? MAX_AMOSTRAS : total_amostras;

    float *gx_buf = malloc(sizeof(float) * amostras);
    float *gy_buf = malloc(sizeof(float) * amostras);
    float *gz_buf = malloc(sizeof(float) * amostras);
    float *ax_buf = malloc(sizeof(float) * amostras);
    float *ay_buf = malloc(sizeof(float) * amostras);
    float *az_buf = malloc(sizeof(float) * amostras);

    if (!gx_buf || !gy_buf || !gz_buf || !ax_buf || !ay_buf || !az_buf) {
        printf("❌ Falha ao alocar memória para buffers de calibração.\n");
        return;
    }

    uint32_t idx = 0;
    sensor_dataf_t dado;

    printf("\nIniciando calibração móvel robusta (%lu ms)...\n", duracao_ms);

    while (idx < amostras) {
        if (mpu_ler_dados(i2c, &dado)) {
            gx_buf[idx] = dado.gx;
            gy_buf[idx] = dado.gy;
            gz_buf[idx] = dado.gz;
            ax_buf[idx] = dado.ax;
            ay_buf[idx] = dado.ay;
            az_buf[idx] = dado.az - 1.0f;
            idx++;
        }
        vTaskDelay(pdMS_TO_TICKS(intervalo_ms));
    }

    if (idx < 10) {
        printf("❌ Leituras insuficientes para calibração.\n");
        goto liberar;
    }

    qsort(gx_buf, idx, sizeof(float), comparar_float);
    qsort(gy_buf, idx, sizeof(float), comparar_float);
    qsort(gz_buf, idx, sizeof(float), comparar_float);
    qsort(ax_buf, idx, sizeof(float), comparar_float);
    qsort(ay_buf, idx, sizeof(float), comparar_float);
    qsort(az_buf, idx, sizeof(float), comparar_float);

    uint32_t ini = idx / 10;
    uint32_t fim = idx - ini;
    uint32_t usados = fim - ini;

    float soma_gx = 0, soma_gy = 0, soma_gz = 0;
    float soma_ax = 0, soma_ay = 0, soma_az = 0;

    for (uint32_t i = ini; i < fim; i++) {
        soma_gx += gx_buf[i];
        soma_gy += gy_buf[i];
        soma_gz += gz_buf[i];
        soma_ax += ax_buf[i];
        soma_ay += ay_buf[i];
        soma_az += az_buf[i];
    }

    offset_gx = soma_gx / usados;
    offset_gy = soma_gy / usados;
    offset_gz = soma_gz / usados;
    offset_ax = soma_ax / usados;
    offset_ay = soma_ay / usados;
    offset_az = soma_az / usados;

    printf("\n📏 Calibração móvel robusta concluída:\n");
    printf("Ax: %.4f  Ay: %.4f  Az: %.4f\n", offset_ax, offset_ay, offset_az);
    printf("Gx: %.4f  Gy: %.4f  Gz: %.4f\n", offset_gx, offset_gy, offset_gz);

liberar:
    free(gx_buf);
    free(gy_buf);
    free(gz_buf);
    free(ax_buf);
    free(ay_buf);
    free(az_buf);
}

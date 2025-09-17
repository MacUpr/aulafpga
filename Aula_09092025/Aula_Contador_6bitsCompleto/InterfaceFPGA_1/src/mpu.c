#include "mpu.h"
#include <stdio.h>
#include <stdint.h>   // ← Aqui garante o tipo uint8_t

// Variável global visível a todo o programa
// Valor WHO_AM_I lido
volatile uint8_t sensor_detectado = 0;

// Nome descritivo do sensor
volatile const char* nome_sensor_global = "Desconhecido";

// Offsets globais aplicados após calibração
extern float offset_ax, offset_ay, offset_az;
extern float offset_gx, offset_gy, offset_gz;

// Endereço I2C base
#define MPU_ADDR 0x68  // MPU6050 e MPU6500 usam esse endereço

// Valor esperado de WHO_AM_I
#define WHO_AMI_6050 0x68
#define WHO_AMI_6500 0x70


bool mpu_inicializar(i2c_inst_t *i2c) {
    uint8_t id = 0;
    uint8_t reg = WHO_AM_I_REG;

    // Leitura do WHO_AM_I
    if (i2c_write_blocking(i2c, MPU_ADDR, &reg, 1, true) < 0 ||
        i2c_read_blocking(i2c, MPU_ADDR, &id, 1, false) < 0) {
        printf("Erro na comunicação I2C\n");
        return false;
    }

    sensor_detectado = id;

    if (id == WHO_AMI_6500) {
        nome_sensor_global = "MPU6500";
    } else if (id == WHO_AMI_6050) {
        nome_sensor_global = "MPU6050";
    } else {
        nome_sensor_global = "Desconhecido";
        printf("Sensor desconhecido (WHO_AM_I = 0x%02X)\n", id);
        return false;
    }

    printf("%s inicializado com sucesso (WHO_AM_I = 0x%02X)\n", nome_sensor_global, id);

    // Acorda o sensor
    uint8_t wake_cmd[2] = {PWR_MGMT_1_REG, 0x00};
    if (i2c_write_blocking(i2c, MPU_ADDR, wake_cmd, 2, false) < 0) {
        printf("Falha ao acordar sensor\n");
        return false;
    }

    return true;
}


bool mpu_ler_dados(i2c_inst_t *i2c, sensor_dataf_t *dados) {
    uint8_t reg = DATA_START_REG;
    uint8_t buf[14];

    if (i2c_write_blocking(i2c, MPU_ADDR, &reg, 1, true) < 0) return false;
    if (i2c_read_blocking(i2c, MPU_ADDR, buf, 14, false) < 0) return false;

    int16_t ax_raw = (buf[0] << 8) | buf[1];
    int16_t ay_raw = (buf[2] << 8) | buf[3];
    int16_t az_raw = (buf[4] << 8) | buf[5];
    int16_t temp_raw = (buf[6] << 8) | buf[7];
    int16_t gx_raw = (buf[8] << 8) | buf[9];
    int16_t gy_raw = (buf[10] << 8) | buf[11];
    int16_t gz_raw = (buf[12] << 8) | buf[13];

    // Conversão padrão (±2g, ±250°/s)
    dados->ax = ax_raw / 16384.0f;
    dados->ay = ay_raw / 16384.0f;
    dados->az = az_raw / 16384.0f;

    dados->gx = gx_raw / 131.0f;
    dados->gy = gy_raw / 131.0f;
    dados->gz = gz_raw / 131.0f;

    dados->temp_c = temp_raw / 340.0f + 36.53f;

    // Aplica offsets calibrados
    dados->ax -= offset_ax;
    dados->ay -= offset_ay;
    dados->az -= offset_az;

    dados->gx -= offset_gx;
    dados->gy -= offset_gy;
    dados->gz -= offset_gz;

    return true;
}

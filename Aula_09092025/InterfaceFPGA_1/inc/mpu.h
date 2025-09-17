#ifndef MPU_H
#define MPU_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

// Registradores básicos
#define WHO_AM_I_REG     0x75
#define PWR_MGMT_1_REG   0x6B
#define DATA_START_REG   0x3B

extern volatile uint8_t sensor_detectado;
extern volatile const char* nome_sensor_global;


typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float temp_c;
} sensor_dataf_t;

// Inicializa MPU6050 ou MPU6500
bool mpu_inicializar(i2c_inst_t *i2c);

// Lê dados com compensação de offset
bool mpu_ler_dados(i2c_inst_t *i2c, sensor_dataf_t *dados);

#endif

#ifndef MPU_CALIBRACAO_H
#define MPU_CALIBRACAO_H

#include "mpu.h"
#include "hardware/i2c.h"

// Estrutura de offset para correção
typedef struct {
    float gx_offset;
    float gy_offset;
    float gz_offset;
} mpu_gyro_offset_t;

// Calcula offset do giroscópio com base em N amostras
void mpu_calibrar_gyro(i2c_inst_t *i2c, mpu_gyro_offset_t *offset, int amostras);

// Calibra acelerômetro e giroscópio usando média inicial
void mpu_calibrar_completo(i2c_inst_t *i2c);

// Calibra acelerômetro e giroscópio por média móvel durante tempo definido
void mpu_calibrar_movel(i2c_inst_t *i2c, uint32_t duracao_ms);

// Exposição dos offsets globais para o mpu.c
extern float offset_ax, offset_ay, offset_az;
extern float offset_gx, offset_gy, offset_gz;

#endif
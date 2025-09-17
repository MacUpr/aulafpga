#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    int16_t temp_raw;
} sensor_data_raw_t;

typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float temp_c;
} sensor_dataf_t;

#endif

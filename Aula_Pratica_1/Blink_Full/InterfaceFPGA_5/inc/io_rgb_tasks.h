#ifndef IO_RGB_TASKS_H
#define IO_RGB_TASKS_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// --------- Ajustes de hardware (BitDogLab: R=13, G=11, B=12) ---------
#define GPIO_IN_PIN       18
#define LED_R_PIN         13
#define LED_G_PIN         11
#define LED_B_PIN         12

// LED ativo-baixo? (true para comum-ânodo)
#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW    false
#endif

// Entrada ativa-alta? (true = nível alto acende LED)
#ifndef INPUT_ACTIVE_HIGH
#define INPUT_ACTIVE_HIGH true
#endif

// Habilitar pull-up interno na entrada?
#ifndef INPUT_USE_PULLUP
#define INPUT_USE_PULLUP  true
#endif

// Debounce e período de varredura (ms)
#ifndef INPUT_SCAN_MS
#define INPUT_SCAN_MS     5
#endif

#ifndef INPUT_DEBOUNCE_MS
#define INPUT_DEBOUNCE_MS 20
#endif

// API
BaseType_t io_rgb_tasks_start(void);

#endif // IO_RGB_TASKS_H

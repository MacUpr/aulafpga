#ifndef TAREFA_WORD6_H
#define TAREFA_WORD6_H

#include <stdint.h>
#include "FreeRTOS.h"

// Mapa de bits (LSB→MSB): 18,19,20,4,9,8
#define GPIO_WORD_B0   18
#define GPIO_WORD_B1   19
#define GPIO_WORD_B2   20
#define GPIO_WORD_B3    4
#define GPIO_WORD_B4    9
#define GPIO_WORD_B5    8

// Valor global (0..63) atualizado pela tarefa
extern volatile uint8_t g_word6_value;

// Acesso seguro (snapshot) ao valor atual
static inline uint8_t word6_get(void) {
    __asm volatile ("" ::: "memory"); // barreira leve
    return g_word6_value;
}

// Cria a tarefa leitora (núcleo definido e opção pull-up/alta Z)
void criar_tarefa_word6(UBaseType_t prio, UBaseType_t core_mask, bool use_pullup);

#endif // TAREFA_WORD6_H

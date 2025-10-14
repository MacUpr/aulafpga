#ifndef TAREFA_WORD5_H
#define TAREFA_WORD5_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"

// Mapa de bits (LSB→MSB) lidos das ENTRADAS: 18,19,20,4,9
#define GPIO_WORD_B0   18
#define GPIO_WORD_B1   19
#define GPIO_WORD_B2   20
#define GPIO_WORD_B3    4
#define GPIO_WORD_B4    9

// Valor global (0..31) atualizado pela tarefa (leitor de pinos)
extern volatile uint8_t g_word5_value;

// Snapshot seguro do valor atual
static inline uint8_t word5_get(void) {
    __asm volatile ("" ::: "memory"); // barreira leve
    return (uint8_t)(g_word5_value & 0x1Fu);
}

// Cria a tarefa leitora (pinos como ENTRADA; se use_pullup=true, ativa pull-up)
void criar_tarefa_word5(UBaseType_t prio, UBaseType_t core_mask, bool use_pullup);

#endif // TAREFA_WORD5_H

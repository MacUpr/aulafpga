#ifndef TAREFA_WORD_9_H
#define TAREFA_WORD_9_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"

// Máscara para 9 bits (B0..B8)
#define WORD9_MASK  (0x1FFu)

// Agora a palavra é 16-bit (usa 9 bits válidos)
extern volatile uint16_t g_word6_value;

// Getter padronizado (mascara os 9 bits)
static inline uint16_t word6_get(void) {
    return (uint16_t)(g_word6_value & WORD9_MASK);
}

// Assinatura mantida
void criar_tarefa_word6(UBaseType_t prio, UBaseType_t core_mask, bool use_pullup);

#endif /* TAREFA_WORD6_H */

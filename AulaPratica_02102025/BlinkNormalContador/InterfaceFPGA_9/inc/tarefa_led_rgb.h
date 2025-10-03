// tarefa_led_verde.h
#ifndef TAREFA_LED_RGB_H
#define TAREFA_LED_RGB_H

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Cria a tarefa que configura o LED RGB em verde e se suspende.
 *
 * A implementação usa pinos e polaridade definidos no .c
 * (GPIO_LED_R/G/B e LED_ACTIVE_LOW). Ajuste no .c se necessário.
 *
 * @param prio      Prioridade da tarefa (ex.: tskIDLE_PRIORITY+1).
 * @param core_mask Máscara de afinidade de núcleo (1<<0 para core0, 1<<1 para core1).
 */
void criar_tarefa_led_verde(UBaseType_t prio, UBaseType_t core_mask);

#ifdef __cplusplus
}
#endif

#endif // TAREFA_LED_VERDE_H

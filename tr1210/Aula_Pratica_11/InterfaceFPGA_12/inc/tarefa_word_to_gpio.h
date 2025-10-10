#ifndef TAREFA_WORD_TO_GPIO_H
#define TAREFA_WORD_TO_GPIO_H

#include <stdbool.h>
#include "FreeRTOS.h"

/*
 * Esta tarefa lê uma palavra global (ex.: g_word5_value via word5_get())
 * e escreve os bits B3..B0 nos GPIOs abaixo.
 *
 * Mapeamento padrão (altere se quiser):
 *   B2 -> GP16
 *   B1 -> GP17
 *   B0 -> GP28
 *   B3 -> GP8
 */
#ifndef OUT_B0_PIN
#define OUT_B0_PIN   28
#endif
#ifndef OUT_B1_PIN
#define OUT_B1_PIN   17
#endif
#ifndef OUT_B2_PIN
#define OUT_B2_PIN   16
#endif
#ifndef OUT_B3_PIN
#define OUT_B3_PIN    8
#endif

/*
 * Cria a tarefa que espelha B3..B0 nos pinos OUT_B3_PIN..OUT_B0_PIN.
 * - prio / core_mask: mesmas semantics das suas outras tarefas.
 * - active_high: true = nível lógico 1 escreve 3V3; false = lógica invertida.
 */
void criar_tarefa_word_to_gpio(UBaseType_t prio, UBaseType_t core_mask, bool active_high);

#endif // TAREFA_WORD_TO_GPIO_H

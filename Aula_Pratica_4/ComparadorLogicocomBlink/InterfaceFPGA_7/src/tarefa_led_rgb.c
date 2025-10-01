// tarefa_led_verde.c
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>

// ======== SELECIONE SEU LED =========
// Mapeamento compatível com seu LED_RGB_A (BitDogLab)
#define GPIO_LED_R   13   // LED_RGB_A.R
#define GPIO_LED_G   11   // LED_RGB_A.G
#define GPIO_LED_B   12   // LED_RGB_A.B

// Se seu hardware for ativo-baixo (comum na BitDogLab), deixe 1.
// Se for ativo-alto, mude para 0.
#define LED_ACTIVE_LOW   1

// Converte "ligado/desligado" para nível lógico considerando polaridade
static inline int level_on(bool on) {
#if LED_ACTIVE_LOW
    return on ? 1 : 0;
#else
    return on ? 0 : 1;
#endif
}

static inline void rgb_init_pins(void) {
    gpio_init(GPIO_LED_R); gpio_set_dir(GPIO_LED_R, GPIO_OUT);
    gpio_init(GPIO_LED_G); gpio_set_dir(GPIO_LED_G, GPIO_OUT);
    gpio_init(GPIO_LED_B); gpio_set_dir(GPIO_LED_B, GPIO_OUT);
}

static inline void rgb_write(bool r_on, bool g_on, bool b_on) {
    gpio_put(GPIO_LED_R, level_on(r_on));
    gpio_put(GPIO_LED_G, level_on(g_on));
    gpio_put(GPIO_LED_B, level_on(b_on));
}

// ======== TAREFA: manter verde ao ligar ========
static void tarefa_led_verde(void *params) {
    (void)params;

    rgb_init_pins();

    // Verde = G ligado, R/B desligados
    rgb_write(false, true, false);

    // Opção A: configurar e suspender (economia de CPU)
    vTaskSuspend(NULL);

    // Opção B: reforçar periodicamente a cor (descomente se quiser)
    // for (;;) {
    //     rgb_write(false, true, false);
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}

// Wrapper de criação com afinidade de núcleo
void criar_tarefa_led_verde(UBaseType_t prio, UBaseType_t core_mask) {
    TaskHandle_t th = NULL;
    BaseType_t ok = xTaskCreate(tarefa_led_verde, "LED_VERDE", 1024, NULL, prio, &th);
    configASSERT(ok == pdPASS);
    vTaskCoreAffinitySet(th, core_mask);
}

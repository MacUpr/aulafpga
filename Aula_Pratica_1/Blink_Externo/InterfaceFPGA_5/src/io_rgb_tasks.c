#include "io_rgb_tasks.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"   // se usar filas


// ================== Config de build ==================
#ifndef INPUT_SCAN_MS
#define INPUT_SCAN_MS      5u      // período de varredura
#endif
#ifndef INPUT_DEBOUNCE_MS
#define INPUT_DEBOUNCE_MS 20u      // janela estável
#endif

#ifndef GPIO_IN_PIN
#define GPIO_IN_PIN        18
#endif

#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW     1
#endif
#ifndef INPUT_USE_PULLUP
#define INPUT_USE_PULLUP   1
#endif
#ifndef INPUT_ACTIVE_HIGH
#define INPUT_ACTIVE_HIGH  0
#endif

#ifndef LOG_INPUT
#define LOG_INPUT          0       // 1 = imprime mudanças
#endif

// ============== Fila de estado (último valor) ==============
static QueueHandle_t q_state = NULL;

// ---------------- Utilidades LED -----------------
static inline void led_all_off(void) {
#if LED_ACTIVE_LOW
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 1);
#else
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);
#endif
}

static inline void led_green(bool on) {
#if LED_ACTIVE_LOW
    // apaga R e B; controla G (0 acende, 1 apaga)
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_B_PIN, 1);
    gpio_put(LED_G_PIN, on ? 0 : 1);
#else
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_B_PIN, 0);
    gpio_put(LED_G_PIN, on ? 1 : 0);
#endif
}

// --------------- Utilidade de leitura ---------------
static inline bool input_ativo(void) {
    const bool raw = gpio_get(GPIO_IN_PIN);  // 0 ou 1
#if INPUT_ACTIVE_HIGH
    return raw;       // ativo quando 1
#else
    return !raw;      // ativo quando 0 (ativo-baixo)
#endif
}

// --------------- Tarefa de leitura (núcleo 0) ---------------
static void tarefa_leitura_gpio(void *arg) {
    (void)arg;

    // Configura entrada
    gpio_init(GPIO_IN_PIN);
    gpio_set_dir(GPIO_IN_PIN, GPIO_IN);
#if INPUT_USE_PULLUP
    gpio_disable_pulls(GPIO_IN_PIN);
    gpio_pull_up(GPIO_IN_PIN);
#else
    gpio_disable_pulls(GPIO_IN_PIN);
#endif

    // Estado base
    bool confirmado = input_ativo();
    bool amostrado  = confirmado;
    TickType_t janela_ini = 0;

    // Seed na fila (se existir)
    if (q_state) (void)xQueueOverwrite(q_state, &confirmado);

    // Varredura periódica precisa
    const TickType_t periodo = pdMS_TO_TICKS(INPUT_SCAN_MS);
    TickType_t wake = xTaskGetTickCount();

    for (;;) {
        vTaskDelayUntil(&wake, periodo);

        const bool agora = input_ativo();

        // Início (ou reinício) de janela estável?
        if (agora != amostrado) {
            amostrado = agora;
            janela_ini = xTaskGetTickCount();
            continue;
        }

        // Se está estável por tempo suficiente e difere do confirmado, comuta
        if (amostrado != confirmado) {
            const TickType_t dt = xTaskGetTickCount() - janela_ini;
            if (dt >= pdMS_TO_TICKS(INPUT_DEBOUNCE_MS)) {
                confirmado = amostrado;
                if (q_state) (void)xQueueOverwrite(q_state, &confirmado);

#if LOG_INPUT
                printf("[GPIO%u] novo estado confirmado: %d\n",
                       (unsigned)GPIO_IN_PIN, confirmado ? 1 : 0);
                fflush(stdout);
#endif
            }
        }
    }
}

// --------------- Tarefa do LED (núcleo 1) ---------------
static void tarefa_led_rgb(void *arg) {
    (void)arg;

    gpio_init(LED_R_PIN); gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN); gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN); gpio_set_dir(LED_B_PIN, GPIO_OUT);
    led_all_off();

    // Estado inicial (da fila, se já semeada)
    bool state = false;
    if (q_state) (void)xQueuePeek(q_state, &state, 0);
    led_green(state);

    for (;;) {
        if (q_state && xQueueReceive(q_state, &state, portMAX_DELAY) == pdTRUE) {
            led_green(state);
        }
    }
}

// --------------- Criação / afinidade ---------------
BaseType_t io_rgb_tasks_start(void) {
    if (!q_state) {
        q_state = xQueueCreate(1, sizeof(bool));
        if (!q_state) return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;

        // Semeia com o estado atual da entrada
        bool init = false;
        // Garante que o pino foi configurado ao menos uma vez
        gpio_init(GPIO_IN_PIN);
        gpio_set_dir(GPIO_IN_PIN, GPIO_IN);
#if INPUT_USE_PULLUP
        gpio_disable_pulls(GPIO_IN_PIN);
        gpio_pull_up(GPIO_IN_PIN);
#else
        gpio_disable_pulls(GPIO_IN_PIN);
#endif
        init = input_ativo();
        (void)xQueueOverwrite(q_state, &init);
    }

    TaskHandle_t h_leitura = NULL, h_led = NULL;

    BaseType_t ok1 = xTaskCreate(
        tarefa_leitura_gpio, "t_in_gpio",
        640, NULL, (tskIDLE_PRIORITY + 2), &h_leitura);

    BaseType_t ok2 = xTaskCreate(
        tarefa_led_rgb, "t_led_rgb",
        640, NULL, (tskIDLE_PRIORITY + 1), &h_led);

    if (ok1 != pdPASS || ok2 != pdPASS)
        return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;

#if (configNUM_CORES > 1)
    extern void vTaskCoreAffinitySet(TaskHandle_t xTask, UBaseType_t uxCoreAffinityMask);
    vTaskCoreAffinitySet(h_leitura, (1u << 0)); // núcleo 0
    vTaskCoreAffinitySet(h_led,     (1u << 1)); // núcleo 1
#endif

    return pdPASS;
}

#include "tarefa_onda_quadrada.h"
#include "hardware/gpio.h"

// Calcula ticks a partir da frequência escolhida
#define HALF_PERIOD_TICKS  pdMS_TO_TICKS( (1000 / (2 * ONDA_FREQUENCIA_HZ)) )

typedef struct {
    tarefa_onda_quadrada_cfg_t cfg;
} onda_ctx_t;

static inline void led_blue_set(const tarefa_onda_quadrada_cfg_t *c, bool on) {
    if (c->led_active_low) {
        gpio_put(c->gpio_led_r, 1);
        gpio_put(c->gpio_led_g, 1);
        gpio_put(c->gpio_led_b, on ? 0 : 1);
    } else {
        gpio_put(c->gpio_led_r, 0);
        gpio_put(c->gpio_led_g, 0);
        gpio_put(c->gpio_led_b, on ? 1 : 0);
    }
}

static void tarefa_onda(void *arg) {
    onda_ctx_t *ctx = (onda_ctx_t *)arg;
    const tarefa_onda_quadrada_cfg_t *c = &ctx->cfg;

    gpio_init(c->gpio_wave);
    gpio_set_dir(c->gpio_wave, GPIO_OUT);
    gpio_put(c->gpio_wave, 0);

    gpio_init(c->gpio_led_r); gpio_set_dir(c->gpio_led_r, GPIO_OUT);
    gpio_init(c->gpio_led_g); gpio_set_dir(c->gpio_led_g, GPIO_OUT);
    gpio_init(c->gpio_led_b); gpio_set_dir(c->gpio_led_b, GPIO_OUT);

    led_blue_set(c, false);

    bool level = false;
    for (;;) {
        level = !level;
        gpio_put(c->gpio_wave, level);
        led_blue_set(c, level);
        vTaskDelay(HALF_PERIOD_TICKS);
    }
}

BaseType_t tarefa_onda_quadrada_start(const tarefa_onda_quadrada_cfg_t *cfg, TaskHandle_t *handle_out) {
    if (!cfg) return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    onda_ctx_t *ctx = (onda_ctx_t *)pvPortMalloc(sizeof(onda_ctx_t));
    if (!ctx) return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    ctx->cfg = *cfg;

    TaskHandle_t h = NULL;
    BaseType_t ok = xTaskCreate(
        tarefa_onda,
        cfg->task_name ? cfg->task_name : "onda_sq",
        cfg->task_stack ? cfg->task_stack : 1024,
        (void*)ctx,
        cfg->task_priority,
        &h
    );

#if (configNUM_CORES > 1)
    if (ok == pdPASS && cfg->core_affinity_mask != 0) {
        extern void vTaskCoreAffinitySet(TaskHandle_t xTask, UBaseType_t uxCoreAffinityMask);
        vTaskCoreAffinitySet(h, (UBaseType_t)cfg->core_affinity_mask);
    }
#endif

    if (ok == pdPASS && handle_out) *handle_out = h;
    if (ok != pdPASS) vPortFree(ctx);
    return ok;
}

void tarefa_onda_quadrada_stop(TaskHandle_t handle) {
    if (handle) vTaskDelete(handle);
}

void tarefa_onda_quadrada_suspend(TaskHandle_t handle) {
    if (handle) vTaskSuspend(handle);
}

void tarefa_onda_quadrada_resume(TaskHandle_t handle) {
    if (handle) vTaskResume(handle);
}

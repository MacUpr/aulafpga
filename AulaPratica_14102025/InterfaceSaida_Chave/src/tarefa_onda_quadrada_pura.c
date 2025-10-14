#include "tarefa_onda_quadrada_pura.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

// Calcula ticks a partir da frequência escolhida
#define HALF_PERIOD_TICKS  pdMS_TO_TICKS( (1000 / (2 * ONDA_FREQUENCIA_HZ)) )

typedef struct {
    tarefa_onda_quadrada_cfg_p cfg;
} onda_ctx_t;

static void tarefa_onda(void *arg) {
    onda_ctx_t *ctx = (onda_ctx_t *)arg;
    const tarefa_onda_quadrada_cfg_p *c = &ctx->cfg;

    // Saída da onda quadrada
    gpio_init(c->gpio_wave);
    gpio_set_dir(c->gpio_wave, GPIO_OUT);


    for (;;) {
        vTaskDelay(HALF_PERIOD_TICKS);     // segura o nível atual pelo meio-período
        level = !level;                    // alterna
        gpio_put(c->gpio_wave, level);
    }
}

BaseType_t tarefa_onda_quadrada_start(const tarefa_onda_quadrada_cfg_p *cfg, TaskHandle_t *handle_out) {
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

void tarefa_onda_quadrada_stop2(TaskHandle_t handle) {
    if (handle) vTaskDelete(handle);
}

void tarefa_onda_quadrada_suspend2(TaskHandle_t handle) {
    if (handle) vTaskSuspend(handle);
}

void tarefa_onda_quadrada_resume2(TaskHandle_t handle) {
    if (handle) vTaskResume(handle);
}

// tarefa_pulso_gpio.c
#include "tarefa_pulso_gpio.h"
#include "hardware/gpio.h"
#include "FreeRTOS.h"
#include "task.h"

// Calcula os ticks do meio-período a partir da frequência definida no .h
#define HALF_PERIOD_TICKS_PULSO  pdMS_TO_TICKS((uint32_t)(1000.0f / (2.0f * PULSO_FREQUENCIA_HZ) + 0.5f))

typedef struct {
    tarefa_pulso_gpio_cfg_t2 cfg;
} pulso_ctx_t;

static void tarefa_pulso(void *arg) {
    pulso_ctx_t *ctx = (pulso_ctx_t *)arg;
    const tarefa_pulso_gpio_cfg_t2 *c = &ctx->cfg;

    // Configura GPIO como saída
    gpio_init(c->gpio);
    gpio_set_dir(c->gpio, GPIO_OUT);

    // Inicia em nível alto e alterna a cada meio-período
    bool level = true;
    gpio_put(c->gpio, 1);

    for (;;) {
        vTaskDelay(HALF_PERIOD_TICKS_PULSO);
        level = !level;
        gpio_put(c->gpio, level);
    }
}

BaseType_t tarefa_pulso_gpio_start(const tarefa_pulso_gpio_cfg_t2 *cfg, TaskHandle_t *handle_out) {
    if (!cfg) return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;

    pulso_ctx_t *ctx = (pulso_ctx_t *)pvPortMalloc(sizeof(pulso_ctx_t));
    if (!ctx) return errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    ctx->cfg = *cfg;

    TaskHandle_t h = NULL;
    BaseType_t ok = xTaskCreate(
        tarefa_pulso,
        cfg->task_name ? cfg->task_name : "pulso_gpio",
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

void tarefa_pulso_gpio_stop(TaskHandle_t handle) {
    if (handle) vTaskDelete(handle);
}

void tarefa_pulso_gpio_suspend(TaskHandle_t handle) {
    if (handle) vTaskSuspend(handle);
}

void tarefa_pulso_gpio_resume(TaskHandle_t handle) {
    if (handle) vTaskResume(handle);
}

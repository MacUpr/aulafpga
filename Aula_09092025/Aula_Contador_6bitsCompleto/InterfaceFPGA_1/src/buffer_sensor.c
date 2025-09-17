#include "buffer_sensor.h"
#include "FreeRTOS.h"
#include "task.h"

// Tamanho do buffer circular (pode ser ajustado conforme desejado)
#define BUFFER_TAMANHO 64

// Buffer circular
static sensor_dataf_t buffer[BUFFER_TAMANHO];

// Índice de escrita (posição onde o próximo dado será inserido)
static volatile uint32_t indice_head = 0;

// Total de leituras válidas no buffer
static volatile uint32_t total_validas = 0;

// Inicializa o buffer circular (pode ser usada no início do sistema)
void buffer_sensor_inicializar(void) {
    indice_head = 0;
    total_validas = 0;
}

// Insere uma nova leitura no buffer
void buffer_sensor_adicionar(const sensor_dataf_t *novo) {
    // Região crítica para escrita (evita conflito com leitores)
    taskENTER_CRITICAL();

    buffer[indice_head] = *novo;
    indice_head = (indice_head + 1) % BUFFER_TAMANHO;

    if (total_validas < BUFFER_TAMANHO) {
        total_validas++;
    }

    taskEXIT_CRITICAL();
}

// Obtém o dado mais recente
bool buffer_sensor_obter_mais_recente(sensor_dataf_t *saida) {
    if (total_validas == 0) return false;

    taskENTER_CRITICAL();

    // Cálculo da última posição escrita (head - 1)
    uint32_t ultima_pos = (indice_head + BUFFER_TAMANHO - 1) % BUFFER_TAMANHO;
    *saida = buffer[ultima_pos];

    taskEXIT_CRITICAL();

    return true;
}

// Calcula a média móvel de todas as variáveis do buffer
bool buffer_sensor_calcular_media(sensor_dataf_t *saida) {
    if (total_validas == 0) return false;

    taskENTER_CRITICAL();

    float soma_ax = 0, soma_ay = 0, soma_az = 0;
    float soma_gx = 0, soma_gy = 0, soma_gz = 0;
    float soma_temp = 0;

    for (uint32_t i = 0; i < total_validas; i++) {
        soma_ax   += buffer[i].ax;
        soma_ay   += buffer[i].ay;
        soma_az   += buffer[i].az;
        soma_gx   += buffer[i].gx;
        soma_gy   += buffer[i].gy;
        soma_gz   += buffer[i].gz;
        soma_temp += buffer[i].temp_c;
    }

    saida->ax     = soma_ax   / total_validas;
    saida->ay     = soma_ay   / total_validas;
    saida->az     = soma_az   / total_validas;
    saida->gx     = soma_gx   / total_validas;
    saida->gy     = soma_gy   / total_validas;
    saida->gz     = soma_gz   / total_validas;
    saida->temp_c = soma_temp / total_validas;

    taskEXIT_CRITICAL();
    return true;
}

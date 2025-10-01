#ifndef BUFFER_SENSOR_H
#define BUFFER_SENSOR_H

#include <stdbool.h>
#include "mpu.h"  // Inclui sensor_dataf_t

// Inicializa o buffer (opcional)
void buffer_sensor_inicializar(void);

// Insere uma nova leitura do sensor (chamada pela tarefa de leitura)
void buffer_sensor_adicionar(const sensor_dataf_t *novo);

// Obtém a leitura mais recente disponível (chamada pelas tarefas consumidoras)
bool buffer_sensor_obter_mais_recente(sensor_dataf_t *saida);

bool buffer_sensor_calcular_media(sensor_dataf_t *saida);


#endif
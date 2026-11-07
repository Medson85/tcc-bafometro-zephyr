/*
 * Sensor MQ303B: fases do aquecedor (Q5/Q7), leitura do ADC e persistência
 * do R0. Pinos e canal vêm do nó zephyr,user do devicetree.
 */

#ifndef BAFOMETRO_SENSOR_ALCOOL_H
#define BAFOMETRO_SENSOR_ALCOOL_H

#include <stdint.h>

enum FaseAquecedor {
	FASE_AQUECIMENTO, /* Q5 conduz (2,0 V), Q7 desligado */
	FASE_OPERACAO,    /* Q7 conduz (0,9 V), Q5 desligado */
	FASE_REPOUSO,     /* PROVISÓRIO: os dois desligados; confirmar no esquemático */
};

int sensor_alcool_iniciar(void);
void sensor_alcool_definirFase(enum FaseAquecedor fase);

/* Lê a tensão sobre RL em mV. Retorna 0 ou um erro negativo. */
int sensor_alcool_lerTensaoMv(int32_t *tensaoMv);

/* Retorna 0 e o R0 persistido, ou um erro se não houver valor válido. */
int sensor_alcool_carregarR0(uint32_t *r0Ohms);

#endif /* BAFOMETRO_SENSOR_ALCOOL_H */

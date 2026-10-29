/*
 * Sensor MQ303B: fases do aquecedor (Q5/Q7), leitura do ADC e persistência
 * do R0.
 */

#ifndef BAFOMETRO_SENSOR_ALCOOL_H
#define BAFOMETRO_SENSOR_ALCOOL_H

#include <stdint.h>

enum FaseAquecedor {
	FASE_AQUECIMENTO, /* Q7 baixo, Q5 alto */
	FASE_OPERACAO,    /* Q7 alto, Q5 baixo */
	FASE_REPOUSO,     /* combinação pendente no esquemático */
};

int sensor_alcool_iniciar(void);
void sensor_alcool_definirFase(enum FaseAquecedor fase);

/* Retorna 0 e o R0 persistido, ou um erro se não houver valor válido. */
int sensor_alcool_carregarR0(uint32_t *r0Ohms);

#endif /* BAFOMETRO_SENSOR_ALCOOL_H */

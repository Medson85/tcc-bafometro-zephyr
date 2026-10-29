/*
 * Etapa 1: fases e R0 só registrados em LOG. Os GPIOs do aquecedor e o ADC
 * entram na etapa 3; a persistência do R0, na etapa 6.
 */

#include <errno.h>

#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "dispositivos/sensor_alcool.h"

LOG_MODULE_REGISTER(sensor_alcool);

int sensor_alcool_iniciar(void)
{
	return 0;
}

void sensor_alcool_definirFase(enum FaseAquecedor fase)
{
	switch (fase) {
	case FASE_AQUECIMENTO:
		LOG_INF("Aquecedor: aquecimento (Q7=0, Q5=1)");
		break;
	case FASE_OPERACAO:
		LOG_INF("Aquecedor: operação (Q7=1, Q5=0)");
		break;
	case FASE_REPOUSO:
		LOG_INF("Aquecedor: repouso (combinação de Q5/Q7 pendente no esquemático)");
		break;
	default:
		break;
	}
}

int sensor_alcool_carregarR0(uint32_t *r0Ohms)
{
	ARG_UNUSED(r0Ohms);
	return -ENOENT;
}

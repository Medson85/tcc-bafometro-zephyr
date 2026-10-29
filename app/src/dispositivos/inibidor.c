/*
 * Etapa 1: só o estado lógico, registrado em LOG. A saída digital entra na
 * etapa 3, depois de definido o pino (ver docs/hardware.md).
 */

#include <zephyr/logging/log.h>

#include "dispositivos/inibidor.h"

LOG_MODULE_REGISTER(inibidor);

static bool ativo = true;

void inibidor_iniciar(void)
{
	ativo = true;
	LOG_INF("Inibidor ATIVO (partida)");
}

void inibidor_ativar(void)
{
	ativo = true;
	LOG_INF("Inibidor ATIVO");
}

void inibidor_liberar(void)
{
	ativo = false;
	LOG_INF("Inibidor LIBERADO");
}

bool inibidor_estaAtivo(void)
{
	return ativo;
}

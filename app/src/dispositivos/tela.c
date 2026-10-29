/*
 * Etapa 1: a tela só registra em LOG. O display entra na etapa 4, depois de
 * confirmado o controlador (ver docs/hardware.md).
 */

#include <zephyr/logging/log.h>

#include "dispositivos/tela.h"

LOG_MODULE_REGISTER(tela);

void tela_mostrarMensagem(const char *texto)
{
	LOG_INF("\"%s\"", texto);
}

void tela_mostrarContagem(const char *texto, uint32_t segundos)
{
	LOG_INF("\"%s %u s\"", texto, segundos);
}

void tela_mostrarResultado(const struct ResultadoMedicao *resultado)
{
	if (!resultado->valido) {
		LOG_INF("\"Resultado indisponível\" (medição %u)", resultado->sequencia);
		return;
	}

	LOG_INF("\"%u.%03u mg/L\" (medição %u)", resultado->concentracaoMilesimosMgL / 1000U,
		resultado->concentracaoMilesimosMgL % 1000U, resultado->sequencia);
}

void tela_apagar(void)
{
	LOG_INF("Tela apagada");
}

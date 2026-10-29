/*
 * Etapa 1: o BLE só registra em LOG. Pacote, estado e comando entram na
 * etapa 5.
 */

#include <zephyr/logging/log.h>

#include "dispositivos/ble.h"

LOG_MODULE_REGISTER(ble);

void ble_notificarEstado(uint8_t estado)
{
	LOG_DBG("Estado %u notificado", estado);
}

void ble_notificarResultado(const struct ResultadoMedicao *resultado)
{
	LOG_INF("Resultado da medição %u notificado", resultado->sequencia);
}

void ble_definirAnuncioLento(bool lento)
{
	LOG_INF("Anúncio %s", lento ? "lento" : "normal");
}

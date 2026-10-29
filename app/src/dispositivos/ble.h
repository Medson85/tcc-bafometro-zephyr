/*
 * Comunicação BLE: estado do teste, resultado e intervalo de anúncio.
 */

#ifndef BAFOMETRO_BLE_H
#define BAFOMETRO_BLE_H

#include <stdbool.h>
#include <stdint.h>

#include "dominio/resultado.h"

void ble_notificarEstado(uint8_t estado);
void ble_notificarResultado(const struct ResultadoMedicao *resultado);
void ble_definirAnuncioLento(bool lento);

#endif /* BAFOMETRO_BLE_H */

/*
 * Display OLED: mensagens de cada estágio, contagem regressiva e resultado.
 */

#ifndef BAFOMETRO_TELA_H
#define BAFOMETRO_TELA_H

#include <stdint.h>

#include "dominio/resultado.h"

void tela_mostrarMensagem(const char *texto);
void tela_mostrarContagem(const char *texto, uint32_t segundos);
void tela_mostrarResultado(const struct ResultadoMedicao *resultado);
void tela_apagar(void);

#endif /* BAFOMETRO_TELA_H */

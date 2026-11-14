/*
 * Cálculo do R0 em ar limpo (portado de calibrarR0ArLimpo): média de Rs das
 * amostras válidas, aceita só se pelo menos PORCENTAGEM_MINIMA_AMOSTRAS_VALIDAS
 * das QTD_AMOSTRAS_CALIBRACAO previstas forem válidas e se o R0 ficar entre
 * FAIXA_R0_MIN_OHMS e FAIXA_R0_MAX_OHMS (RF13).
 */

#ifndef BAFOMETRO_CALIBRACAO_H
#define BAFOMETRO_CALIBRACAO_H

#include <stdbool.h>
#include <stdint.h>

struct CalibracaoR0 {
	float somaRsOhms;
	uint16_t validas;
	uint16_t recebidas;
};

void calibracao_iniciar(struct CalibracaoR0 *calibracao);
void calibracao_adicionar(struct CalibracaoR0 *calibracao, float tensaoRlMv);

/* Retorna false, sem alterar *r0Ohms, com amostras válidas insuficientes ou R0 fora da faixa. */
bool calibracao_concluir(const struct CalibracaoR0 *calibracao, float *r0Ohms);

#endif /* BAFOMETRO_CALIBRACAO_H */

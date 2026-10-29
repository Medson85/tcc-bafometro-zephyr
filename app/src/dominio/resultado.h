/*
 * Resultado de uma medição. medicao_calcular() preenche os campos de cálculo;
 * a máquina de estados preenche sequencia, calibrado e inibir.
 */

#ifndef BAFOMETRO_RESULTADO_H
#define BAFOMETRO_RESULTADO_H

#include <stdbool.h>
#include <stdint.h>

#include "dominio/conversao.h"
#include "dominio/qualidade.h"

struct ResultadoMedicao {
	bool valido;
	bool calibrado;
	bool inibir;
	enum QualidadeLeitura qualidade;
	enum FaixaCurva faixa;
	uint16_t sequencia;
	uint16_t quantidadeAmostras;       /* amostras do trecho estável */
	uint16_t concentracaoMilesimosMgL; /* mg/L × 1000, como no pacote BLE */
	float tensaoMediaMv;
	float desvioPadraoMv;
	float coefVariacaoPct;
	float resistenciaRsOhms;
	float relacaoRsR0;
	float concentracaoPpm;
};

#endif /* BAFOMETRO_RESULTADO_H */

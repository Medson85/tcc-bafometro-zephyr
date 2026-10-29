/*
 * Conversões do sensor MQ303B (portadas de sensor.cpp e main.cpp do protótipo):
 * tensão sobre RL -> Rs -> Rs/R0 -> ppm -> mg/L.
 */

#ifndef BAFOMETRO_CONVERSAO_H
#define BAFOMETRO_CONVERSAO_H

#include <stdbool.h>
#include <stdint.h>

/* Posição da razão Rs/R0 em relação à curva de sensibilidade (50 a 500 ppm). */
enum FaixaCurva {
	FAIXA_DENTRO,
	FAIXA_ABAIXO, /* concentração abaixo de 50 ppm: o valor fica saturado em 50 */
	FAIXA_ACIMA,  /* concentração acima de 500 ppm: o valor fica saturado em 500 */
};

/*
 * Rs = RL * (VCC - VRL) / VRL, para o divisor VCC - Rs - ADC - RL - GND.
 * Retorna false se a tensão estiver a menos de MARGEM_TENSAO_VALIDA_MV dos trilhos.
 */
bool conversao_calcularRs(float tensaoRlMv, float *resistenciaRsOhms);

/* Interpolação linear entre os pontos da curva; satura nos extremos e informa a faixa. */
float conversao_rsR0ParaPpm(float relacaoRsR0, enum FaixaCurva *faixa);

float conversao_ppmParaMgL(float concentracaoPpm);

/* mg/L -> milésimos de mg/L, arredondado; limitado a 0..UINT16_MAX. */
uint16_t conversao_mgLParaMilesimos(float concentracaoMgL);

#endif /* BAFOMETRO_CONVERSAO_H */

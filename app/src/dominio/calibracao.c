#include <stddef.h>

#include "config_produto.h"
#include "dominio/calibracao.h"
#include "dominio/conversao.h"

_Static_assert((FAIXA_R0_MIN_OHMS <= R0_PADRAO_OHMS) && (R0_PADRAO_OHMS <= FAIXA_R0_MAX_OHMS),
	       "o R0 padrão deve estar dentro da faixa de validade");

void calibracao_iniciar(struct CalibracaoR0 *calibracao)
{
	*calibracao = (struct CalibracaoR0){0};
}

void calibracao_adicionar(struct CalibracaoR0 *calibracao, float tensaoRlMv)
{
	float resistenciaRsOhms;

	calibracao->recebidas++;
	if (conversao_calcularRs(tensaoRlMv, &resistenciaRsOhms) && (resistenciaRsOhms > 0.0f)) {
		calibracao->somaRsOhms += resistenciaRsOhms;
		calibracao->validas++;
	}
}

bool calibracao_concluir(const struct CalibracaoR0 *calibracao, float *r0Ohms)
{
	const uint32_t minimoPonderado =
		(uint32_t)QTD_AMOSTRAS_CALIBRACAO * PORCENTAGEM_MINIMA_AMOSTRAS_VALIDAS;
	float r0CalculadoOhms;

	if ((r0Ohms == NULL) || (calibracao->validas == 0U) ||
	    (((uint32_t)calibracao->validas * 100U) < minimoPonderado)) {
		return false;
	}

	r0CalculadoOhms = calibracao->somaRsOhms / (float)calibracao->validas;
	if ((r0CalculadoOhms < (float)FAIXA_R0_MIN_OHMS) ||
	    (r0CalculadoOhms > (float)FAIXA_R0_MAX_OHMS)) {
		return false;
	}

	*r0Ohms = r0CalculadoOhms;
	return true;
}

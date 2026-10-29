#include "config_produto.h"
#include "dominio/qualidade.h"

enum QualidadeLeitura qualidade_avaliar(float desvioPadrao, float media)
{
	float coefVariacaoPct;

	if (!(media > 0.0f)) {
		return QUALIDADE_DESCARTAR;
	}

	coefVariacaoPct = (desvioPadrao / media) * 100.0f;

	if (coefVariacaoPct < QUALIDADE_CV_EXCELENTE_PCT) {
		return QUALIDADE_EXCELENTE;
	}
	if (coefVariacaoPct < QUALIDADE_CV_BOA_PCT) {
		return QUALIDADE_BOA;
	}
	if (coefVariacaoPct < QUALIDADE_CV_ACEITAVEL_PCT) {
		return QUALIDADE_ACEITAVEL;
	}

	/* Inclui desvio inválido (NaN): nenhuma comparação acima é verdadeira. */
	return QUALIDADE_DESCARTAR;
}

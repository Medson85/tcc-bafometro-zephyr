#include <math.h>
#include <stddef.h>

#include "config_produto.h"
#include "dominio/conversao.h"

struct PontoCurva {
	float relacaoRsR0;
	float concentracaoPpm;
};

/*
 * Curva de sensibilidade do datasheet, como registrada no protótipo.
 * A razão Rs/R0 cai à medida que a concentração sobe.
 */
static const struct PontoCurva curvaSensibilidade[] = {
	{0.180f, 50.0f},  {0.086f, 100.0f}, {0.060f, 150.0f}, {0.047f, 200.0f}, {0.039f, 250.0f},
	{0.034f, 300.0f}, {0.030f, 350.0f}, {0.028f, 400.0f}, {0.025f, 450.0f}, {0.023f, 500.0f},
};

#define QTD_PONTOS_CURVA (sizeof(curvaSensibilidade) / sizeof(curvaSensibilidade[0]))

bool conversao_calcularRs(float tensaoRlMv, float *resistenciaRsOhms)
{
	const bool dentroDosTrilhos =
		(tensaoRlMv > MARGEM_TENSAO_VALIDA_MV) &&
		(tensaoRlMv < (TENSAO_ALIMENTACAO_MV - MARGEM_TENSAO_VALIDA_MV));

	if ((resistenciaRsOhms == NULL) || !dentroDosTrilhos) {
		return false;
	}

	*resistenciaRsOhms =
		RESISTENCIA_CARGA_OHMS * (TENSAO_ALIMENTACAO_MV - tensaoRlMv) / tensaoRlMv;
	return true;
}

float conversao_rsR0ParaPpm(float relacaoRsR0, enum FaixaCurva *faixa)
{
	const struct PontoCurva *inicio = &curvaSensibilidade[0];
	const struct PontoCurva *fim = &curvaSensibilidade[QTD_PONTOS_CURVA - 1U];
	enum FaixaCurva faixaCalculada = FAIXA_DENTRO;
	float concentracaoPpm = fim->concentracaoPpm;

	if (isnan(relacaoRsR0) || (relacaoRsR0 < fim->relacaoRsR0)) {
		/* Razão inválida também satura no máximo: o erro cai do lado de inibir. */
		faixaCalculada = FAIXA_ACIMA;
	} else if (relacaoRsR0 > inicio->relacaoRsR0) {
		faixaCalculada = FAIXA_ABAIXO;
		concentracaoPpm = inicio->concentracaoPpm;
	} else {
		for (size_t i = 0U; i < (QTD_PONTOS_CURVA - 1U); i++) {
			const struct PontoCurva *a = &curvaSensibilidade[i];
			const struct PontoCurva *b = &curvaSensibilidade[i + 1U];

			if (relacaoRsR0 >= b->relacaoRsR0) {
				concentracaoPpm = a->concentracaoPpm +
						  (b->concentracaoPpm - a->concentracaoPpm) *
							  (relacaoRsR0 - a->relacaoRsR0) /
							  (b->relacaoRsR0 - a->relacaoRsR0);
				break;
			}
		}
	}

	if (faixa != NULL) {
		*faixa = faixaCalculada;
	}

	return concentracaoPpm;
}

float conversao_ppmParaMgL(float concentracaoPpm)
{
	return concentracaoPpm / FATOR_PPM_PARA_MGL;
}

uint16_t conversao_mgLParaMilesimos(float concentracaoMgL)
{
	const float milesimos = concentracaoMgL * 1000.0f;

	if (!(milesimos > 0.0f)) {
		return 0U;
	}
	if (milesimos >= (float)UINT16_MAX) {
		return UINT16_MAX;
	}

	return (uint16_t)(milesimos + 0.5f);
}

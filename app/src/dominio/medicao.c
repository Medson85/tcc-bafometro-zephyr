#include <math.h>
#include <stddef.h>

#include "config_produto.h"
#include "dominio/estatistica.h"
#include "dominio/filtro.h"
#include "dominio/medicao.h"

_Static_assert(MEDICAO_INICIO_TRECHO_ESTAVEL < QTD_TIQUES_COLETA,
	       "o transiente e a janela do filtro devem ser menores que a coleta");
_Static_assert(QTD_TIQUES_COLETA <= QTD_MAXIMA_AMOSTRAS, "a coleta não cabe no vetor de amostras");

/* Saídas do filtro no trecho estável; alocação estática, uso só pela thread da aplicação. */
static int32_t amostrasFiltradasMv[QTD_MAXIMA_AMOSTRAS];

bool medicao_calcular(const int32_t *amostrasMv, uint16_t quantidade, float r0Ohms,
		      struct ResultadoMedicao *resultado)
{
	struct ContextoFiltro filtro;
	struct Estatistica estatistica;
	uint16_t quantidadeEstavel = 0U;

	if (resultado == NULL) {
		return false;
	}

	*resultado = (struct ResultadoMedicao){
		.qualidade = QUALIDADE_DESCARTAR,
	};

	if ((amostrasMv == NULL) || (quantidade <= MEDICAO_INICIO_TRECHO_ESTAVEL) ||
	    (quantidade > QTD_MAXIMA_AMOSTRAS)) {
		return false;
	}

	filtro_iniciar(&filtro);
	for (uint16_t i = 0U; i < quantidade; i++) {
		filtro_adicionar(&filtro, amostrasMv[i]);
		if (i >= MEDICAO_INICIO_TRECHO_ESTAVEL) {
			amostrasFiltradasMv[quantidadeEstavel++] = filtro_valor(&filtro);
		}
	}

	if (!estatistica_calcular(amostrasFiltradasMv, quantidadeEstavel, &estatistica)) {
		return false;
	}

	resultado->quantidadeAmostras = estatistica.quantidade;
	resultado->tensaoMediaMv = estatistica.mediaMv;
	resultado->desvioPadraoMv = estatistica.desvioPadraoMv;
	resultado->coefVariacaoPct = estatistica.coefVariacaoPct;

	if (!conversao_calcularRs(estatistica.mediaMv, &resultado->resistenciaRsOhms) ||
	    !isfinite(r0Ohms) || !(r0Ohms > 0.0f)) {
		return false;
	}

	resultado->relacaoRsR0 = resultado->resistenciaRsOhms / r0Ohms;
	resultado->concentracaoPpm =
		conversao_rsR0ParaPpm(resultado->relacaoRsR0, &resultado->faixa);
	resultado->concentracaoMilesimosMgL =
		conversao_mgLParaMilesimos(conversao_ppmParaMgL(resultado->concentracaoPpm));
	resultado->qualidade = qualidade_avaliar(estatistica.desvioPadraoMv, estatistica.mediaMv);
	resultado->valido = true;

	return true;
}

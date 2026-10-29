#include <math.h>
#include <stddef.h>

#include "dominio/estatistica.h"

bool estatistica_calcular(const int32_t *amostrasMv, uint16_t quantidade, struct Estatistica *saida)
{
	int64_t soma = 0;
	float somaQuadrados = 0.0f;

	if ((amostrasMv == NULL) || (saida == NULL) || (quantidade == 0U)) {
		return false;
	}

	saida->minimoMv = amostrasMv[0];
	saida->maximoMv = amostrasMv[0];
	for (uint16_t i = 0U; i < quantidade; i++) {
		soma += amostrasMv[i];
		if (amostrasMv[i] < saida->minimoMv) {
			saida->minimoMv = amostrasMv[i];
		}
		if (amostrasMv[i] > saida->maximoMv) {
			saida->maximoMv = amostrasMv[i];
		}
	}

	saida->quantidade = quantidade;
	saida->mediaMv = (float)soma / (float)quantidade;

	for (uint16_t i = 0U; i < quantidade; i++) {
		const float diferenca = (float)amostrasMv[i] - saida->mediaMv;

		somaQuadrados += diferenca * diferenca;
	}

	saida->desvioPadraoMv = sqrtf(somaQuadrados / (float)quantidade);
	saida->coefVariacaoPct =
		(saida->mediaMv > 0.0f) ? (saida->desvioPadraoMv / saida->mediaMv) * 100.0f : 0.0f;

	return true;
}

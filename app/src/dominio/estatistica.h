/*
 * Estatística de uma sequência de amostras de tensão (portada de baseline.cpp).
 */

#ifndef BAFOMETRO_ESTATISTICA_H
#define BAFOMETRO_ESTATISTICA_H

#include <stdbool.h>
#include <stdint.h>

struct Estatistica {
	float mediaMv;
	float desvioPadraoMv; /* populacional, como no protótipo */
	float coefVariacaoPct;
	int32_t minimoMv;
	int32_t maximoMv;
	uint16_t quantidade;
};

/* Retorna false se não houver amostras. */
bool estatistica_calcular(const int32_t *amostrasMv, uint16_t quantidade,
			  struct Estatistica *saida);

#endif /* BAFOMETRO_ESTATISTICA_H */

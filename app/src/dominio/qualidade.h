/*
 * Qualidade da leitura pelo coeficiente de variação (portada de baseline.cpp).
 * Limiares em config_produto.h; foram definidos com 5 amostras e precisam ser revalidados.
 */

#ifndef BAFOMETRO_QUALIDADE_H
#define BAFOMETRO_QUALIDADE_H

enum QualidadeLeitura {
	QUALIDADE_DESCARTAR = 0,
	QUALIDADE_ACEITAVEL = 1,
	QUALIDADE_BOA = 2,
	QUALIDADE_EXCELENTE = 3,
};

enum QualidadeLeitura qualidade_avaliar(float desvioPadrao, float media);

#endif /* BAFOMETRO_QUALIDADE_H */

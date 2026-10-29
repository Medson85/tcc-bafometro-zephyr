/*
 * Contrato do filtro digital. A implementação atual é a média móvel
 * (filtro_media_movel.c); um filtro futuro implementa as mesmas funções e
 * troca este contexto, sem mudar quem o usa.
 */

#ifndef BAFOMETRO_FILTRO_H
#define BAFOMETRO_FILTRO_H

#include <stdint.h>

#include "config_produto.h"

struct ContextoFiltro {
	int32_t janela[JANELA_MEDIA_MOVEL];
	int64_t soma;
	uint8_t indice;
	uint8_t preenchidas; /* trata a janela parcial do início */
};

void filtro_iniciar(struct ContextoFiltro *ctx);
void filtro_adicionar(struct ContextoFiltro *ctx, int32_t amostraMv);

/* Média arredondada das amostras na janela; 0 se ainda não houver amostras. */
int32_t filtro_valor(const struct ContextoFiltro *ctx);

#endif /* BAFOMETRO_FILTRO_H */

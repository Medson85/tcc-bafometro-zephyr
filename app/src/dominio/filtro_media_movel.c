/*
 * Média móvel incremental: soma corrente sobre uma janela circular.
 */

#include "dominio/filtro.h"

_Static_assert((JANELA_MEDIA_MOVEL >= 1U) && (JANELA_MEDIA_MOVEL <= UINT8_MAX),
	       "a janela da média móvel deve ter de 1 a 255 amostras");

void filtro_iniciar(struct ContextoFiltro *ctx)
{
	*ctx = (struct ContextoFiltro){0};
}

void filtro_adicionar(struct ContextoFiltro *ctx, int32_t amostraMv)
{
	if (ctx->preenchidas < JANELA_MEDIA_MOVEL) {
		ctx->preenchidas++;
	} else {
		ctx->soma -= ctx->janela[ctx->indice];
	}

	ctx->janela[ctx->indice] = amostraMv;
	ctx->soma += amostraMv;
	ctx->indice = (uint8_t)((ctx->indice + 1U) % JANELA_MEDIA_MOVEL);
}

int32_t filtro_valor(const struct ContextoFiltro *ctx)
{
	const int64_t quantidade = ctx->preenchidas;

	if (quantidade == 0) {
		return 0;
	}

	/* Arredonda para o inteiro mais próximo, também para valores negativos. */
	if (ctx->soma >= 0) {
		return (int32_t)((ctx->soma + (quantidade / 2)) / quantidade);
	}

	return (int32_t)((ctx->soma - (quantidade / 2)) / quantidade);
}

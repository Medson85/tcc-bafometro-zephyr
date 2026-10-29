#include <zephyr/ztest.h>

#include "config_produto.h"
#include "dominio/filtro.h"

static struct ContextoFiltro filtro;

static void antesDeCadaTeste(void *fixture)
{
	ARG_UNUSED(fixture);
	filtro_iniciar(&filtro);
}

ZTEST_SUITE(filtro, NULL, NULL, antesDeCadaTeste, NULL, NULL);

ZTEST(filtro, test_sem_amostras_retorna_zero)
{
	zassert_equal(filtro_valor(&filtro), 0);
}

ZTEST(filtro, test_janela_parcial_no_inicio)
{
	filtro_adicionar(&filtro, 10);
	zassert_equal(filtro_valor(&filtro), 10);
	filtro_adicionar(&filtro, 20);
	zassert_equal(filtro_valor(&filtro), 15);
	filtro_adicionar(&filtro, 30);
	zassert_equal(filtro_valor(&filtro), 20);
}

ZTEST(filtro, test_janela_cheia_descarta_a_amostra_mais_antiga)
{
	for (uint32_t i = 0U; i < JANELA_MEDIA_MOVEL; i++) {
		filtro_adicionar(&filtro, 100);
	}
	zassert_equal(filtro_valor(&filtro), 100);

	filtro_adicionar(&filtro, 100 + 8 * (int32_t)JANELA_MEDIA_MOVEL);
	zassert_equal(filtro_valor(&filtro), 108);
}

ZTEST(filtro, test_degrau_e_alcancado_apos_uma_janela)
{
	for (uint32_t i = 0U; i < JANELA_MEDIA_MOVEL; i++) {
		filtro_adicionar(&filtro, 1000);
	}

	for (uint32_t i = 0U; i < JANELA_MEDIA_MOVEL; i++) {
		filtro_adicionar(&filtro, 2000);
		if (i < (JANELA_MEDIA_MOVEL - 1U)) {
			zassert_true(filtro_valor(&filtro) < 2000);
		}
	}

	zassert_equal(filtro_valor(&filtro), 2000);
}

ZTEST(filtro, test_arredonda_para_o_inteiro_mais_proximo)
{
	filtro_adicionar(&filtro, 1);
	filtro_adicionar(&filtro, 2);
	zassert_equal(filtro_valor(&filtro), 2);

	filtro_iniciar(&filtro);
	filtro_adicionar(&filtro, -1);
	filtro_adicionar(&filtro, -2);
	zassert_equal(filtro_valor(&filtro), -2);
}

ZTEST(filtro, test_reinicio_descarta_o_historico)
{
	filtro_adicionar(&filtro, 500);
	filtro_iniciar(&filtro);
	filtro_adicionar(&filtro, 10);
	zassert_equal(filtro_valor(&filtro), 10);
}

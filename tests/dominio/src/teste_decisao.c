#include <zephyr/ztest.h>

#include "config_produto.h"
#include "dominio/decisao.h"

#define LIMITE    LIMITE_INIBICAO_MILESIMOS_MGL
#define LIBERACAO (LIMITE_INIBICAO_MILESIMOS_MGL - HISTERESE_LIBERACAO_MILESIMOS_MGL)

ZTEST_SUITE(decisao, NULL, NULL, NULL, NULL, NULL);

ZTEST(decisao, test_medicao_nao_aceita_mantem_o_estado)
{
	zassert_true(decisao_deveInibir(true, false, 0U));
	zassert_false(decisao_deveInibir(false, false, 1000U));
}

ZTEST(decisao, test_ativa_a_partir_do_limite)
{
	zassert_true(decisao_deveInibir(false, true, LIMITE));
	zassert_true(decisao_deveInibir(false, true, 1000U));
	zassert_true(decisao_deveInibir(true, true, LIMITE));
}

ZTEST(decisao, test_libera_somente_abaixo_do_limite_menos_a_histerese)
{
	zassert_false(decisao_deveInibir(true, true, LIBERACAO - 1U));
	zassert_false(decisao_deveInibir(true, true, 0U));
	zassert_true(decisao_deveInibir(true, true, LIBERACAO));
}

ZTEST(decisao, test_faixa_de_histerese_mantem_o_estado)
{
	for (uint32_t concentracao = LIBERACAO; concentracao < LIMITE; concentracao++) {
		zassert_true(decisao_deveInibir(true, true, (uint16_t)concentracao));
		zassert_false(decisao_deveInibir(false, true, (uint16_t)concentracao));
	}
}

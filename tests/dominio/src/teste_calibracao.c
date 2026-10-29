#include <zephyr/ztest.h>

#include "auxiliar_teste.h"
#include "config_produto.h"
#include "dominio/calibracao.h"

#define R0_OHMS 24000.0f

/* Mínimo de amostras válidas, como no protótipo: 50 % das previstas. */
#define MINIMO_VALIDAS                                                                             \
	((QTD_AMOSTRAS_CALIBRACAO * PORCENTAGEM_MINIMA_AMOSTRAS_VALIDAS + 99U) / 100U)

static struct CalibracaoR0 calibracao;

static void adicionar(uint32_t validas, uint32_t invalidas)
{
	for (uint32_t i = 0U; i < validas; i++) {
		calibracao_adicionar(&calibracao, tensaoSobreRlMv(R0_OHMS));
	}
	for (uint32_t i = 0U; i < invalidas; i++) {
		calibracao_adicionar(&calibracao, 0.0f);
	}
}

static void antesDeCadaTeste(void *fixture)
{
	ARG_UNUSED(fixture);
	calibracao_iniciar(&calibracao);
}

ZTEST_SUITE(calibracao, NULL, NULL, antesDeCadaTeste, NULL, NULL);

ZTEST(calibracao, test_r0_e_a_media_de_rs_em_ar_limpo)
{
	float r0Ohms;

	adicionar(QTD_AMOSTRAS_CALIBRACAO, 0U);
	zassert_true(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_within(r0Ohms, R0_OHMS, R0_OHMS * 1e-4f);
	zassert_equal(calibracao.recebidas, QTD_AMOSTRAS_CALIBRACAO);
}

ZTEST(calibracao, test_amostras_invalidas_sao_ignoradas)
{
	float r0Ohms;

	adicionar(MINIMO_VALIDAS, QTD_AMOSTRAS_CALIBRACAO - MINIMO_VALIDAS);
	zassert_true(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_within(r0Ohms, R0_OHMS, R0_OHMS * 1e-4f);
	zassert_equal(calibracao.validas, MINIMO_VALIDAS);
}

ZTEST(calibracao, test_amostras_validas_insuficientes_rejeitam)
{
	float r0Ohms = -1.0f;

	adicionar(MINIMO_VALIDAS - 1U, QTD_AMOSTRAS_CALIBRACAO - MINIMO_VALIDAS + 1U);
	zassert_false(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_within(r0Ohms, -1.0f, 1e-6f, "R0 não pode ser alterado na falha");
}

ZTEST(calibracao, test_sem_amostras_rejeita)
{
	float r0Ohms;

	zassert_false(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_false(calibracao_concluir(&calibracao, NULL));
}

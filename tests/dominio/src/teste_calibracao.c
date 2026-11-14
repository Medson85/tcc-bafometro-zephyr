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

static void adicionarComRs(float resistenciaRsOhms, uint32_t quantidade)
{
	for (uint32_t i = 0U; i < quantidade; i++) {
		calibracao_adicionar(&calibracao, tensaoSobreRlMv(resistenciaRsOhms));
	}
}

/* RF13: o R0 só vale dentro da faixa; junto às bordas (0,1 %) ele é aceito. */
ZTEST(calibracao, test_r0_junto_as_bordas_da_faixa_e_aceito)
{
	const float r0MinimoOhms = (float)FAIXA_R0_MIN_OHMS * 1.001f;
	const float r0MaximoOhms = (float)FAIXA_R0_MAX_OHMS * 0.999f;
	float r0Ohms;

	adicionarComRs(r0MinimoOhms, QTD_AMOSTRAS_CALIBRACAO);
	zassert_true(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_within(r0Ohms, r0MinimoOhms, r0MinimoOhms * 1e-4f);

	calibracao_iniciar(&calibracao);
	adicionarComRs(r0MaximoOhms, QTD_AMOSTRAS_CALIBRACAO);
	zassert_true(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_within(r0Ohms, r0MaximoOhms, r0MaximoOhms * 1e-4f);
}

/* Calibração com 50 ppm no ar (Rs/R0 = 0,18): o R0 sairia 4,3 kΩ e é rejeitado. */
ZTEST(calibracao, test_r0_abaixo_da_faixa_rejeita)
{
	float r0Ohms = -1.0f;

	adicionarComRs(R0_OHMS * 0.18f, QTD_AMOSTRAS_CALIBRACAO);
	zassert_equal(calibracao.validas, QTD_AMOSTRAS_CALIBRACAO);
	zassert_false(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_within(r0Ohms, -1.0f, 1e-6f, "R0 não pode ser alterado na falha");
}

ZTEST(calibracao, test_r0_acima_da_faixa_rejeita)
{
	float r0Ohms = -1.0f;

	/* 103 kΩ ainda passa pela guarda de tensão, mas fica acima da faixa. */
	adicionarComRs(103000.0f, QTD_AMOSTRAS_CALIBRACAO);
	zassert_equal(calibracao.validas, QTD_AMOSTRAS_CALIBRACAO);
	zassert_false(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_within(r0Ohms, -1.0f, 1e-6f);
}

/* Sensor desconectado: as amostras ficam abaixo da guarda e nenhuma é válida. */
ZTEST(calibracao, test_sensor_desconectado_rejeita)
{
	float r0Ohms = -1.0f;

	for (uint32_t i = 0U; i < QTD_AMOSTRAS_CALIBRACAO; i++) {
		calibracao_adicionar(&calibracao, 5.0f);
	}
	zassert_equal(calibracao.validas, 0U);
	zassert_false(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_within(r0Ohms, -1.0f, 1e-6f);
}

ZTEST(calibracao, test_sem_amostras_rejeita)
{
	float r0Ohms;

	zassert_false(calibracao_concluir(&calibracao, &r0Ohms));
	zassert_false(calibracao_concluir(&calibracao, NULL));
}

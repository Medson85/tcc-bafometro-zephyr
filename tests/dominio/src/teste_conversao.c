#include <math.h>

#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "auxiliar_teste.h"
#include "config_produto.h"
#include "dominio/conversao.h"

#define R0_OHMS 24000.0f

struct PontoEsperado {
	float relacaoRsR0;
	float concentracaoPpm;
};

/* Curva do protótipo: cada ponto deve ser reproduzido exatamente. */
static const struct PontoEsperado pontosCurva[] = {
	{0.180f, 50.0f},  {0.086f, 100.0f}, {0.060f, 150.0f}, {0.047f, 200.0f}, {0.039f, 250.0f},
	{0.034f, 300.0f}, {0.030f, 350.0f}, {0.028f, 400.0f}, {0.025f, 450.0f}, {0.023f, 500.0f},
};

ZTEST_SUITE(conversao, NULL, NULL, NULL, NULL, NULL);

ZTEST(conversao, test_rs_reconstroi_a_resistencia_do_divisor)
{
	const float resistencias[] = {500.0f, 1000.0f, 3300.0f, 24000.0f, 100000.0f, 400000.0f};

	for (size_t i = 0U; i < ARRAY_SIZE(resistencias); i++) {
		float resistenciaRsOhms;

		zassert_true(
			conversao_calcularRs(tensaoSobreRlMv(resistencias[i]), &resistenciaRsOhms));
		zassert_within(resistenciaRsOhms, resistencias[i], resistencias[i] * 1e-4f,
			       "Rs de %d ohms", (int)resistencias[i]);
	}
}

ZTEST(conversao, test_mais_alcool_eleva_a_tensao_e_a_concentracao)
{
	/* Com álcool, Rs cai, VRL sobe e a concentração calculada deve subir. */
	const float tensaoArLimpoMv = tensaoSobreRlMv(R0_OHMS);
	const float tensaoAlcoolMv = tensaoSobreRlMv(R0_OHMS * 0.047f);
	float rsArLimpo;
	float rsAlcool;
	enum FaixaCurva faixa;

	zassert_true(tensaoAlcoolMv > tensaoArLimpoMv);
	zassert_true(conversao_calcularRs(tensaoArLimpoMv, &rsArLimpo));
	zassert_true(conversao_calcularRs(tensaoAlcoolMv, &rsAlcool));
	zassert_true(rsAlcool < rsArLimpo);

	zassert_within(conversao_rsR0ParaPpm(rsArLimpo / R0_OHMS, &faixa), 50.0f, 1e-3f);
	zassert_equal(faixa, FAIXA_ABAIXO);
	zassert_within(conversao_rsR0ParaPpm(rsAlcool / R0_OHMS, &faixa), 200.0f, 0.5f);
	zassert_equal(faixa, FAIXA_DENTRO);
}

ZTEST(conversao, test_rs_rejeita_tensoes_nos_trilhos)
{
	float resistenciaRsOhms;

	zassert_false(conversao_calcularRs(0.0f, &resistenciaRsOhms));
	zassert_false(conversao_calcularRs(MARGEM_TENSAO_VALIDA_MV, &resistenciaRsOhms));
	zassert_true(conversao_calcularRs(MARGEM_TENSAO_VALIDA_MV + 0.5f, &resistenciaRsOhms));
	zassert_true(conversao_calcularRs(TENSAO_ALIMENTACAO_MV - MARGEM_TENSAO_VALIDA_MV - 0.5f,
					  &resistenciaRsOhms));
	zassert_false(conversao_calcularRs(TENSAO_ALIMENTACAO_MV - MARGEM_TENSAO_VALIDA_MV,
					   &resistenciaRsOhms));
	zassert_false(conversao_calcularRs(TENSAO_ALIMENTACAO_MV, &resistenciaRsOhms));
	zassert_false(conversao_calcularRs(NAN, &resistenciaRsOhms));
	zassert_false(conversao_calcularRs(1000.0f, NULL));
}

ZTEST(conversao, test_pontos_da_curva_sao_reproduzidos)
{
	for (size_t i = 0U; i < ARRAY_SIZE(pontosCurva); i++) {
		enum FaixaCurva faixa;

		zassert_within(conversao_rsR0ParaPpm(pontosCurva[i].relacaoRsR0, &faixa),
			       pontosCurva[i].concentracaoPpm, 1e-3f, "ponto %d", (int)i);
		zassert_equal(faixa, FAIXA_DENTRO, "ponto %d", (int)i);
	}
}

ZTEST(conversao, test_interpolacao_linear_entre_pontos)
{
	for (size_t i = 0U; i < (ARRAY_SIZE(pontosCurva) - 1U); i++) {
		const float relacaoMeio =
			(pontosCurva[i].relacaoRsR0 + pontosCurva[i + 1U].relacaoRsR0) / 2.0f;
		const float ppmMeio =
			(pontosCurva[i].concentracaoPpm + pontosCurva[i + 1U].concentracaoPpm) /
			2.0f;

		zassert_within(conversao_rsR0ParaPpm(relacaoMeio, NULL), ppmMeio, 1e-2f,
			       "segmento %d", (int)i);
	}
}

ZTEST(conversao, test_curva_continua_e_decrescente)
{
	/* A concentração só pode cair quando a razão sobe, sem saltos entre segmentos. */
	float anterior = conversao_rsR0ParaPpm(0.0225f, NULL);

	for (int passo = 1; passo <= 1600; passo++) {
		const float relacao = 0.0225f + (float)passo * 0.0001f;
		const float atual = conversao_rsR0ParaPpm(relacao, NULL);

		zassert_true(atual <= anterior + 1e-3f, "subiu em Rs/R0 = %d/10000",
			     (int)(relacao * 10000.0f));
		zassert_true((anterior - atual) <= 3.0f, "salto em Rs/R0 = %d/10000",
			     (int)(relacao * 10000.0f));
		anterior = atual;
	}
}

ZTEST(conversao, test_saturacao_e_faixa_fora_da_curva)
{
	enum FaixaCurva faixa;

	zassert_within(conversao_rsR0ParaPpm(1.0f, &faixa), 50.0f, 1e-3f);
	zassert_equal(faixa, FAIXA_ABAIXO);
	zassert_within(conversao_rsR0ParaPpm(0.181f, &faixa), 50.0f, 1e-3f);
	zassert_equal(faixa, FAIXA_ABAIXO);
	zassert_within(conversao_rsR0ParaPpm(0.0229f, &faixa), 500.0f, 1e-3f);
	zassert_equal(faixa, FAIXA_ACIMA);
	zassert_within(conversao_rsR0ParaPpm(0.0f, &faixa), 500.0f, 1e-3f);
	zassert_equal(faixa, FAIXA_ACIMA);

	/* Razão inválida satura no máximo: o erro cai do lado de inibir. */
	zassert_within(conversao_rsR0ParaPpm(NAN, &faixa), 500.0f, 1e-3f);
	zassert_equal(faixa, FAIXA_ACIMA);
	zassert_within(conversao_rsR0ParaPpm(-0.5f, &faixa), 500.0f, 1e-3f);
	zassert_equal(faixa, FAIXA_ACIMA);
}

ZTEST(conversao, test_ppm_para_mg_por_litro)
{
	zassert_within(conversao_ppmParaMgL(50.0f), 0.100f, 1e-6f);
	zassert_within(conversao_ppmParaMgL(250.0f), 0.500f, 1e-6f);
	zassert_within(conversao_ppmParaMgL(500.0f), 1.000f, 1e-6f);
}

ZTEST(conversao, test_milesimos_de_mg_por_litro_arredondados)
{
	zassert_equal(conversao_mgLParaMilesimos(0.100f), 100U);
	zassert_equal(conversao_mgLParaMilesimos(0.1004f), 100U);
	zassert_equal(conversao_mgLParaMilesimos(0.1006f), 101U);
	zassert_equal(conversao_mgLParaMilesimos(1.000f), 1000U);
	zassert_equal(conversao_mgLParaMilesimos(-0.2f), 0U);
	zassert_equal(conversao_mgLParaMilesimos(NAN), 0U);
	zassert_equal(conversao_mgLParaMilesimos(1000.0f), UINT16_MAX);
}

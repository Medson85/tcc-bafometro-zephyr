#include <math.h>

#include <zephyr/ztest.h>

#include "auxiliar_teste.h"
#include "config_produto.h"
#include "dominio/medicao.h"

#define R0_OHMS            24000.0f
#define QTD_AMOSTRAS       QTD_TIQUES_COLETA
#define QTD_TRECHO_ESTAVEL (QTD_AMOSTRAS - MEDICAO_INICIO_TRECHO_ESTAVEL)

static int32_t amostras[QTD_MAXIMA_AMOSTRAS];
static struct ResultadoMedicao resultado;

/* Rampa no transiente, da tensão inicial até o patamar, e patamar no restante. */
static void montarSopro(float tensaoInicialMv, float tensaoPatamarMv)
{
	for (uint32_t i = 0U; i < QTD_AMOSTRAS; i++) {
		float tensaoMv = tensaoPatamarMv;

		if (i < QTD_AMOSTRAS_TRANSIENTE) {
			tensaoMv = tensaoInicialMv + (tensaoPatamarMv - tensaoInicialMv) *
							     (float)i /
							     (float)QTD_AMOSTRAS_TRANSIENTE;
		}
		amostras[i] = (int32_t)lroundf(tensaoMv);
	}
}

ZTEST_SUITE(medicao, NULL, NULL, NULL, NULL, NULL);

ZTEST(medicao, test_ar_limpo_fica_abaixo_da_curva)
{
	const float tensaoArLimpoMv = tensaoSobreRlMv(R0_OHMS);

	montarSopro(tensaoArLimpoMv, tensaoArLimpoMv);
	zassert_true(medicao_calcular(amostras, QTD_AMOSTRAS, R0_OHMS, &resultado));

	zassert_true(resultado.valido);
	zassert_equal(resultado.quantidadeAmostras, QTD_TRECHO_ESTAVEL);
	zassert_within(resultado.relacaoRsR0, 1.0f, 0.01f);
	zassert_equal(resultado.faixa, FAIXA_ABAIXO);
	zassert_within(resultado.concentracaoPpm, 50.0f, 1e-3f);
	zassert_equal(resultado.concentracaoMilesimosMgL, 100U);
	zassert_equal(resultado.qualidade, QUALIDADE_EXCELENTE);
}

ZTEST(medicao, test_sopro_com_alcool_e_convertido_pela_curva)
{
	const float tensaoArLimpoMv = tensaoSobreRlMv(R0_OHMS);
	const float tensaoAlcoolMv = tensaoSobreRlMv(R0_OHMS * 0.047f); /* 200 ppm */

	montarSopro(tensaoArLimpoMv, tensaoAlcoolMv);
	zassert_true(medicao_calcular(amostras, QTD_AMOSTRAS, R0_OHMS, &resultado));

	/* A rampa do transiente fica fora do cálculo, inclusive dentro da janela do filtro. */
	zassert_within(resultado.tensaoMediaMv, tensaoAlcoolMv, 1.0f);
	zassert_equal(resultado.faixa, FAIXA_DENTRO);
	zassert_within(resultado.concentracaoPpm, 200.0f, 2.0f);
	zassert_within(resultado.concentracaoMilesimosMgL, 400U, 4U);
	zassert_equal(resultado.qualidade, QUALIDADE_EXCELENTE);
}

ZTEST(medicao, test_sinal_instavel_reduz_a_qualidade)
{
	/* Patamar que muda no meio do trecho estável: CV alto mesmo após o filtro. */
	montarSopro(2000.0f, 2000.0f);
	for (uint32_t i = MEDICAO_INICIO_TRECHO_ESTAVEL + (QTD_TRECHO_ESTAVEL / 2U);
	     i < QTD_AMOSTRAS; i++) {
		amostras[i] = 2600;
	}

	zassert_true(medicao_calcular(amostras, QTD_AMOSTRAS, R0_OHMS, &resultado));
	zassert_true(resultado.coefVariacaoPct > QUALIDADE_CV_ACEITAVEL_PCT);
	zassert_equal(resultado.qualidade, QUALIDADE_DESCARTAR);
}

ZTEST(medicao, test_tensao_fora_dos_trilhos_invalida)
{
	montarSopro(0.0f, 0.0f);
	zassert_false(medicao_calcular(amostras, QTD_AMOSTRAS, R0_OHMS, &resultado));
	zassert_false(resultado.valido);
	zassert_equal(resultado.qualidade, QUALIDADE_DESCARTAR);

	montarSopro(TENSAO_ALIMENTACAO_MV, TENSAO_ALIMENTACAO_MV);
	zassert_false(medicao_calcular(amostras, QTD_AMOSTRAS, R0_OHMS, &resultado));
}

ZTEST(medicao, test_r0_invalido_invalida)
{
	montarSopro(1000.0f, 1000.0f);
	zassert_false(medicao_calcular(amostras, QTD_AMOSTRAS, 0.0f, &resultado));
	zassert_false(medicao_calcular(amostras, QTD_AMOSTRAS, -1.0f, &resultado));
	zassert_false(medicao_calcular(amostras, QTD_AMOSTRAS, NAN, &resultado));
	zassert_false(medicao_calcular(amostras, QTD_AMOSTRAS, INFINITY, &resultado));
}

ZTEST(medicao, test_quantidade_de_amostras_fora_dos_limites_invalida)
{
	montarSopro(1000.0f, 1000.0f);
	zassert_false(
		medicao_calcular(amostras, MEDICAO_INICIO_TRECHO_ESTAVEL, R0_OHMS, &resultado));
	zassert_true(medicao_calcular(amostras, MEDICAO_INICIO_TRECHO_ESTAVEL + 1U, R0_OHMS,
				      &resultado));
	zassert_equal(resultado.quantidadeAmostras, 1U);
	zassert_false(medicao_calcular(amostras, QTD_MAXIMA_AMOSTRAS + 1U, R0_OHMS, &resultado));
	zassert_false(medicao_calcular(NULL, QTD_AMOSTRAS, R0_OHMS, &resultado));
	zassert_false(medicao_calcular(amostras, QTD_AMOSTRAS, R0_OHMS, NULL));
}

ZTEST(medicao, test_resultado_e_zerado_antes_do_calculo)
{
	resultado = (struct ResultadoMedicao){
		.valido = true,
		.inibir = true,
		.sequencia = 7U,
		.concentracaoMilesimosMgL = 999U,
	};

	montarSopro(0.0f, 0.0f);
	zassert_false(medicao_calcular(amostras, QTD_AMOSTRAS, R0_OHMS, &resultado));
	zassert_false(resultado.valido);
	zassert_false(resultado.inibir);
	zassert_equal(resultado.sequencia, 0U);
	zassert_equal(resultado.concentracaoMilesimosMgL, 0U);
}

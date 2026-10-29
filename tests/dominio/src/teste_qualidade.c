#include <math.h>

#include <zephyr/ztest.h>

#include "config_produto.h"
#include "dominio/qualidade.h"

/* Com média de 1000 mV, um desvio de X mV corresponde a um CV de X / 10 %. */
#define MEDIA_MV 1000.0f

static float desvioParaCv(float coefVariacaoPct)
{
	return coefVariacaoPct * MEDIA_MV / 100.0f;
}

ZTEST_SUITE(qualidade, NULL, NULL, NULL, NULL, NULL);

ZTEST(qualidade, test_limiares_do_coeficiente_de_variacao)
{
	zassert_equal(qualidade_avaliar(desvioParaCv(QUALIDADE_CV_EXCELENTE_PCT) - 1.0f, MEDIA_MV),
		      QUALIDADE_EXCELENTE);
	zassert_equal(qualidade_avaliar(desvioParaCv(QUALIDADE_CV_EXCELENTE_PCT) + 1.0f, MEDIA_MV),
		      QUALIDADE_BOA);
	zassert_equal(qualidade_avaliar(desvioParaCv(QUALIDADE_CV_BOA_PCT) - 1.0f, MEDIA_MV),
		      QUALIDADE_BOA);
	zassert_equal(qualidade_avaliar(desvioParaCv(QUALIDADE_CV_BOA_PCT) + 1.0f, MEDIA_MV),
		      QUALIDADE_ACEITAVEL);
	zassert_equal(qualidade_avaliar(desvioParaCv(QUALIDADE_CV_ACEITAVEL_PCT) - 1.0f, MEDIA_MV),
		      QUALIDADE_ACEITAVEL);
	zassert_equal(qualidade_avaliar(desvioParaCv(QUALIDADE_CV_ACEITAVEL_PCT) + 1.0f, MEDIA_MV),
		      QUALIDADE_DESCARTAR);
}

ZTEST(qualidade, test_media_nao_positiva_ou_desvio_invalido_descartam)
{
	zassert_equal(qualidade_avaliar(0.0f, 0.0f), QUALIDADE_DESCARTAR);
	zassert_equal(qualidade_avaliar(1.0f, -10.0f), QUALIDADE_DESCARTAR);
	zassert_equal(qualidade_avaliar(NAN, MEDIA_MV), QUALIDADE_DESCARTAR);
	zassert_equal(qualidade_avaliar(1.0f, NAN), QUALIDADE_DESCARTAR);
}

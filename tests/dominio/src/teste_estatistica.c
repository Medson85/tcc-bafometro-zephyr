#include <zephyr/sys/util.h>
#include <zephyr/ztest.h>

#include "dominio/estatistica.h"

ZTEST_SUITE(estatistica, NULL, NULL, NULL, NULL, NULL);

ZTEST(estatistica, test_media_desvio_populacional_minimo_e_maximo)
{
	const int32_t amostras[] = {2, 4, 4, 4, 5, 5, 7, 9};
	struct Estatistica estatistica;

	zassert_true(estatistica_calcular(amostras, ARRAY_SIZE(amostras), &estatistica));
	zassert_within(estatistica.mediaMv, 5.0f, 1e-6f);
	zassert_within(estatistica.desvioPadraoMv, 2.0f, 1e-6f);
	zassert_within(estatistica.coefVariacaoPct, 40.0f, 1e-4f);
	zassert_equal(estatistica.minimoMv, 2);
	zassert_equal(estatistica.maximoMv, 9);
	zassert_equal(estatistica.quantidade, ARRAY_SIZE(amostras));
}

ZTEST(estatistica, test_amostra_unica_tem_desvio_zero)
{
	const int32_t amostras[] = {1234};
	struct Estatistica estatistica;

	zassert_true(estatistica_calcular(amostras, ARRAY_SIZE(amostras), &estatistica));
	zassert_within(estatistica.mediaMv, 1234.0f, 1e-3f);
	zassert_within(estatistica.desvioPadraoMv, 0.0f, 1e-6f);
	zassert_within(estatistica.coefVariacaoPct, 0.0f, 1e-6f);
}

ZTEST(estatistica, test_media_nula_tem_coeficiente_de_variacao_zero)
{
	const int32_t amostras[] = {0, 0, 0};
	struct Estatistica estatistica;

	zassert_true(estatistica_calcular(amostras, ARRAY_SIZE(amostras), &estatistica));
	zassert_within(estatistica.coefVariacaoPct, 0.0f, 1e-6f);
}

ZTEST(estatistica, test_sem_amostras_retorna_falso)
{
	const int32_t amostras[] = {1};
	struct Estatistica estatistica;

	zassert_false(estatistica_calcular(amostras, 0U, &estatistica));
	zassert_false(estatistica_calcular(NULL, 1U, &estatistica));
	zassert_false(estatistica_calcular(amostras, 1U, NULL));
}

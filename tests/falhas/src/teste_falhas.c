/*
 * Testes de falhas (RF10, ADR 0002): a máquina de estados real, ligada a dublês dos
 * dispositivos, recebe falhas injetadas que o GPIO e o ADC emulados não produzem.
 * Toda falha deve levar ao ERRO com o inibidor ativo e o aquecedor em repouso.
 */

#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "aplicacao/bafometro.h"
#include "config_produto.h"
#include "dispositivos_falsos.h"

#define FALHA_DE_ESCRITA (-EIO)

/* 200 ppm com o R0 padrão (Rs/R0 = 0,047): VRL = VCC * RL / (RL + Rs). */
#define TENSAO_ALCOOL_MV                                                                           \
	((int32_t)(TENSAO_ALIMENTACAO_MV * RESISTENCIA_CARGA_OHMS /                                \
			   (RESISTENCIA_CARGA_OHMS + (float)R0_PADRAO_OHMS * 0.047f) +             \
		   0.5f))

static bool processarAte(enum EstadoBafometro alvo, uint32_t limiteMs)
{
	int64_t inicio = k_uptime_get();

	while (bafometro_estadoAtual() != alvo) {
		if ((k_uptime_get() - inicio) > limiteMs) {
			return false;
		}
		(void)bafometro_processarProximoEvento(K_MSEC(10));
	}

	return true;
}

static void processarPor(uint32_t duracaoMs)
{
	int64_t fim = k_uptime_get() + duracaoMs;

	while (k_uptime_get() < fim) {
		(void)bafometro_processarProximoEvento(K_MSEC(5));
	}
}

static void chegarEmAguardandoSopro(void)
{
	zassert_true(processarAte(ESTADO_AGUARDANDO_SOPRO,
				  TEMPO_PRE_AQUECIMENTO_MS + TEMPO_ACOMODACAO_FASE_MS + 100U));
}

static void chegarEmColetando(void)
{
	chegarEmAguardandoSopro();
	falsos_iniciarSopro();
	zassert_true(processarAte(ESTADO_COLETANDO, 10U));
}

/* Estado seguro exigido em qualquer falha. */
static void conferirErro(enum CodigoFalha codigo)
{
	zassert_equal(bafometro_estadoAtual(), ESTADO_ERRO);
	zassert_equal(bafometro_codigoErro(), codigo);
	zassert_true(falsos.inibidorAtivo);
	zassert_equal(falsos.fase, FASE_REPOUSO);
}

static void antesDeCadaTeste(void *fixture)
{
	ARG_UNUSED(fixture);

	/* Cada teste configura as falhas e só então inicia a máquina. */
	falsos_reiniciar();
}

ZTEST_SUITE(falhas, NULL, NULL, antesDeCadaTeste, NULL, NULL);

ZTEST(falhas, test_falha_ao_iniciar_o_inibidor_leva_ao_erro)
{
	falsos.erroInibidorIniciar = -ENODEV;
	zassert_ok(bafometro_iniciar());
	conferirErro(FALHA_INIBIDOR);
}

ZTEST(falhas, test_falha_ao_iniciar_o_detector_leva_ao_erro)
{
	falsos.erroDetectorIniciar = -ENODEV;
	zassert_ok(bafometro_iniciar());
	conferirErro(FALHA_DETECTOR_SOPRO);
}

ZTEST(falhas, test_falha_ao_iniciar_o_sensor_leva_ao_erro)
{
	falsos.erroSensorIniciar = -ENODEV;
	zassert_ok(bafometro_iniciar());
	conferirErro(FALHA_SENSOR_ALCOOL);
}

ZTEST(falhas, test_falha_na_fase_de_aquecimento_leva_ao_erro)
{
	falsos.erroFase[FASE_AQUECIMENTO] = FALHA_DE_ESCRITA;
	zassert_ok(bafometro_iniciar());
	conferirErro(FALHA_AQUECEDOR);
}

ZTEST(falhas, test_falha_na_fase_de_operacao_leva_ao_erro)
{
	falsos.erroFase[FASE_OPERACAO] = FALHA_DE_ESCRITA;
	zassert_ok(bafometro_iniciar());
	zassert_true(processarAte(ESTADO_ERRO, TEMPO_PRE_AQUECIMENTO_MS + 100U));
	conferirErro(FALHA_AQUECEDOR);
}

ZTEST(falhas, test_falha_no_repouso_da_economia_leva_ao_erro)
{
	falsos.erroFase[FASE_REPOUSO] = FALHA_DE_ESCRITA;
	zassert_ok(bafometro_iniciar());
	chegarEmAguardandoSopro();
	zassert_true(processarAte(ESTADO_ERRO, TEMPO_ESPERA_SOPRO_MS + 100U));

	/* O repouso também falha dentro do ERRO; o dublê registra o aquecedor sem energia. */
	conferirErro(FALHA_AQUECEDOR);
}

ZTEST(falhas, test_falha_ao_armar_o_detector_leva_ao_erro)
{
	falsos.erroDetectorArmar = FALHA_DE_ESCRITA;
	zassert_ok(bafometro_iniciar());
	zassert_true(processarAte(ESTADO_ERRO,
				  TEMPO_PRE_AQUECIMENTO_MS + TEMPO_ACOMODACAO_FASE_MS + 100U));
	conferirErro(FALHA_DETECTOR_SOPRO);
}

/* Ar limpo levaria à liberação; o comando falha e o resultado não é apresentado. */
ZTEST(falhas, test_falha_ao_liberar_o_inibidor_leva_ao_erro)
{
	zassert_ok(bafometro_iniciar());
	chegarEmColetando();
	falsos.erroInibidorComando = FALHA_DE_ESCRITA;
	zassert_true(processarAte(ESTADO_ERRO, TEMPO_SOPRO_MINIMO_MS + 100U));
	falsos_encerrarSopro();
	conferirErro(FALHA_INIBIDOR);
}

ZTEST(falhas, test_falha_ao_ativar_o_inibidor_leva_ao_erro)
{
	falsos.tensaoMv = TENSAO_ALCOOL_MV;
	zassert_ok(bafometro_iniciar());
	chegarEmColetando();
	falsos.erroInibidorComando = FALHA_DE_ESCRITA;
	zassert_true(processarAte(ESTADO_ERRO, TEMPO_SOPRO_MINIMO_MS + 100U));
	falsos_encerrarSopro();
	conferirErro(FALHA_INIBIDOR);
}

ZTEST(falhas, test_falha_de_leitura_na_coleta_leva_ao_erro)
{
	zassert_ok(bafometro_iniciar());
	chegarEmColetando();
	processarPor(1000U);
	falsos.erroLeitura = FALHA_DE_ESCRITA;
	zassert_true(processarAte(ESTADO_ERRO, 2U * PERIODO_AMOSTRAGEM_MS));
	falsos_encerrarSopro();
	conferirErro(FALHA_SENSOR_ALCOOL);
}

ZTEST(falhas, test_falha_de_leitura_na_calibracao_leva_ao_erro)
{
	zassert_ok(bafometro_iniciar());
	chegarEmAguardandoSopro();
	falsos.erroLeitura = FALHA_DE_ESCRITA;
	zassert_ok(bafometro_publicarComando(COMANDO_CALIBRAR));
	zassert_true(processarAte(ESTADO_ERRO, 2U * INTERVALO_CALIBRACAO_MS));
	conferirErro(FALHA_SENSOR_ALCOOL);
}

/* RF10 a partir do estado liberado: a falha volta a ativar o inibidor. */
ZTEST(falhas, test_falha_depois_da_liberacao_ativa_o_inibidor)
{
	zassert_ok(bafometro_iniciar());
	chegarEmColetando();
	zassert_true(processarAte(ESTADO_APRESENTANDO_RESULTADO, TEMPO_SOPRO_MINIMO_MS + 100U));
	falsos_encerrarSopro();
	zassert_false(falsos.inibidorAtivo);
	zassert_equal(falsos.fase, FASE_OPERACAO);

	zassert_ok(bafometro_publicarFalha(FALHA_SENSOR_ALCOOL));
	zassert_true(processarAte(ESTADO_ERRO, 10U));
	conferirErro(FALHA_SENSOR_ALCOOL);
}

/*
 * Testes da máquina de estados no native_sim, com eventos injetados:
 * o detector é acionado pelo GPIO emulado e o tempo é o simulado.
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>

#include "aplicacao/bafometro.h"
#include "config_produto.h"
#include "dispositivos/inibidor.h"

static const struct gpio_dt_spec pinoDetector =
	GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), detector_sopro_gpios);

static void soprar(bool presente)
{
	zassert_ok(gpio_emul_input_set(pinoDetector.port, pinoDetector.pin, presente ? 1 : 0));
}

/* Processa eventos até a máquina chegar ao estado alvo ou o tempo simulado esgotar. */
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

/* Processa eventos durante um intervalo de tempo simulado. */
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
	soprar(true);
	zassert_true(processarAte(ESTADO_COLETANDO, 10U));
}

static void antesDeCadaTeste(void *fixture)
{
	ARG_UNUSED(fixture);

	/* O pino só aceita valores depois de configurado na inicialização da máquina. */
	zassert_ok(bafometro_iniciar());
	soprar(false);
}

ZTEST_SUITE(maquina, NULL, NULL, antesDeCadaTeste, NULL, NULL);

ZTEST(maquina, test_inicia_com_inibidor_ativo_em_pre_aquecimento)
{
	zassert_equal(bafometro_estadoAtual(), ESTADO_PRE_AQUECENDO);
	zassert_true(inibidor_estaAtivo());
}

ZTEST(maquina, test_pre_aquecimento_e_acomodacao_duram_o_configurado)
{
	int64_t inicio = k_uptime_get();

	chegarEmAguardandoSopro();
	zassert_within(k_uptime_get() - inicio, TEMPO_PRE_AQUECIMENTO_MS + TEMPO_ACOMODACAO_FASE_MS,
		       2);
}

ZTEST(maquina, test_sopro_continuo_coleta_por_5s_e_mantem_inibidor_ativo)
{
	int64_t inicio;

	chegarEmColetando();
	inicio = k_uptime_get();
	zassert_true(processarAte(ESTADO_APRESENTANDO_RESULTADO, TEMPO_SOPRO_MINIMO_MS + 100U));
	zassert_within(k_uptime_get() - inicio, TEMPO_SOPRO_MINIMO_MS, 2);
	soprar(false);

	/* Sem resultado válido (etapa 1), o inibidor não pode ser liberado. */
	zassert_true(inibidor_estaAtivo());

	zassert_true(processarAte(ESTADO_FINALIZANDO, TEMPO_APRESENTACAO_MS + 100U));
	zassert_true(processarAte(ESTADO_AGUARDANDO_SOPRO, TEMPO_MIN_ENTRE_TESTES_MS + 100U));
}

ZTEST(maquina, test_sopro_interrompido_aos_3s_invalida_a_coleta)
{
	chegarEmColetando();
	processarPor(3000U);
	zassert_equal(bafometro_estadoAtual(), ESTADO_COLETANDO);

	soprar(false);
	zassert_true(processarAte(ESTADO_AGUARDANDO_SOPRO,
				  TOLERANCIA_QUEDA_SOPRO_MS + 2U * PERIODO_AMOSTRAGEM_MS));
}

ZTEST(maquina, test_microqueda_dentro_da_tolerancia_nao_invalida)
{
	chegarEmColetando();
	processarPor(1000U);
	soprar(false);
	processarPor(60U);
	soprar(true);

	zassert_true(processarAte(ESTADO_APRESENTANDO_RESULTADO, TEMPO_SOPRO_MINIMO_MS));
	soprar(false);
}

ZTEST(maquina, test_espera_esgotada_entra_em_economia_e_sopro_desperta)
{
	int64_t inicio;

	chegarEmAguardandoSopro();
	inicio = k_uptime_get();
	zassert_true(processarAte(ESTADO_ECONOMIA, TEMPO_ESPERA_SOPRO_MS + 100U));
	zassert_within(k_uptime_get() - inicio, TEMPO_ESPERA_SOPRO_MS, 2);

	soprar(true);
	zassert_true(processarAte(ESTADO_PRE_AQUECENDO, 10U));
	soprar(false);
}

ZTEST(maquina, test_comando_reinicia_a_espera_pelo_sopro)
{
	chegarEmAguardandoSopro();
	processarPor(TEMPO_ESPERA_SOPRO_MS - 1000U);
	zassert_ok(bafometro_publicarComando(COMANDO_INICIAR));

	/* O prazo original vence aqui, mas a espera foi reiniciada. */
	processarPor(1500U);
	zassert_equal(bafometro_estadoAtual(), ESTADO_AGUARDANDO_SOPRO);
	zassert_true(processarAte(ESTADO_ECONOMIA, TEMPO_ESPERA_SOPRO_MS));
}

ZTEST(maquina, test_evento_de_tempo_obsoleto_e_descartado)
{
	struct EventoBafometro obsoleto = {
		.tipo = EVENTO_TEMPO,
		.geracao = 0U,
	};
	uint32_t descartadosAntes;

	chegarEmAguardandoSopro();
	descartadosAntes = bafometro_eventosDescartados();

	zassert_ok(bafometro_publicarEvento(&obsoleto));
	zassert_ok(bafometro_processarProximoEvento(K_NO_WAIT));
	zassert_equal(bafometro_estadoAtual(), ESTADO_AGUARDANDO_SOPRO);
	zassert_equal(bafometro_eventosDescartados(), descartadosAntes + 1U);
}

ZTEST(maquina, test_fila_cheia_durante_a_coleta_invalida_a_medicao)
{
	uint32_t perdidosAntes;

	chegarEmColetando();
	perdidosAntes = bafometro_eventosPerdidos();

	/* Enche a fila sem processar: os tiques de amostragem seguintes se perdem. */
	for (int i = 0; i < QTD_POSICOES_FILA_EVENTOS; i++) {
		(void)bafometro_publicarComando(COMANDO_INICIAR);
	}
	k_sleep(K_MSEC(2U * PERIODO_AMOSTRAGEM_MS));
	zassert_true(bafometro_eventosPerdidos() > perdidosAntes);

	zassert_true(processarAte(ESTADO_AGUARDANDO_SOPRO, 3U * PERIODO_AMOSTRAGEM_MS));
	soprar(false);
}

ZTEST(maquina, test_cancelar_na_espera_entra_em_economia)
{
	chegarEmAguardandoSopro();
	zassert_ok(bafometro_publicarComando(COMANDO_CANCELAR));
	zassert_true(processarAte(ESTADO_ECONOMIA, 10U));
}

ZTEST(maquina, test_falha_leva_ao_erro_com_inibidor_ativo)
{
	chegarEmAguardandoSopro();
	zassert_ok(bafometro_publicarFalha(FALHA_SENSOR_ALCOOL));
	zassert_true(processarAte(ESTADO_ERRO, 10U));
	zassert_true(inibidor_estaAtivo());

	/* Em erro, só o comando de reinício tira a máquina do estado. */
	soprar(true);
	zassert_ok(bafometro_publicarComando(COMANDO_INICIAR));
	processarPor(100U);
	zassert_equal(bafometro_estadoAtual(), ESTADO_ERRO);
	soprar(false);

	zassert_ok(bafometro_publicarComando(COMANDO_REINICIAR));
	zassert_true(processarAte(ESTADO_PRE_AQUECENDO, 10U));
	zassert_true(inibidor_estaAtivo());
}

ZTEST(maquina, test_calibrar_na_espera_volta_a_aguardar)
{
	int64_t inicio;

	chegarEmAguardandoSopro();
	zassert_ok(bafometro_publicarComando(COMANDO_CALIBRAR));
	zassert_true(processarAte(ESTADO_CALIBRANDO_R0, 10U));

	inicio = k_uptime_get();
	zassert_true(processarAte(ESTADO_AGUARDANDO_SOPRO,
				  QTD_AMOSTRAS_CALIBRACAO * INTERVALO_CALIBRACAO_MS + 100U));
	zassert_within(k_uptime_get() - inicio, QTD_AMOSTRAS_CALIBRACAO * INTERVALO_CALIBRACAO_MS,
		       2);
}

ZTEST(maquina, test_calibrar_em_economia_pre_aquece_antes_de_calibrar)
{
	chegarEmAguardandoSopro();
	zassert_true(processarAte(ESTADO_ECONOMIA, TEMPO_ESPERA_SOPRO_MS + 100U));

	zassert_ok(bafometro_publicarComando(COMANDO_CALIBRAR));
	zassert_true(processarAte(ESTADO_PRE_AQUECENDO, 10U));
	zassert_true(processarAte(ESTADO_CALIBRANDO_R0,
				  TEMPO_PRE_AQUECIMENTO_MS + TEMPO_ACOMODACAO_FASE_MS + 100U));
}

ZTEST(maquina, test_sopro_antes_do_aviso_e_ignorado)
{
	/* Sopro durante o pré-aquecimento: o detector ainda não está armado. */
	soprar(true);
	processarPor(100U);
	zassert_equal(bafometro_estadoAtual(), ESTADO_PRE_AQUECENDO);

	/* Mesmo contínuo, ele não conta: é preciso iniciar um sopro após o aviso. */
	chegarEmAguardandoSopro();
	processarPor(100U);
	zassert_equal(bafometro_estadoAtual(), ESTADO_AGUARDANDO_SOPRO);

	soprar(false);
	soprar(true);
	zassert_true(processarAte(ESTADO_COLETANDO, 10U));
	soprar(false);
}

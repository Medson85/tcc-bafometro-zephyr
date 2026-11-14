/*
 * Roteiro de simulação, só no native_sim: percorre o fluxo completo sem
 * hardware, acionando o detector pelo GPIO emulado, definindo a tensão sobre
 * RL no ADC emulado e enviando comandos pela fila de eventos. Esta thread
 * existe apenas na simulação; no firmware há uma única thread de aplicação.
 */

#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/adc/adc_emul.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

#include <posix_board_if.h>

#include "aplicacao/bafometro.h"
#include "config_produto.h"

LOG_MODULE_REGISTER(roteiro);

static const struct gpio_dt_spec pinoDetector =
	GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), detector_sopro_gpios);
static const struct adc_dt_spec canalSensor = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

static void soprar(bool presente)
{
	(void)gpio_emul_input_set(pinoDetector.port, pinoDetector.pin, presente ? 1 : 0);
}

/* Tensão sobre RL para uma razão Rs/R0, com o R0 padrão: VRL = VCC * RL / (RL + Rs). */
static void definirRelacaoRsR0(float relacaoRsR0)
{
	const float rsOhms = (float)R0_PADRAO_OHMS * relacaoRsR0;
	const float tensaoMv =
		TENSAO_ALIMENTACAO_MV * RESISTENCIA_CARGA_OHMS / (RESISTENCIA_CARGA_OHMS + rsOhms);

	(void)adc_emul_const_value_set(canalSensor.dev, canalSensor.channel_id,
				       (uint32_t)(tensaoMv + 0.5f));
}

static void aguardarEstado(enum EstadoBafometro alvo)
{
	while (bafometro_estadoAtual() != alvo) {
		k_sleep(K_MSEC(10));
	}
}

static void soprarPorCincoSegundos(void)
{
	k_sleep(K_SECONDS(1));
	soprar(true);
	aguardarEstado(ESTADO_APRESENTANDO_RESULTADO);
	soprar(false);
}

static void executarRoteiro(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	definirRelacaoRsR0(1.0f); /* ar limpo */

	LOG_INF("--- Roteiro 1: sopro interrompido aos 3 s ---");
	aguardarEstado(ESTADO_AGUARDANDO_SOPRO);
	soprar(true);
	k_sleep(K_SECONDS(3));
	soprar(false);
	aguardarEstado(ESTADO_AGUARDANDO_SOPRO);

	LOG_INF("--- Roteiro 2: sopro com álcool (Rs/R0 = 0,047, 200 ppm): inibe ---");
	definirRelacaoRsR0(0.047f);
	soprarPorCincoSegundos();

	LOG_INF("--- Roteiro 3: depois da recuperação, sopro em ar limpo: libera ---");
	definirRelacaoRsR0(1.0f);
	aguardarEstado(ESTADO_AGUARDANDO_SOPRO);
	soprarPorCincoSegundos();

	LOG_INF("--- Roteiro 4: sem sopro, a espera esgota e o aparelho entra em economia ---");
	aguardarEstado(ESTADO_ECONOMIA);

	LOG_INF("--- Roteiro 5: sopro desperta da economia ---");
	k_sleep(K_SECONDS(2));
	soprar(true);
	k_sleep(K_MSEC(100));
	soprar(false);
	aguardarEstado(ESTADO_AGUARDANDO_SOPRO);

	LOG_INF("--- Roteiro 6: calibração em ar limpo ---");
	(void)bafometro_publicarComando(COMANDO_CALIBRAR);
	aguardarEstado(ESTADO_CALIBRANDO_R0);
	aguardarEstado(ESTADO_AGUARDANDO_SOPRO);

	LOG_INF("--- Roteiro 7: sensor desconectado: falha, ERRO e inibidor ativo ---");
	(void)adc_emul_const_value_set(canalSensor.dev, canalSensor.channel_id, 5U);
	k_sleep(K_SECONDS(1));
	soprar(true);
	aguardarEstado(ESTADO_ERRO);
	soprar(false);

	LOG_INF("--- Roteiro concluído ---");
	LOG_PANIC();
	posix_exit(0);
}

K_THREAD_DEFINE(threadRoteiro, 2048, executarRoteiro, NULL, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);

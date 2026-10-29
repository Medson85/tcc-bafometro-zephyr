/*
 * Roteiro de simulação, só no native_sim: percorre o fluxo completo sem
 * hardware, acionando o detector pelo GPIO emulado e enviando comandos pela
 * fila de eventos. Esta thread existe apenas na simulação; no firmware há
 * uma única thread de aplicação.
 */

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

#include <posix_board_if.h>

#include "aplicacao/bafometro.h"

LOG_MODULE_REGISTER(roteiro);

static const struct gpio_dt_spec pinoDetector =
	GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), detector_sopro_gpios);

static void soprar(bool presente)
{
	(void)gpio_emul_input_set(pinoDetector.port, pinoDetector.pin, presente ? 1 : 0);
}

static void aguardarEstado(enum EstadoBafometro alvo)
{
	while (bafometro_estadoAtual() != alvo) {
		k_sleep(K_MSEC(10));
	}
}

static void executarRoteiro(void *p1, void *p2, void *p3)
{
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	LOG_INF("--- Roteiro 1: sopro interrompido aos 3 s ---");
	aguardarEstado(ESTADO_AGUARDANDO_SOPRO);
	soprar(true);
	k_sleep(K_SECONDS(3));
	soprar(false);
	aguardarEstado(ESTADO_AGUARDANDO_SOPRO);

	LOG_INF("--- Roteiro 2: sopro de 5 s contínuos ---");
	k_sleep(K_SECONDS(1));
	soprar(true);
	aguardarEstado(ESTADO_APRESENTANDO_RESULTADO);
	soprar(false);

	LOG_INF("--- Roteiro 3: sem sopro, a espera esgota e o aparelho entra em economia ---");
	aguardarEstado(ESTADO_ECONOMIA);

	LOG_INF("--- Roteiro 4: sopro desperta da economia ---");
	k_sleep(K_SECONDS(2));
	soprar(true);
	k_sleep(K_MSEC(100));
	soprar(false);
	aguardarEstado(ESTADO_AGUARDANDO_SOPRO);

	LOG_INF("--- Roteiro 5: comando de calibração ---");
	(void)bafometro_publicarComando(COMANDO_CALIBRAR);
	aguardarEstado(ESTADO_CALIBRANDO_R0);
	aguardarEstado(ESTADO_AGUARDANDO_SOPRO);

	LOG_INF("--- Roteiro concluído ---");
	LOG_PANIC();
	posix_exit(0);
}

K_THREAD_DEFINE(threadRoteiro, 2048, executarRoteiro, NULL, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);

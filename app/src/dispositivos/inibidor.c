/*
 * Saída de inibição pelo pino inibidor-gpios do zephyr,user. Placas sem o
 * pino mantêm só o estado lógico, registrado em LOG.
 */

#include <errno.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "dispositivos/inibidor.h"

LOG_MODULE_REGISTER(inibidor);

#define TEM_PINO DT_NODE_HAS_PROP(DT_PATH(zephyr_user), inibidor_gpios)

#if TEM_PINO
static const struct gpio_dt_spec pinoInibidor =
	GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), inibidor_gpios);
#endif

static bool ativo = true;

static int aplicar(bool novoEstado)
{
	int erro = 0;

#if TEM_PINO
	erro = gpio_pin_set_dt(&pinoInibidor, novoEstado ? 1 : 0);
#endif
	if (erro != 0) {
		/* Sem confirmação da escrita, o estado lógico não pode afirmar a liberação. */
		ativo = true;
		LOG_ERR("Falha ao comandar o inibidor (%d)", erro);
		return erro;
	}

	ativo = novoEstado;
	LOG_INF("Inibidor %s", ativo ? "ATIVO" : "LIBERADO");
	return 0;
}

int inibidor_iniciar(void)
{
	ativo = true;
#if TEM_PINO
	if (!gpio_is_ready_dt(&pinoInibidor) ||
	    (gpio_pin_configure_dt(&pinoInibidor, GPIO_OUTPUT_ACTIVE) != 0)) {
		LOG_ERR("Falha ao configurar o pino do inibidor");
		return -ENODEV;
	}
#else
	LOG_WRN("Placa sem pino do inibidor no devicetree: estado só em LOG");
#endif
	return aplicar(true);
}

int inibidor_ativar(void)
{
	return aplicar(true);
}

int inibidor_liberar(void)
{
	return aplicar(false);
}

bool inibidor_estaAtivo(void)
{
	return ativo;
}

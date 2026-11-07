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

static void aplicar(void)
{
#if TEM_PINO
	(void)gpio_pin_set_dt(&pinoInibidor, ativo ? 1 : 0);
#endif
	LOG_INF("Inibidor %s", ativo ? "ATIVO" : "LIBERADO");
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
	aplicar();
	return 0;
}

void inibidor_ativar(void)
{
	ativo = true;
	aplicar();
}

void inibidor_liberar(void)
{
	ativo = false;
	aplicar();
}

bool inibidor_estaAtivo(void)
{
	return ativo;
}

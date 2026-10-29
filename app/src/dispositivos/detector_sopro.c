#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "dispositivos/detector_sopro.h"

LOG_MODULE_REGISTER(detector_sopro);

static const struct gpio_dt_spec pinoDetector =
	GPIO_DT_SPEC_GET(DT_PATH(zephyr_user), detector_sopro_gpios);

static struct gpio_callback callbackDetector;
static AoDetectarSopro aoDetectarSopro;
static bool iniciado;

static void aoInterromper(const struct device *porta, struct gpio_callback *callback,
			  gpio_port_pins_t pinos)
{
	ARG_UNUSED(porta);
	ARG_UNUSED(callback);
	ARG_UNUSED(pinos);

	if (aoDetectarSopro != NULL) {
		aoDetectarSopro();
	}
}

int detector_sopro_iniciar(AoDetectarSopro aoDetectar)
{
	int erro;

	aoDetectarSopro = aoDetectar;
	if (iniciado) {
		return 0;
	}

	if (!gpio_is_ready_dt(&pinoDetector)) {
		LOG_ERR("GPIO do detector indisponível");
		return -ENODEV;
	}

	erro = gpio_pin_configure_dt(&pinoDetector, GPIO_INPUT);
	if (erro != 0) {
		LOG_ERR("Falha ao configurar o detector (%d)", erro);
		return erro;
	}

	gpio_init_callback(&callbackDetector, aoInterromper, BIT(pinoDetector.pin));
	erro = gpio_add_callback_dt(&pinoDetector, &callbackDetector);
	if (erro != 0) {
		LOG_ERR("Falha ao registrar o callback do detector (%d)", erro);
		return erro;
	}

	iniciado = true;
	return 0;
}

int detector_sopro_armar(void)
{
	if (!iniciado) {
		return -ENODEV;
	}

	return gpio_pin_interrupt_configure_dt(&pinoDetector, GPIO_INT_EDGE_TO_ACTIVE);
}

int detector_sopro_desarmar(void)
{
	if (!iniciado) {
		return 0;
	}

	return gpio_pin_interrupt_configure_dt(&pinoDetector, GPIO_INT_DISABLE);
}

bool detector_sopro_presente(void)
{
	return iniciado && (gpio_pin_get_dt(&pinoDetector) > 0);
}

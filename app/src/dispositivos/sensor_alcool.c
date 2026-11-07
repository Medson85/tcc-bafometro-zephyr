/*
 * Etapa 3: aquecedor e ADC pelo devicetree. Placas sem os pinos do aquecedor
 * no zephyr,user só registram a fase em LOG; sem io-channels, a leitura falha.
 * A persistência do R0 entra na etapa 6.
 */

#include <errno.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "dispositivos/sensor_alcool.h"

LOG_MODULE_REGISTER(sensor_alcool);

#define NO_USUARIO DT_PATH(zephyr_user)

#define TEM_AQUECEDOR                                                                              \
	(DT_NODE_HAS_PROP(NO_USUARIO, aquecimento2v_gpios) &&                                      \
	 DT_NODE_HAS_PROP(NO_USUARIO, aquecimento0v9_gpios))
#define TEM_ADC DT_NODE_HAS_PROP(NO_USUARIO, io_channels)

#if TEM_AQUECEDOR
static const struct gpio_dt_spec pinoAquecimento2V =
	GPIO_DT_SPEC_GET(NO_USUARIO, aquecimento2v_gpios); /* Q5 */
static const struct gpio_dt_spec pinoAquecimento0V9 =
	GPIO_DT_SPEC_GET(NO_USUARIO, aquecimento0v9_gpios); /* Q7 */
#endif

#if TEM_ADC
static const struct adc_dt_spec canalSensor = ADC_DT_SPEC_GET(NO_USUARIO);
#endif

int sensor_alcool_iniciar(void)
{
	int erro;

#if TEM_AQUECEDOR
	if (!gpio_is_ready_dt(&pinoAquecimento2V) || !gpio_is_ready_dt(&pinoAquecimento0V9)) {
		LOG_ERR("GPIO do aquecedor indisponível");
		return -ENODEV;
	}

	/* Os dois nascem desligados, antes de qualquer fase. */
	erro = gpio_pin_configure_dt(&pinoAquecimento2V, GPIO_OUTPUT_INACTIVE);
	if (erro == 0) {
		erro = gpio_pin_configure_dt(&pinoAquecimento0V9, GPIO_OUTPUT_INACTIVE);
	}
	if (erro != 0) {
		LOG_ERR("Falha ao configurar o aquecedor (%d)", erro);
		return erro;
	}
#else
	LOG_WRN("Placa sem pinos do aquecedor no devicetree: fases só em LOG");
#endif

#if TEM_ADC
	if (!adc_is_ready_dt(&canalSensor)) {
		LOG_ERR("ADC indisponível");
		return -ENODEV;
	}

	erro = adc_channel_setup_dt(&canalSensor);
	if (erro != 0) {
		LOG_ERR("Falha ao configurar o canal do ADC (%d)", erro);
		return erro;
	}
#else
	LOG_WRN("Placa sem canal de ADC no devicetree: leituras indisponíveis");
#endif

	ARG_UNUSED(erro);
	return 0;
}

void sensor_alcool_definirFase(enum FaseAquecedor fase)
{
	const bool liga2V = (fase == FASE_AQUECIMENTO);
	const bool liga0V9 = (fase == FASE_OPERACAO);

#if TEM_AQUECEDOR
	/* Desliga antes de ligar: as duas fases nunca conduzem juntas. */
	if (!liga2V) {
		(void)gpio_pin_set_dt(&pinoAquecimento2V, 0);
	}
	if (!liga0V9) {
		(void)gpio_pin_set_dt(&pinoAquecimento0V9, 0);
	}
	if (liga2V) {
		(void)gpio_pin_set_dt(&pinoAquecimento2V, 1);
	}
	if (liga0V9) {
		(void)gpio_pin_set_dt(&pinoAquecimento0V9, 1);
	}
#endif

	LOG_INF("Aquecedor: %s (Q5=%d, Q7=%d)",
		liga2V ? "aquecimento" : (liga0V9 ? "operação" : "repouso"), liga2V, liga0V9);
}

int sensor_alcool_lerTensaoMv(int32_t *tensaoMv)
{
#if TEM_ADC
	int16_t bruto = 0;
	struct adc_sequence sequencia = {
		.buffer = &bruto,
		.buffer_size = sizeof(bruto),
	};
	int erro = adc_sequence_init_dt(&canalSensor, &sequencia);

	if (erro == 0) {
		erro = adc_read_dt(&canalSensor, &sequencia);
	}
	if (erro != 0) {
		return erro;
	}

	*tensaoMv = bruto;
	return adc_raw_to_millivolts_dt(&canalSensor, tensaoMv);
#else
	ARG_UNUSED(tensaoMv);
	return -ENOTSUP;
#endif
}

int sensor_alcool_carregarR0(uint32_t *r0Ohms)
{
	ARG_UNUSED(r0Ohms);
	return -ENOENT;
}

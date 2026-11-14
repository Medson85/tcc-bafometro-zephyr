#include <errno.h>

#include <zephyr/sys/util.h>

#include "config_produto.h"
#include "dispositivos/ble.h"
#include "dispositivos/detector_sopro.h"
#include "dispositivos/inibidor.h"
#include "dispositivos/sensor_alcool.h"
#include "dispositivos/tela.h"
#include "dispositivos_falsos.h"

struct DispositivosFalsos falsos;

/* Ar limpo com o R0 padrão: VRL = VCC * RL / (RL + R0). */
#define TENSAO_AR_LIMPO_MV                                                                         \
	((int32_t)(TENSAO_ALIMENTACAO_MV * RESISTENCIA_CARGA_OHMS /                                \
			   (RESISTENCIA_CARGA_OHMS + (float)R0_PADRAO_OHMS) +                      \
		   0.5f))

void falsos_reiniciar(void)
{
	falsos = (struct DispositivosFalsos){
		.tensaoMv = TENSAO_AR_LIMPO_MV,
		.inibidorAtivo = true,
		.fase = FASE_REPOUSO,
	};
}

void falsos_iniciarSopro(void)
{
	falsos.soproPresente = true;
	if (falsos.detectorArmado && (falsos.aoDetectar != NULL)) {
		falsos.aoDetectar();
	}
}

void falsos_encerrarSopro(void)
{
	falsos.soproPresente = false;
}

/* Inibidor: mesmo contrato do módulo real; em erro, o estado lógico fica ativo. */
static int comandarInibidor(bool ativo)
{
	if (falsos.erroInibidorComando != 0) {
		falsos.inibidorAtivo = true;
		return falsos.erroInibidorComando;
	}

	falsos.inibidorAtivo = ativo;
	return 0;
}

int inibidor_iniciar(void)
{
	falsos.inibidorAtivo = true;
	return falsos.erroInibidorIniciar;
}

int inibidor_ativar(void)
{
	return comandarInibidor(true);
}

int inibidor_liberar(void)
{
	return comandarInibidor(false);
}

bool inibidor_estaAtivo(void)
{
	return falsos.inibidorAtivo;
}

/* Sensor: a fase só muda quando a troca não falha. */
int sensor_alcool_iniciar(void)
{
	return falsos.erroSensorIniciar;
}

int sensor_alcool_definirFase(enum FaseAquecedor fase)
{
	if (falsos.erroFase[fase] != 0) {
		falsos.fase = FASE_REPOUSO;
		return falsos.erroFase[fase];
	}

	falsos.fase = fase;
	return 0;
}

int sensor_alcool_lerTensaoMv(int32_t *tensaoMv)
{
	if (falsos.erroLeitura != 0) {
		return falsos.erroLeitura;
	}

	*tensaoMv = falsos.tensaoMv;
	return 0;
}

int sensor_alcool_carregarR0(uint32_t *r0Ohms)
{
	ARG_UNUSED(r0Ohms);
	return -ENOENT;
}

/* Detector */
int detector_sopro_iniciar(AoDetectarSopro aoDetectar)
{
	falsos.aoDetectar = aoDetectar;
	return falsos.erroDetectorIniciar;
}

int detector_sopro_armar(void)
{
	if (falsos.erroDetectorArmar != 0) {
		return falsos.erroDetectorArmar;
	}

	falsos.detectorArmado = true;
	return 0;
}

int detector_sopro_desarmar(void)
{
	falsos.detectorArmado = false;
	return 0;
}

bool detector_sopro_presente(void)
{
	return falsos.soproPresente;
}

/* Tela e BLE: sem efeito nos testes de falhas. */
void tela_mostrarMensagem(const char *texto)
{
	ARG_UNUSED(texto);
}

void tela_mostrarContagem(const char *texto, uint32_t segundos)
{
	ARG_UNUSED(texto);
	ARG_UNUSED(segundos);
}

void tela_mostrarResultado(const struct ResultadoMedicao *resultado)
{
	ARG_UNUSED(resultado);
}

void tela_apagar(void)
{
}

void ble_notificarEstado(uint8_t estado)
{
	ARG_UNUSED(estado);
}

void ble_notificarResultado(const struct ResultadoMedicao *resultado)
{
	ARG_UNUSED(resultado);
}

void ble_definirAnuncioLento(bool lento)
{
	ARG_UNUSED(lento);
}

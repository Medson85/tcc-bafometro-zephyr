/*
 * Dublês dos módulos de dispositivo para os testes de falhas: mesmas assinaturas
 * de app/src/dispositivos, com retornos controlados pelo teste e estado observável.
 */

#ifndef BAFOMETRO_DISPOSITIVOS_FALSOS_H
#define BAFOMETRO_DISPOSITIVOS_FALSOS_H

#include <stdbool.h>
#include <stdint.h>

#include "dispositivos/detector_sopro.h"
#include "dispositivos/sensor_alcool.h"

#define QTD_FASES_AQUECEDOR 3

struct DispositivosFalsos {
	/* Erros devolvidos às chamadas (0 = sucesso). */
	int erroInibidorIniciar;
	int erroInibidorComando; /* inibidor_ativar e inibidor_liberar */
	int erroDetectorIniciar;
	int erroDetectorArmar;
	int erroSensorIniciar;
	int erroFase[QTD_FASES_AQUECEDOR];
	int erroLeitura;

	/* Entradas simuladas. */
	int32_t tensaoMv;
	bool soproPresente;

	/* Estado observado. */
	bool inibidorAtivo;
	enum FaseAquecedor fase;
	bool detectorArmado;
	AoDetectarSopro aoDetectar;
};

extern struct DispositivosFalsos falsos;

/* Sem erros, ar limpo com o R0 padrão, inibidor ativo e aquecedor em repouso. */
void falsos_reiniciar(void);

/* Início de um sopro: marca a presença e, se o detector estiver armado, avisa a máquina. */
void falsos_iniciarSopro(void);
void falsos_encerrarSopro(void);

#endif /* BAFOMETRO_DISPOSITIVOS_FALSOS_H */

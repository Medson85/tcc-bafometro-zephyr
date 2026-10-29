/*
 * Máquina de estados do bafômetro: fila de eventos tipada, temporizador
 * único com geração e SMF plano, executados por uma única thread.
 */

#ifndef BAFOMETRO_BAFOMETRO_H
#define BAFOMETRO_BAFOMETRO_H

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>

enum EstadoBafometro {
	ESTADO_INICIALIZANDO,
	ESTADO_PRE_AQUECENDO,
	ESTADO_AGUARDANDO_SOPRO,
	ESTADO_COLETANDO,
	ESTADO_PROCESSANDO,
	ESTADO_APRESENTANDO_RESULTADO,
	ESTADO_FINALIZANDO,
	ESTADO_ECONOMIA,
	ESTADO_CALIBRANDO_R0,
	ESTADO_ERRO,
	QTD_ESTADOS,
};

enum TipoEvento {
	EVENTO_TEMPO,
	EVENTO_SOPRO,
	EVENTO_COMANDO,
	EVENTO_FALHA,
};

enum CodigoComando {
	COMANDO_INICIAR,
	COMANDO_REPETIR,
	COMANDO_CANCELAR,
	COMANDO_CALIBRAR,
	COMANDO_REINICIAR,
};

enum CodigoFalha {
	FALHA_NENHUMA,
	FALHA_DETECTOR_SOPRO,
	FALHA_SENSOR_ALCOOL,
};

struct EventoBafometro {
	uint8_t tipo;     /* enum TipoEvento */
	uint8_t codigo;   /* finalidade, enum CodigoComando ou enum CodigoFalha */
	uint16_t geracao; /* só em EVENTO_TEMPO: descarta temporizações obsoletas */
	uint32_t valor;   /* auxiliar, opcional */
};

/* Inicia (ou reinicia) a máquina no estado INICIALIZANDO. */
int bafometro_iniciar(void);

/*
 * Espera o próximo evento da fila e o entrega à máquina.
 * Retorna 0 se um evento foi tratado ou descartado, ou o erro de k_msgq_get().
 */
int bafometro_processarProximoEvento(k_timeout_t espera);

/* Publicação de eventos; seguras em interrupção. Retornam o erro de k_msgq_put(). */
int bafometro_publicarEvento(const struct EventoBafometro *evento);
int bafometro_publicarComando(enum CodigoComando comando);
int bafometro_publicarFalha(enum CodigoFalha falha);

enum EstadoBafometro bafometro_estadoAtual(void);
const char *bafometro_nomeEstado(enum EstadoBafometro estado);

/* Diagnóstico: eventos perdidos por fila cheia e de tempo descartados por geração. */
uint32_t bafometro_eventosPerdidos(void);
uint32_t bafometro_eventosDescartados(void);

#endif /* BAFOMETRO_BAFOMETRO_H */

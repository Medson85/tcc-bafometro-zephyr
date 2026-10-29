/*
 * Máquina de estados do bafômetro.
 *
 * Regras do esqueleto (etapa 1):
 * - todo tempo chega como EVENTO_TEMPO do único k_timer; nenhum estado espera;
 * - cada entrada de estado arma ou para o temporizador, e cada armação
 *   incrementa a geração, descartando eventos de tempo antigos;
 * - callbacks (temporizador e GPIO) apenas publicam o evento na fila;
 * - smf_set_state() só em ações de entrada ou execução, como última instrução.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/smf.h>
#include <zephyr/sys/atomic.h>

#include "aplicacao/bafometro.h"
#include "config_produto.h"
#include "dispositivos/ble.h"
#include "dispositivos/detector_sopro.h"
#include "dispositivos/inibidor.h"
#include "dispositivos/sensor_alcool.h"
#include "dispositivos/tela.h"
#include "dominio/decisao.h"
#include "dominio/resultado.h"

LOG_MODULE_REGISTER(bafometro);

BUILD_ASSERT(sizeof(struct EventoBafometro) == 8U, "o evento deve ocupar 8 bytes");
BUILD_ASSERT((TEMPO_PRE_AQUECIMENTO_MS % PERIODO_CONTAGEM_MS) == 0U,
	     "o pré-aquecimento deve ser múltiplo do período de contagem");
BUILD_ASSERT(QTD_TIQUES_PRE_AQUECIMENTO >= 1U, "pré-aquecimento menor que um período de contagem");
BUILD_ASSERT((TEMPO_SOPRO_MINIMO_MS % PERIODO_AMOSTRAGEM_MS) == 0U,
	     "a coleta deve ser múltipla do período de amostragem");

/* Finalidade de cada armação do temporizador (campo codigo de EVENTO_TEMPO). */
enum FinalidadeTempo {
	FINALIDADE_CONTAGEM,
	FINALIDADE_ACOMODACAO,
	FINALIDADE_ESPERA_SOPRO,
	FINALIDADE_AMOSTRAGEM,
	FINALIDADE_APRESENTACAO,
	FINALIDADE_RECUPERACAO,
	FINALIDADE_CALIBRACAO,
};

struct ContextoBafometro {
	struct smf_ctx smf; /* primeiro campo, exigido pelo SMF */
	struct EventoBafometro eventoAtual;
	uint16_t geracaoAtual;
	uint16_t tiquesRestantes;
	uint16_t tiquesColetados;
	uint16_t ausenciaSoproMs;
	atomic_val_t perdasNoInicioColeta;
	bool acomodando;
	bool calibracaoPendente;
	enum CodigoFalha codigoErro;
	uint32_t r0Ohms;
	bool r0Calibrado;
	uint16_t sequenciaMedicao;
	struct ResultadoMedicao resultado;
};

static const struct smf_state estados[QTD_ESTADOS];
static struct ContextoBafometro contexto;

static atomic_t eventosPerdidos;
static atomic_t eventosDescartados;

/* Geração e finalidade da armação corrente, lidas pelo callback do temporizador. */
static uint16_t geracaoArmada;
static uint8_t finalidadeArmada;

K_MSGQ_DEFINE(filaEventos, sizeof(struct EventoBafometro), QTD_POSICOES_FILA_EVENTOS, 4);

static const char *const nomesEstados[QTD_ESTADOS] = {
	[ESTADO_INICIALIZANDO] = "INICIALIZANDO",
	[ESTADO_PRE_AQUECENDO] = "PRE_AQUECENDO",
	[ESTADO_AGUARDANDO_SOPRO] = "AGUARDANDO_SOPRO",
	[ESTADO_COLETANDO] = "COLETANDO",
	[ESTADO_PROCESSANDO] = "PROCESSANDO",
	[ESTADO_APRESENTANDO_RESULTADO] = "APRESENTANDO_RESULTADO",
	[ESTADO_FINALIZANDO] = "FINALIZANDO",
	[ESTADO_ECONOMIA] = "ECONOMIA",
	[ESTADO_CALIBRANDO_R0] = "CALIBRANDO_R0",
	[ESTADO_ERRO] = "ERRO",
};

/* ---------------------------------------------------------------------------
 * Eventos e temporizador
 * ------------------------------------------------------------------------- */

int bafometro_publicarEvento(const struct EventoBafometro *evento)
{
	int erro = k_msgq_put(&filaEventos, evento, K_NO_WAIT);

	if (erro != 0) {
		atomic_inc(&eventosPerdidos);
		LOG_ERR("Fila cheia: evento %u/%u perdido", evento->tipo, evento->codigo);
	}

	return erro;
}

int bafometro_publicarComando(enum CodigoComando comando)
{
	struct EventoBafometro evento = {
		.tipo = EVENTO_COMANDO,
		.codigo = (uint8_t)comando,
	};

	return bafometro_publicarEvento(&evento);
}

int bafometro_publicarFalha(enum CodigoFalha falha)
{
	struct EventoBafometro evento = {
		.tipo = EVENTO_FALHA,
		.codigo = (uint8_t)falha,
	};

	return bafometro_publicarEvento(&evento);
}

static void aoExpirarTemporizador(struct k_timer *temporizador)
{
	struct EventoBafometro evento = {
		.tipo = EVENTO_TEMPO,
		.codigo = finalidadeArmada,
		.geracao = geracaoArmada,
	};

	ARG_UNUSED(temporizador);
	(void)bafometro_publicarEvento(&evento);
}

K_TIMER_DEFINE(temporizador, aoExpirarTemporizador, NULL);

/* Arma o temporizador; periodoMs = 0 significa um único disparo. */
static void armarTemporizador(enum FinalidadeTempo finalidade, uint32_t duracaoMs,
			      uint32_t periodoMs)
{
	k_timer_stop(&temporizador);
	contexto.geracaoAtual++;
	geracaoArmada = contexto.geracaoAtual;
	finalidadeArmada = (uint8_t)finalidade;
	k_timer_start(&temporizador, K_MSEC(duracaoMs),
		      (periodoMs > 0U) ? K_MSEC(periodoMs) : K_NO_WAIT);
}

/* Para o temporizador e invalida eventos de tempo já enfileirados. */
static void pararTemporizador(void)
{
	k_timer_stop(&temporizador);
	contexto.geracaoAtual++;
}

/* Chamado na interrupção do detector. */
static void aoDetectarSopro(void)
{
	struct EventoBafometro evento = {
		.tipo = EVENTO_SOPRO,
	};

	(void)bafometro_publicarEvento(&evento);
}

/* ---------------------------------------------------------------------------
 * Auxiliares dos estados
 * ------------------------------------------------------------------------- */

static void transitarPara(enum EstadoBafometro estado)
{
	smf_set_state(SMF_CTX(&contexto), &estados[estado]);
}

static void anunciarEstado(enum EstadoBafometro estado)
{
	LOG_INF("Estado: %s", nomesEstados[estado]);
	ble_notificarEstado((uint8_t)estado);
}

static bool ehTempo(const struct ContextoBafometro *ctx)
{
	return ctx->eventoAtual.tipo == EVENTO_TEMPO;
}

static bool ehComando(const struct ContextoBafometro *ctx, enum CodigoComando comando)
{
	return (ctx->eventoAtual.tipo == EVENTO_COMANDO) && (ctx->eventoAtual.codigo == comando);
}

/* Uma falha leva ao ERRO a partir de qualquer estado. Retorna true se tratou o evento. */
static bool tratarFalha(struct ContextoBafometro *ctx)
{
	if (ctx->eventoAtual.tipo != EVENTO_FALHA) {
		return false;
	}

	ctx->codigoErro = (enum CodigoFalha)ctx->eventoAtual.codigo;
	transitarPara(ESTADO_ERRO);
	return true;
}

/* ---------------------------------------------------------------------------
 * Estados
 * ------------------------------------------------------------------------- */

static void entrarInicializando(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_INICIALIZANDO);
	pararTemporizador();
	inibidor_iniciar(); /* nasce ATIVO: partida segura */
	tela_mostrarMensagem("Iniciando");

	ctx->codigoErro = FALHA_NENHUMA;
	if (detector_sopro_iniciar(aoDetectarSopro) != 0) {
		ctx->codigoErro = FALHA_DETECTOR_SOPRO;
	} else if (sensor_alcool_iniciar() != 0) {
		ctx->codigoErro = FALHA_SENSOR_ALCOOL;
	}

	if (ctx->codigoErro != FALHA_NENHUMA) {
		transitarPara(ESTADO_ERRO);
		return;
	}

	ctx->r0Calibrado = (sensor_alcool_carregarR0(&ctx->r0Ohms) == 0);
	if (!ctx->r0Calibrado) {
		ctx->r0Ohms = R0_PADRAO_OHMS;
		LOG_WRN("R0 não calibrado: usando o padrão de %u ohms", ctx->r0Ohms);
	}

	transitarPara(ESTADO_PRE_AQUECENDO);
}

static void entrarPreAquecendo(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_PRE_AQUECENDO);
	sensor_alcool_definirFase(FASE_AQUECIMENTO);
	ctx->acomodando = false;
	ctx->tiquesRestantes = QTD_TIQUES_PRE_AQUECIMENTO;
	tela_mostrarContagem("Preparando sensor, aguarde", ctx->tiquesRestantes);
	armarTemporizador(FINALIDADE_CONTAGEM, PERIODO_CONTAGEM_MS, PERIODO_CONTAGEM_MS);
}

static enum smf_state_result executarPreAquecendo(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	if (tratarFalha(ctx)) {
		return SMF_EVENT_HANDLED;
	}

	if (ehComando(ctx, COMANDO_CALIBRAR)) {
		ctx->calibracaoPendente = true;
		LOG_INF("Calibração agendada para o fim do pré-aquecimento");
		return SMF_EVENT_HANDLED;
	}

	if (!ehTempo(ctx)) {
		return SMF_EVENT_HANDLED;
	}

	if (!ctx->acomodando) {
		ctx->tiquesRestantes--;
		if (ctx->tiquesRestantes > 0U) {
			tela_mostrarContagem("Preparando sensor, aguarde", ctx->tiquesRestantes);
			return SMF_EVENT_HANDLED;
		}

		/* Fim do aquecimento: fase de operação e espera da acomodação do sinal. */
		sensor_alcool_definirFase(FASE_OPERACAO);
		ctx->acomodando = true;
		armarTemporizador(FINALIDADE_ACOMODACAO, TEMPO_ACOMODACAO_FASE_MS, 0U);
		return SMF_EVENT_HANDLED;
	}

	if (ctx->calibracaoPendente) {
		ctx->calibracaoPendente = false;
		transitarPara(ESTADO_CALIBRANDO_R0);
	} else {
		transitarPara(ESTADO_AGUARDANDO_SOPRO);
	}

	return SMF_EVENT_HANDLED;
}

static void entrarAguardandoSopro(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_AGUARDANDO_SOPRO);
	tela_mostrarMensagem("Sopre");

	if (detector_sopro_armar() != 0) {
		ctx->codigoErro = FALHA_DETECTOR_SOPRO;
		transitarPara(ESTADO_ERRO);
		return;
	}

	armarTemporizador(FINALIDADE_ESPERA_SOPRO, TEMPO_ESPERA_SOPRO_MS, 0U);
}

static enum smf_state_result executarAguardandoSopro(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	if (tratarFalha(ctx)) {
		return SMF_EVENT_HANDLED;
	}

	switch (ctx->eventoAtual.tipo) {
	case EVENTO_SOPRO:
		transitarPara(ESTADO_COLETANDO);
		break;
	case EVENTO_TEMPO:
		LOG_INF("Espera pelo sopro esgotada");
		transitarPara(ESTADO_ECONOMIA);
		break;
	case EVENTO_COMANDO:
		if (ctx->eventoAtual.codigo == COMANDO_CALIBRAR) {
			transitarPara(ESTADO_CALIBRANDO_R0);
		} else if (ctx->eventoAtual.codigo == COMANDO_CANCELAR) {
			transitarPara(ESTADO_ECONOMIA);
		} else {
			/* Qualquer outro comando reinicia a espera. */
			armarTemporizador(FINALIDADE_ESPERA_SOPRO, TEMPO_ESPERA_SOPRO_MS, 0U);
		}
		break;
	default:
		break;
	}

	return SMF_EVENT_HANDLED;
}

static void sairAguardandoSopro(void *obj)
{
	ARG_UNUSED(obj);
	(void)detector_sopro_desarmar();
}

static void entrarColetando(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_COLETANDO);
	tela_mostrarMensagem("Continue soprando");
	ctx->tiquesColetados = 0U;
	ctx->ausenciaSoproMs = 0U;
	ctx->perdasNoInicioColeta = atomic_get(&eventosPerdidos);
	armarTemporizador(FINALIDADE_AMOSTRAGEM, PERIODO_AMOSTRAGEM_MS, PERIODO_AMOSTRAGEM_MS);
}

static enum smf_state_result executarColetando(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	if (tratarFalha(ctx) || !ehTempo(ctx)) {
		return SMF_EVENT_HANDLED;
	}

	if (atomic_get(&eventosPerdidos) != ctx->perdasNoInicioColeta) {
		LOG_WRN("Evento perdido durante a coleta: medição invalidada");
		tela_mostrarMensagem("Falha na coleta, sopre novamente");
		transitarPara(ESTADO_AGUARDANDO_SOPRO);
		return SMF_EVENT_HANDLED;
	}

	if (!detector_sopro_presente()) {
		ctx->ausenciaSoproMs += PERIODO_AMOSTRAGEM_MS;
		if (ctx->ausenciaSoproMs > TOLERANCIA_QUEDA_SOPRO_MS) {
			LOG_INF("Sopro interrompido após %u ms de coleta",
				ctx->tiquesColetados * PERIODO_AMOSTRAGEM_MS);
			tela_mostrarMensagem("Sopre por 5 s contínuos");
			transitarPara(ESTADO_AGUARDANDO_SOPRO);
			return SMF_EVENT_HANDLED;
		}
	}

	/* A leitura do ADC e o registro das amostras entram na etapa 3. */
	ctx->tiquesColetados++;
	if (ctx->tiquesColetados >= QTD_TIQUES_COLETA) {
		transitarPara(ESTADO_PROCESSANDO);
	}

	return SMF_EVENT_HANDLED;
}

static void entrarProcessando(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_PROCESSANDO);
	pararTemporizador();
	ctx->sequenciaMedicao++;

	/*
	 * O cálculo está em dominio/medicao.c, mas as amostras só chegam com a
	 * leitura do ADC, na etapa 3. Até lá o resultado é inválido e a decisão
	 * mantém o estado do inibidor, que nunca chega a ser liberado. Na etapa 3,
	 * resultado inválido ou de qualidade insuficiente volta para AGUARDANDO_SOPRO.
	 */
	ctx->resultado = (struct ResultadoMedicao){
		.valido = false,
		.qualidade = QUALIDADE_DESCARTAR,
	};
	ctx->resultado.calibrado = ctx->r0Calibrado;
	ctx->resultado.sequencia = ctx->sequenciaMedicao;
	ctx->resultado.inibir = decisao_deveInibir(
		inibidor_estaAtivo(),
		ctx->resultado.valido && (ctx->resultado.qualidade != QUALIDADE_DESCARTAR),
		ctx->resultado.concentracaoMilesimosMgL);

	transitarPara(ESTADO_APRESENTANDO_RESULTADO);
}

static void entrarApresentandoResultado(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_APRESENTANDO_RESULTADO);
	tela_mostrarResultado(&ctx->resultado);
	ble_notificarResultado(&ctx->resultado);

	/* Atua conforme a decisão tomada em PROCESSANDO; a saída é mantida. */
	if (ctx->resultado.inibir) {
		inibidor_ativar();
	} else {
		inibidor_liberar();
	}

	armarTemporizador(FINALIDADE_APRESENTACAO, TEMPO_APRESENTACAO_MS, 0U);
}

static enum smf_state_result executarApresentandoResultado(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	if (tratarFalha(ctx)) {
		return SMF_EVENT_HANDLED;
	}

	if (ehTempo(ctx) || ehComando(ctx, COMANDO_REPETIR)) {
		transitarPara(ESTADO_FINALIZANDO);
	}

	return SMF_EVENT_HANDLED;
}

static void entrarFinalizando(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_FINALIZANDO);
	tela_mostrarMensagem("Aguarde a recuperação do sensor");
	ctx->tiquesColetados = 0U;
	ctx->ausenciaSoproMs = 0U;
	armarTemporizador(FINALIDADE_RECUPERACAO, TEMPO_MIN_ENTRE_TESTES_MS, 0U);
}

static enum smf_state_result executarFinalizando(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	if (tratarFalha(ctx)) {
		return SMF_EVENT_HANDLED;
	}

	if (ehTempo(ctx)) {
		transitarPara(ESTADO_AGUARDANDO_SOPRO);
	}

	return SMF_EVENT_HANDLED;
}

static void entrarEconomia(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_ECONOMIA);
	pararTemporizador();
	sensor_alcool_definirFase(FASE_REPOUSO);
	tela_apagar();
	ble_definirAnuncioLento(true);

	if (detector_sopro_armar() != 0) {
		ctx->codigoErro = FALHA_DETECTOR_SOPRO;
		transitarPara(ESTADO_ERRO);
	}
}

static enum smf_state_result executarEconomia(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	if (tratarFalha(ctx)) {
		return SMF_EVENT_HANDLED;
	}

	if (ctx->eventoAtual.tipo == EVENTO_SOPRO) {
		transitarPara(ESTADO_PRE_AQUECENDO);
	} else if (ehComando(ctx, COMANDO_CALIBRAR)) {
		ctx->calibracaoPendente = true;
		transitarPara(ESTADO_PRE_AQUECENDO);
	} else if (ehComando(ctx, COMANDO_INICIAR) || ehComando(ctx, COMANDO_REPETIR)) {
		transitarPara(ESTADO_PRE_AQUECENDO);
	}

	return SMF_EVENT_HANDLED;
}

static void sairEconomia(void *obj)
{
	ARG_UNUSED(obj);
	(void)detector_sopro_desarmar();
	ble_definirAnuncioLento(false);
}

static void entrarCalibrandoR0(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_CALIBRANDO_R0);
	tela_mostrarMensagem("Calibrando: mantenha o ar limpo");
	ctx->tiquesRestantes = QTD_AMOSTRAS_CALIBRACAO;
	armarTemporizador(FINALIDADE_CALIBRACAO, INTERVALO_CALIBRACAO_MS, INTERVALO_CALIBRACAO_MS);
}

static enum smf_state_result executarCalibrandoR0(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	if (tratarFalha(ctx) || !ehTempo(ctx)) {
		return SMF_EVENT_HANDLED;
	}

	/* A leitura das amostras entra na etapa 3. */
	ctx->tiquesRestantes--;
	if (ctx->tiquesRestantes > 0U) {
		return SMF_EVENT_HANDLED;
	}

	/* O cálculo (dominio/calibracao.c) e a gravação do R0 entram na etapa 6. */
	LOG_WRN("Calibração ainda não implementada: R0 mantido em %u ohms", ctx->r0Ohms);
	tela_mostrarMensagem("Calibração indisponível");
	transitarPara(ESTADO_AGUARDANDO_SOPRO);
	return SMF_EVENT_HANDLED;
}

static void entrarErro(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	anunciarEstado(ESTADO_ERRO);
	pararTemporizador();
	inibidor_ativar();
	sensor_alcool_definirFase(FASE_REPOUSO);
	LOG_ERR("Falha %d", ctx->codigoErro);
	tela_mostrarMensagem("Erro: reinicie o aparelho");
}

static enum smf_state_result executarErro(void *obj)
{
	struct ContextoBafometro *ctx = obj;

	if (ehComando(ctx, COMANDO_REINICIAR)) {
		transitarPara(ESTADO_INICIALIZANDO);
	}

	return SMF_EVENT_HANDLED;
}

static const struct smf_state estados[QTD_ESTADOS] = {
	[ESTADO_INICIALIZANDO] = SMF_CREATE_STATE(entrarInicializando, NULL, NULL, NULL, NULL),
	[ESTADO_PRE_AQUECENDO] =
		SMF_CREATE_STATE(entrarPreAquecendo, executarPreAquecendo, NULL, NULL, NULL),
	[ESTADO_AGUARDANDO_SOPRO] = SMF_CREATE_STATE(entrarAguardandoSopro, executarAguardandoSopro,
						     sairAguardandoSopro, NULL, NULL),
	[ESTADO_COLETANDO] = SMF_CREATE_STATE(entrarColetando, executarColetando, NULL, NULL, NULL),
	[ESTADO_PROCESSANDO] = SMF_CREATE_STATE(entrarProcessando, NULL, NULL, NULL, NULL),
	[ESTADO_APRESENTANDO_RESULTADO] = SMF_CREATE_STATE(
		entrarApresentandoResultado, executarApresentandoResultado, NULL, NULL, NULL),
	[ESTADO_FINALIZANDO] =
		SMF_CREATE_STATE(entrarFinalizando, executarFinalizando, NULL, NULL, NULL),
	[ESTADO_ECONOMIA] =
		SMF_CREATE_STATE(entrarEconomia, executarEconomia, sairEconomia, NULL, NULL),
	[ESTADO_CALIBRANDO_R0] =
		SMF_CREATE_STATE(entrarCalibrandoR0, executarCalibrandoR0, NULL, NULL, NULL),
	[ESTADO_ERRO] = SMF_CREATE_STATE(entrarErro, executarErro, NULL, NULL, NULL),
};

/* ---------------------------------------------------------------------------
 * Interface pública
 * ------------------------------------------------------------------------- */

int bafometro_iniciar(void)
{
	/* Reinício limpo: nada do ciclo anterior pode chegar à nova máquina. */
	k_timer_stop(&temporizador);
	(void)detector_sopro_desarmar();
	k_msgq_purge(&filaEventos);
	atomic_clear(&eventosPerdidos);
	atomic_clear(&eventosDescartados);
	contexto = (struct ContextoBafometro){0};

	smf_set_initial(SMF_CTX(&contexto), &estados[ESTADO_INICIALIZANDO]);
	return 0;
}

int bafometro_processarProximoEvento(k_timeout_t espera)
{
	struct EventoBafometro evento;
	int erro = k_msgq_get(&filaEventos, &evento, espera);

	if (erro != 0) {
		return erro;
	}

	if ((evento.tipo == EVENTO_TEMPO) && (evento.geracao != contexto.geracaoAtual)) {
		atomic_inc(&eventosDescartados);
		LOG_DBG("Tempo obsoleto descartado (geração %u, atual %u)", evento.geracao,
			contexto.geracaoAtual);
		return 0;
	}

	contexto.eventoAtual = evento;
	(void)smf_run_state(SMF_CTX(&contexto));
	return 0;
}

enum EstadoBafometro bafometro_estadoAtual(void)
{
	const struct smf_state *atual = contexto.smf.current;

	if (atual == NULL) {
		return ESTADO_INICIALIZANDO;
	}

	return (enum EstadoBafometro)(atual - &estados[0]);
}

const char *bafometro_nomeEstado(enum EstadoBafometro estado)
{
	return (estado < QTD_ESTADOS) ? nomesEstados[estado] : "?";
}

uint32_t bafometro_eventosPerdidos(void)
{
	return (uint32_t)atomic_get(&eventosPerdidos);
}

uint32_t bafometro_eventosDescartados(void)
{
	return (uint32_t)atomic_get(&eventosDescartados);
}

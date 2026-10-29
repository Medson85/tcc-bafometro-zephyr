/*
 * Parâmetros do produto: fonte única dos valores usados pela aplicação.
 * A origem de cada valor está em docs/arquitetura/arquitetura-firmware.md.
 */

#ifndef BAFOMETRO_CONFIG_PRODUTO_H
#define BAFOMETRO_CONFIG_PRODUTO_H

#if defined(CONFIG_BAFOMETRO_TEMPOS_REDUZIDOS)
#define FATOR_REDUCAO_TEMPO 10U
#else
#define FATOR_REDUCAO_TEMPO 1U
#endif

/*
 * Esperas do fluxo, divididas pelo fator de redução.
 * PROVISÓRIO: valores a levantar em bancada.
 */
#define TEMPO_PRE_AQUECIMENTO_MS  (10000U / FATOR_REDUCAO_TEMPO) /* requisito; origem a citar */
#define TEMPO_ESPERA_SOPRO_MS     (20000U / FATOR_REDUCAO_TEMPO) /* requisito; origem a citar */
#define TEMPO_APRESENTACAO_MS     (10000U / FATOR_REDUCAO_TEMPO) /* proposta */
#define TEMPO_MIN_ENTRE_TESTES_MS (30000U / FATOR_REDUCAO_TEMPO) /* PROVISÓRIO */
#define TEMPO_ACOMODACAO_FASE_MS  (500U / FATOR_REDUCAO_TEMPO)   /* PROVISÓRIO */

/* Medição (não são reduzidos) */
#define TEMPO_SOPRO_MINIMO_MS     5000U
#define PERIODO_AMOSTRAGEM_MS     50U   /* proposta: 20 Hz */
#define TOLERANCIA_QUEDA_SOPRO_MS 150U  /* proposta: queda acumulada tolerada durante a coleta */
#define PERIODO_CONTAGEM_MS       1000U /* contagem regressiva na tela */

/* Calibração do R0 */
#define QTD_AMOSTRAS_CALIBRACAO 60U
#define INTERVALO_CALIBRACAO_MS 100U
#define R0_PADRAO_OHMS          24000U /* valor do protótipo */

/* Fila de eventos */
#define QTD_POSICOES_FILA_EVENTOS 8

/* Derivados */
#define QTD_TIQUES_PRE_AQUECIMENTO (TEMPO_PRE_AQUECIMENTO_MS / PERIODO_CONTAGEM_MS)
#define QTD_TIQUES_COLETA          (TEMPO_SOPRO_MINIMO_MS / PERIODO_AMOSTRAGEM_MS)

#endif /* BAFOMETRO_CONFIG_PRODUTO_H */

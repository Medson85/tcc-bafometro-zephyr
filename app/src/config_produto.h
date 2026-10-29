/*
 * Parâmetros do produto: fonte única dos valores usados pela aplicação.
 * A origem de cada valor está em docs/arquitetura/arquitetura-firmware.md.
 * Só macros: este arquivo é incluído também pelo domínio, que não depende do Zephyr.
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
 * Pré-aquecimento: o datasheet do MQ303B (v1.4) pede 2,0 V por 5 a 10 s antes da detecção.
 */
#define TEMPO_PRE_AQUECIMENTO_MS  (10000U / FATOR_REDUCAO_TEMPO) /* datasheet: 5 a 10 s */
#define TEMPO_ESPERA_SOPRO_MS     (20000U / FATOR_REDUCAO_TEMPO) /* requisito; origem a citar */
#define TEMPO_APRESENTACAO_MS     (10000U / FATOR_REDUCAO_TEMPO) /* proposta */
#define TEMPO_MIN_ENTRE_TESTES_MS (30000U / FATOR_REDUCAO_TEMPO) /* PROVISÓRIO */
#define TEMPO_ACOMODACAO_FASE_MS  (500U / FATOR_REDUCAO_TEMPO)   /* PROVISÓRIO */

/* Medição (não são reduzidos) */
#define TEMPO_SOPRO_MINIMO_MS     5000U
#define PERIODO_AMOSTRAGEM_MS     50U   /* proposta: 20 Hz */
#define TOLERANCIA_QUEDA_SOPRO_MS 150U  /* proposta: queda acumulada tolerada durante a coleta */
#define PERIODO_CONTAGEM_MS       1000U /* contagem regressiva na tela */
#define TEMPO_TRANSIENTE_MS       2000U /* proposta: início da coleta descartado no cálculo */
#define JANELA_MEDIA_MOVEL        8U    /* proposta: faixa de 8 a 16 */
#define QTD_MAXIMA_AMOSTRAS       110U  /* 5 s a 50 ms, com margem */

/*
 * Circuito de medição do MQ303B: VCC - Rs (sensor) - nó do ADC - RL - GND.
 * O ADC mede a tensão sobre RL (VRL), como no datasheet e no texto do TCC.
 */
#define TENSAO_ALIMENTACAO_MV   3300.0f
#define RESISTENCIA_CARGA_OHMS  3300.0f
#define MARGEM_TENSAO_VALIDA_MV 1.0f /* protótipo: rejeita leituras a 1 mV dos trilhos */

/* Conversão: mg/L = ppm / 500 (protótipo; superestima de 6 a 9 % frente ao gás ideal) */
#define FATOR_PPM_PARA_MGL 500.0f

/* Qualidade: coeficiente de variação do trecho estável, em % (a revalidar) */
#define QUALIDADE_CV_EXCELENTE_PCT 2.0f
#define QUALIDADE_CV_BOA_PCT       5.0f
#define QUALIDADE_CV_ACEITAVEL_PCT 10.0f

/* Calibração do R0 */
#define QTD_AMOSTRAS_CALIBRACAO             60U
#define INTERVALO_CALIBRACAO_MS             100U
#define PORCENTAGEM_MINIMA_AMOSTRAS_VALIDAS 50U    /* protótipo */
#define R0_PADRAO_OHMS                      24000U /* valor do protótipo */

/*
 * Inibição, em milésimos de mg/L. PROVISÓRIO: definir pelo método da banda de guarda.
 * A curva começa em 50 ppm (0,100 mg/L): ar limpo é medido como 0,100 mg/L, então
 * só há liberação se LIMITE - HISTERESE for maior que 100.
 */
#define LIMITE_INIBICAO_MILESIMOS_MGL     150U
#define HISTERESE_LIBERACAO_MILESIMOS_MGL 20U

/* Fila de eventos */
#define QTD_POSICOES_FILA_EVENTOS 8

/* Derivados */
#define QTD_TIQUES_PRE_AQUECIMENTO (TEMPO_PRE_AQUECIMENTO_MS / PERIODO_CONTAGEM_MS)
#define QTD_TIQUES_COLETA          (TEMPO_SOPRO_MINIMO_MS / PERIODO_AMOSTRAGEM_MS)
#define QTD_AMOSTRAS_TRANSIENTE    (TEMPO_TRANSIENTE_MS / PERIODO_AMOSTRAGEM_MS)

#endif /* BAFOMETRO_CONFIG_PRODUTO_H */

/*
 * Cálculo de uma medição a partir das amostras brutas da coleta:
 * filtro -> trecho estável -> estatística -> Rs -> Rs/R0 -> ppm -> mg/L -> qualidade.
 */

#ifndef BAFOMETRO_MEDICAO_H
#define BAFOMETRO_MEDICAO_H

#include <stdbool.h>
#include <stdint.h>

#include "config_produto.h"
#include "dominio/resultado.h"

/*
 * Primeira saída do filtro calculada só com amostras posteriores ao transiente:
 * descarta o transiente e as saídas cuja janela ainda contém amostras dele.
 */
#define MEDICAO_INICIO_TRECHO_ESTAVEL (QTD_AMOSTRAS_TRANSIENTE + JANELA_MEDIA_MOVEL - 1U)

/*
 * Aplica o filtro a todas as amostras e calcula o resultado sobre as saídas a partir
 * de MEDICAO_INICIO_TRECHO_ESTAVEL. Zera o resultado antes de preencher.
 * Retorna resultado->valido: false se faltarem amostras, se a tensão média estiver
 * fora dos trilhos ou se o R0 for inválido.
 */
bool medicao_calcular(const int32_t *amostrasMv, uint16_t quantidade, float r0Ohms,
		      struct ResultadoMedicao *resultado);

#endif /* BAFOMETRO_MEDICAO_H */

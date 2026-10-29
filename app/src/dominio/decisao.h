/*
 * Decisão de inibição com histerese. Só uma medição aceita muda o estado:
 * ativa com concentração >= LIMITE_INIBICAO_MILESIMOS_MGL e libera somente
 * abaixo de LIMITE - HISTERESE; entre os dois, o estado atual é mantido.
 */

#ifndef BAFOMETRO_DECISAO_H
#define BAFOMETRO_DECISAO_H

#include <stdbool.h>
#include <stdint.h>

bool decisao_deveInibir(bool inibindoAgora, bool medicaoAceita, uint16_t concentracaoMilesimosMgL);

#endif /* BAFOMETRO_DECISAO_H */

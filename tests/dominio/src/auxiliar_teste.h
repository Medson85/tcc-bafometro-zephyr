/*
 * Auxiliares dos testes de domínio.
 */

#ifndef BAFOMETRO_AUXILIAR_TESTE_H
#define BAFOMETRO_AUXILIAR_TESTE_H

#include "config_produto.h"

/* Tensão sobre RL para uma resistência do sensor: VRL = VCC * RL / (RL + Rs). */
static inline float tensaoSobreRlMv(float resistenciaRsOhms)
{
	return TENSAO_ALIMENTACAO_MV * RESISTENCIA_CARGA_OHMS /
	       (RESISTENCIA_CARGA_OHMS + resistenciaRsOhms);
}

#endif /* BAFOMETRO_AUXILIAR_TESTE_H */

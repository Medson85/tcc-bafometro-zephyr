/*
 * Resultado de uma medição. Os campos de cálculo (mV, Rs, Rs/R0 e ppm)
 * entram na etapa 2, junto com o domínio portado do protótipo.
 */

#ifndef BAFOMETRO_RESULTADO_H
#define BAFOMETRO_RESULTADO_H

#include <stdbool.h>
#include <stdint.h>

struct ResultadoMedicao {
	bool valido;
	bool calibrado;
	bool inibir;
	uint8_t qualidade;
	uint16_t sequencia;
	uint16_t concentracaoMilesimosMgL; /* mg/L × 1000, como no pacote BLE */
};

#endif /* BAFOMETRO_RESULTADO_H */

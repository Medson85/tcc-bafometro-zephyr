/*
 * Bafômetro: ponto de entrada. A única thread da aplicação processa a fila
 * de eventos e executa a máquina de estados.
 */

#include <zephyr/kernel.h>

#include "aplicacao/bafometro.h"

int main(void)
{
	(void)bafometro_iniciar();

	while (true) {
		(void)bafometro_processarProximoEvento(K_FOREVER);
	}

	return 0;
}

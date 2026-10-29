/*
 * Detector digital de sopro, lido pelo GPIO definido em zephyr,user
 * (propriedade detector-sopro-gpios do devicetree).
 */

#ifndef BAFOMETRO_DETECTOR_SOPRO_H
#define BAFOMETRO_DETECTOR_SOPRO_H

#include <stdbool.h>

/* Chamado na interrupção, a cada início de sopro (transição para ativo). */
typedef void (*AoDetectarSopro)(void);

int detector_sopro_iniciar(AoDetectarSopro aoDetectar);
int detector_sopro_armar(void);
int detector_sopro_desarmar(void);
bool detector_sopro_presente(void);

#endif /* BAFOMETRO_DETECTOR_SOPRO_H */

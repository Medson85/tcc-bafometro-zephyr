/*
 * Saída de inibição: mantida e fail-active. Nasce ativa e só é liberada
 * por decisão explícita da aplicação.
 */

#ifndef BAFOMETRO_INIBIDOR_H
#define BAFOMETRO_INIBIDOR_H

#include <stdbool.h>

/* Configura o pino já ativo. Retorna 0 ou um erro negativo. */
int inibidor_iniciar(void);
void inibidor_ativar(void);
void inibidor_liberar(void);
bool inibidor_estaAtivo(void);

#endif /* BAFOMETRO_INIBIDOR_H */

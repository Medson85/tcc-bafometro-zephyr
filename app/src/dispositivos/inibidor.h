/*
 * Saída de inibição: mantida e fail-active. Nasce ativa e só é liberada
 * por decisão explícita da aplicação.
 */

#ifndef BAFOMETRO_INIBIDOR_H
#define BAFOMETRO_INIBIDOR_H

#include <stdbool.h>

/*
 * Configuram e comandam o pino. Retornam 0 ou um erro negativo. Em erro, o estado
 * físico é desconhecido e o estado lógico fica ativo: a aplicação deve ir para ERRO.
 */
int inibidor_iniciar(void); /* configura o pino já ativo */
int inibidor_ativar(void);
int inibidor_liberar(void);
bool inibidor_estaAtivo(void);

#endif /* BAFOMETRO_INIBIDOR_H */

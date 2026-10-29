#include "config_produto.h"
#include "dominio/decisao.h"

_Static_assert(HISTERESE_LIBERACAO_MILESIMOS_MGL < LIMITE_INIBICAO_MILESIMOS_MGL,
	       "a histerese deve ser menor que o limite de inibição");

bool decisao_deveInibir(bool inibindoAgora, bool medicaoAceita, uint16_t concentracaoMilesimosMgL)
{
	if (!medicaoAceita) {
		return inibindoAgora;
	}

	if (concentracaoMilesimosMgL >= LIMITE_INIBICAO_MILESIMOS_MGL) {
		return true;
	}

	if (concentracaoMilesimosMgL <
	    (LIMITE_INIBICAO_MILESIMOS_MGL - HISTERESE_LIBERACAO_MILESIMOS_MGL)) {
		return false;
	}

	return inibindoAgora;
}

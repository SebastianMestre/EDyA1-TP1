#include "expresion.h"

void expresion_limpiar(Expresion* expresion) {
	if (expresion) {
		expresion_limpiar(expresion->sub[0]);
		expresion_limpiar(expresion->sub[1]);
		free(expresion);
	}
}

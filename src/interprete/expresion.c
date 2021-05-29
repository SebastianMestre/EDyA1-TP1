#include "expresion.h"

#include <stdlib.h>

static Expresion* expresion_reservar(Expresion datos) {
	Expresion* resultado = malloc(sizeof(Expresion));
	*resultado = datos;
	return resultado;
}

Expresion* expresion_numero(int valor) {
	return expresion_reservar((Expresion){
		.tag = X_NUMERO,
		.valor = valor,
	});
}

Expresion* expresion_alias(char const* alias, int alias_n) {
	return expresion_reservar((Expresion){
		.tag = X_ALIAS,
		.alias = alias,
		.valor = alias_n,
	});
}

Expresion* expresion_operacion(EntradaTablaOps* op, Expresion* sub0, Expresion* sub1) {
	return expresion_reservar((Expresion){
		.tag = X_OPERACION,
		.op = op,
		.sub = {sub0, sub1},
	});
}

void expresion_limpiar(Expresion* expresion) {
	if (!expresion)
		return;
	expresion_limpiar(expresion->sub[0]);
	expresion_limpiar(expresion->sub[1]);
	free(expresion);
}

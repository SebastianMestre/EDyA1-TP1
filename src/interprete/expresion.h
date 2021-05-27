#ifndef EXPRESION_H
#define EXPRESION_H

#include "..\funcion_evaluacion.h"

typedef struct Expresion Expresion;

// este enum, en el contexto de una expresion, nos permite distinguir si la
// expresion es un alias, un numeros o una operacion
typedef enum {
	X_OPERACION,
	X_NUMERO,
	X_ALIAS,
} ExpressionTag;

struct Expresion {
	ExpressionTag tag;
	int valor; // para guardar los valores numericos, o la longitud de un alias, dependiendo del tag
	char const* alias; // para guardar el texto de un alias
	Expresion* sub[2]; // para guardar las sub-expresiones de una operacion
	FuncionEvaluacion eval; // para guardar el tipo de operacion
};

// no hace free de alias
void expresion_limpiar(Expresion* expresion);

#endif // EXPRESION_H

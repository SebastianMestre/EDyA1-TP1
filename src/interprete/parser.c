#include "parser.h"

#include "../tabla_ops.h"
#include "expresion.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define CANT_STRINGS_FIJOS 4
static int const largo_strings_fijos[CANT_STRINGS_FIJOS] = { 5, 6, 7, 8 };
static char const* const strings_fijos[CANT_STRINGS_FIJOS] = { "salir", "cargar", "evaluar", "imprimir" };
static TokenTag const token_strings_fijos[CANT_STRINGS_FIJOS] = { T_SALIR, T_CARGAR, T_EVALUAR, T_IMPRIMIR };

Tokenizado tokenizar(char const* str, TablaOps* tabla_ops) {
	// TODO: reconocer operadores

	while (isspace(*str))
		str += 1;

	if (*str == '\0')
		return (Tokenizado){str, (Token){T_FIN}};

	// NICETOHAVE chequear si hay un operador que arranca con =
	if (*str == '=')
		return (Tokenizado){str+1, (Token){T_IGUAL}};

	if (isalpha(str[0])) {
		// reconozco un nombre
		int largo = 1;
		while (isalnum(str[largo]))
			largo += 1;

		for (int i = 0; i < CANT_STRINGS_FIJOS; ++i)
			if (largo_strings_fijos[i] == largo &&
			    memcmp(str, strings_fijos[i], largo) == 0)
				return (Tokenizado){str+largo, (Token){token_strings_fijos[i]}};

		return (Tokenizado){str+largo, (Token){T_NOMBRE, str, largo}};
	}

	if (isdigit(str[0])) {
		// reconozco un numero
		int valor = str[0] - '0';
		int largo = 1;
		while (isdigit(str[largo])) {
			valor = (valor * 10) + (str[largo] - '0');
			largo += 1;
		}

		return (Tokenizado){str+largo, (Token){T_NUMERO, NULL, valor}};
	}
}

static Parseado invalido(char const* str) {
	return (Parseado){str, (Sentencia){S_INVALIDO}};
}

typedef struct EntradaPilaDeExpresiones EntradaPilaDeExpresiones;
struct EntradaPilaDeExpresiones {
	EntradaPilaDeExpresiones* sig;
	Expresion* expresion;
};

typedef struct PilaDeExpresiones {
	EntradaPilaDeExpresiones* entradas;
} PilaDeExpresiones;

void pila_de_expresiones_push(PilaDeExpresiones* pila, Expresion* expresion) {
	EntradaPilaDeExpresiones* entrada = malloc(sizeof(*entrada));
	*entrada = (EntradaPilaDeExpresiones){
		.sig = pila->entradas,
		.expresion = expresion
	};
	pila->entradas = entrada;
}

Expresion* pila_de_expresiones_pop(PilaDeExpresiones* pila) {
	if (pila->entradas == NULL) return NULL;
	Expresion* result = pila->entradas->expresion;
	EntradaPilaDeExpresiones* entrada = pila->entradas;
	pila->entradas = entrada->sig;
	free(entrada);
	return result;
}

Parseado parsear(char const* str, TablaOps* tabla_ops) {
	Tokenizado tokenizado = tokenizar(str, tabla_ops);
	str = tokenizado.resto;

	switch (tokenizado.token.tag) {
	case T_SALIR:
		return (Parseado){str, (Sentencia){S_SALIR}};
		break;

	case T_EVALUAR:
		break;
		tokenizado = tokenizar(str, tabla_ops);
		str = tokenizado.resto;
		if (tokenizado.token.tag != T_NOMBRE)
			return invalido(str);
		return (Parseado){str, (Sentencia){S_EVALUAR, tokenizado.token.inicio, tokenizado.token.valor}};
		break;

	case T_IMPRIMIR:
		tokenizado = tokenizar(str, tabla_ops);
		str = tokenizado.resto;
		if (tokenizado.token.tag != T_NOMBRE)
			return invalido(str);
		return (Parseado){str, (Sentencia){S_IMPRIMIR, tokenizado.token.inicio, tokenizado.token.valor}};
		break;

	case T_NOMBRE: {
		char const* alias = tokenizado.token.inicio;
		int alias_n = tokenizado.token.valor;

		tokenizado = tokenizar(str, tabla_ops);
		str = tokenizado.resto;
		if (tokenizado.token.tag != T_IGUAL)
			return invalido(str);

		tokenizado = tokenizar(str, tabla_ops);
		str = tokenizado.resto;
		if (tokenizado.token.tag != T_CARGAR)
			return invalido(str);

		// Ahora parseo la expresion infija dos veces:
		// La primera vez para saber cuantos tokens hay, y asi poder reservar
		// la memoria necesaria para guardarlos
		// Y la segunda para guardarlos en la memoria que reserve

		// {} inicializa con 0s, lo cual es el estado inicial correcto
		PilaDeExpresiones p = {};
		// parseo y, mientras, voy validando
		while (1) {
			tokenizado = tokenizar(str, tabla_ops);
			str = tokenizado.resto;
			Token token = tokenizado.token;

			if (token.tag == T_FIN)
				break;

			switch (token.tag) {
			case T_NUMERO: {
				Expresion* exp = malloc(sizeof(*exp));
				*exp = (Expresion){X_NUMERO, token.valor};
				pila_de_expresiones_push(&p, exp);
				} break;
			case T_NOMBRE: {
				Expresion* exp = malloc(sizeof(*exp));
				*exp = (Expresion){X_ALIAS, token.valor, token.inicio};
				pila_de_expresiones_push(&p, exp);
				} break;
			case T_OPERADOR: {
				Expresion* exp = malloc(sizeof(*exp));
				Expresion* arg1 = pila_de_expresiones_pop(&p);
				Expresion* arg2 = token.op->aridad == 2 ? pila_de_expresiones_pop(&p) : NULL;
				*exp = (Expresion){
					X_OPERACION,
					0, NULL,
					{arg1, arg2},
					token.op->eval
				};
				pila_de_expresiones_push(&p, exp);
				} break;
			default:
				// TODO limpiar
				return invalido(str);
			}
		}

		Expresion* expresion = pila_de_expresiones_pop(&p);

		return (Parseado){str, (Sentencia){S_CARGA, alias, alias_n, expresion}};
		} break;

	default:
		return invalido(str);
	}
}

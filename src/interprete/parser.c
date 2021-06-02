#include "parser.h"

#include "../tabla_ops.h"
#include "expresion.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// En el contexto de un Token, sirve para interpretar la informacion de este
typedef enum {
	T_NOMBRE,   // una cadena alfanumrica
	T_OPERADOR, // un operador
	T_NUMERO,   // un numero (secuencia de digitos)
	T_IMPRIMIR, // 'imprimir'
	T_EVALUAR,  // 'evaluar'
	T_CARGAR,   // 'cargar'
	T_SALIR,    // 'salir'
	T_IGUAL,    // '='
	T_FIN,      // el final del string
	T_INVALIDO, // un error
} TokenTag;

// Representa una secuencia de caracteres.
// Consolidarla en una sola entidad permite expresar algunas partes del codigo
// mas claramente.
typedef struct Token {
	TokenTag tag;
	char const* inicio;  // apunta al texto, en caso de ser un nombre
	int valor;           // la longitud del nombre, o el valor de un numero
	EntradaTablaOps* op; // la entrada en la tabla de operadores, de ser un operador
} Token;

typedef struct Tokenizado {
	char const* resto;
	Token token;
} Tokenizado;

#define CANT_STRINGS_FIJOS 4
static int const largo_strings_fijos[CANT_STRINGS_FIJOS] = { 5, 6, 7, 8 };
static char const* const strings_fijos[CANT_STRINGS_FIJOS] = { "salir", "cargar", "evaluar", "imprimir" };
static TokenTag const token_strings_fijos[CANT_STRINGS_FIJOS] = { T_SALIR, T_CARGAR, T_EVALUAR, T_IMPRIMIR };

// Analiza el pricipio del string, y extrae una pieza, dandole sentido.
// Luego, devuelve una representacion de esa pieza, y un puntero a donde esa
// pieza termina, y empieza el resto del string.
//
// # uso de memoria
// argumentos: No limpia nada
// resultado: Nada se debe limpiar
static Tokenizado tokenizar(char const* str, TablaOps* tabla_ops) {
	// NICETOHAVE soportar operadores alfanumericos

	while (isspace(*str))
		str += 1;

	if (*str == '\0')
		return (Tokenizado){str, (Token){T_FIN}};

	// NICETOHAVE soportar operadores que empiezan con =
	if (*str == '=')
		return (Tokenizado){str+1, (Token){T_IGUAL}};

	// reconozco un nombre
	if (isalpha(str[0])) {
		int largo = 1;
		while (isalnum(str[largo]))
			largo += 1;

		for (int i = 0; i < CANT_STRINGS_FIJOS; ++i)
			if (largo_strings_fijos[i] == largo &&
			    memcmp(str, strings_fijos[i], largo) == 0)
				return (Tokenizado){str+largo, (Token){token_strings_fijos[i]}};

		return (Tokenizado){str+largo, (Token){T_NOMBRE, str, largo}};
	}

	// reconozco un numero
	if (isdigit(str[0])) {
		int valor = str[0] - '0';
		int largo = 1;
		while (isdigit(str[largo])) {
			valor = (valor * 10) + (str[largo] - '0');
			largo += 1;
		}

		return (Tokenizado){str+largo, (Token){T_NUMERO, NULL, valor}};
	}

	{
		// reconozco un operador (el mas largo que matchee)
		EntradaTablaOps* op_que_matchea = NULL;
		int largo_de_op_que_matchea = 0;
		for (EntradaTablaOps* it = tabla_ops->entradas; it; it = it->sig) {
			int largo_de_op = strlen(it->simbolo);

			if (largo_de_op < largo_de_op_que_matchea)
				continue;

			int matchea = 1;
			for (int i = 0; i < largo_de_op; ++i) {
				if (str[i] != it->simbolo[i]) {
					matchea = 0;
					break;
				}
			}

			if (matchea) {
				op_que_matchea = it;
				largo_de_op_que_matchea = largo_de_op;
			}
		}

		if (op_que_matchea != NULL)
			return (Tokenizado){str+largo_de_op_que_matchea, (Token){T_OPERADOR, NULL, 0, op_que_matchea}};
	}

	return (Tokenizado){str, (Token){T_INVALIDO}};
}

static Parseado invalido(char const* str, ErrorTag error) {
	return (Parseado){str, (Sentencia){.tag = S_INVALIDO}, error};
}

typedef struct EntradaPilaDeExpresiones EntradaPilaDeExpresiones;
struct EntradaPilaDeExpresiones {
	EntradaPilaDeExpresiones* sig;
	Expresion* expresion;
};

typedef struct PilaDeExpresiones {
	EntradaPilaDeExpresiones* entradas;
} PilaDeExpresiones;

static void pila_de_expresiones_push(PilaDeExpresiones* pila, Expresion* expresion) {
	EntradaPilaDeExpresiones* entrada = malloc(sizeof(*entrada));
	*entrada = (EntradaPilaDeExpresiones){
		.sig = pila->entradas,
		.expresion = expresion
	};
	pila->entradas = entrada;
}

static Expresion* pila_de_expresiones_pop(PilaDeExpresiones* pila) {
	if (pila->entradas == NULL) return NULL;
	Expresion* result = pila->entradas->expresion;
	EntradaPilaDeExpresiones* entrada = pila->entradas;
	pila->entradas = entrada->sig;
	free(entrada);
	return result;
}

static Expresion* pila_de_expresiones_top(PilaDeExpresiones* pila) {
	if (pila->entradas == NULL)
		return NULL;
	return pila->entradas->expresion;
}

static void pila_de_expresiones_limpiar_datos(PilaDeExpresiones* pila) {
	EntradaPilaDeExpresiones* it = pila->entradas;
	while (it) {
		EntradaPilaDeExpresiones* sig = it->sig;
		expresion_limpiar(it->expresion);
		free(it);
		it = sig;
	}
}

Parseado parsear(char const* str, TablaOps* tabla_ops) {
	Tokenizado tokenizado = tokenizar(str, tabla_ops);
	str = tokenizado.resto;

	switch (tokenizado.token.tag) {
	case T_SALIR:
		return (Parseado){str, (Sentencia){S_SALIR}};
		break;

	case T_EVALUAR:
		tokenizado = tokenizar(str, tabla_ops);
		str = tokenizado.resto;
		if (tokenizado.token.tag != T_NOMBRE)
			return invalido(str, E_ALIAS);
		return (Parseado){str, (Sentencia){S_EVALUAR, tokenizado.token.inicio, tokenizado.token.valor}};
		break;

	case T_IMPRIMIR:
		tokenizado = tokenizar(str, tabla_ops);
		str = tokenizado.resto;
		if (tokenizado.token.tag != T_NOMBRE)
			return invalido(str, E_ALIAS);
		return (Parseado){str, (Sentencia){S_IMPRIMIR, tokenizado.token.inicio, tokenizado.token.valor}};
		break;

	case T_NOMBRE: {
		char const* alias = tokenizado.token.inicio;
		int alias_n = tokenizado.token.valor;

		tokenizado = tokenizar(str, tabla_ops);
		str = tokenizado.resto;
		if (tokenizado.token.tag != T_IGUAL)
			return invalido(str, E_OPERACION);

		tokenizado = tokenizar(str, tabla_ops);
		str = tokenizado.resto;
		if (tokenizado.token.tag != T_CARGAR)
			return invalido(str, E_CARGA);

		// Convierto expresion postfija a infija, usando una pila:
		// Los valores sueltos, como numeros y aliases, los inserto en la pila.
		// Al encontrar un operador, extraigo tantas expresiones de la pila como
		// sea la aridad del operador, y creo la expresion que representa la
		// aplicacion del operador a sus operandos. Finalmente, inserto esa
		// expresion en la pila

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
				pila_de_expresiones_push(&p, expresion_numero(token.valor));
				} break;
			case T_NOMBRE: {
				pila_de_expresiones_push(&p, expresion_alias(token.inicio, token.valor));
				} break;
			case T_OPERADOR: {
				Expresion* arg1 = pila_de_expresiones_pop(&p);
				if (arg1 == NULL)
					goto fail_arg1;

				Expresion* arg2 = NULL;
				if (token.op->aridad == 2) {
					arg2 = pila_de_expresiones_pop(&p);
					if (arg2 == NULL)
						goto fail_arg2;
				}

				pila_de_expresiones_push(&p, expresion_operacion(token.op, arg1, arg2));
				break;

				fail_arg2:
				expresion_limpiar(arg1);

				fail_arg1:
				pila_de_expresiones_limpiar_datos(&p);
				return invalido(str, E_EXPRESION);

				} break;
			default:
				pila_de_expresiones_limpiar_datos(&p);
				return invalido(str, E_EXPRESION);
			}
		}

		Expresion* expresion = pila_de_expresiones_pop(&p);

		// si no se ingreso ninguna expresion informamos el error
		if (expresion == NULL) return invalido(str, E_VACIA);

		if (pila_de_expresiones_top(&p) != NULL) {
			expresion_limpiar(expresion);
			pila_de_expresiones_limpiar_datos(&p);
			return invalido(str, E_EXPRESION);
		}

		return (Parseado){str, (Sentencia){S_CARGA, alias, alias_n, expresion}};
		} break;

	default:
		return invalido(str, E_OPERACION);
	}
}

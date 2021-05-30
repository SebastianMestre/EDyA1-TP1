#include "interpretar.h"

#include "expresion.h"
#include "parser.h"
#include "expresion_infija.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER 1024

// Explicacion:
// para simplificar el uso de memoria, en vez de guardar los aliases, cada uno
// en su propia region de memoria, referenciamos su posicion original en la
// linea que ingreso el usuario, mediante un puntero. (char const* alias)
// Esta linea se guarda en su entrada correspondiente en la tabla de aliases
// especificamente, el puntero al buffer de entrada va en el campo 'input' de
// EntradaTablaAlias

typedef struct EntradaTablaAlias EntradaTablaAlias;
struct EntradaTablaAlias {
	EntradaTablaAlias* sig;

	char* input;
	char const* alias;
	int alias_n;
	Expresion* expresion;
};

typedef struct {
	EntradaTablaAlias* entradas;
} TablaAlias;

static EntradaTablaAlias* ta_encontrar(TablaAlias* tabla, char const* alias, int alias_n) {
	for (EntradaTablaAlias* it = tabla->entradas; it; it = it->sig)
		if (it->alias_n == alias_n && memcmp(it->alias, alias, alias_n) == 0)
			return it;
	return NULL;
}

// limpia 'input' y 'expresion'
static EntradaTablaAlias* ta_insertar(TablaAlias* tabla, char* input, char const* alias, int alias_n, Expresion* expresion) {
	EntradaTablaAlias* nuevo = malloc(sizeof(*nuevo));
	*nuevo = (EntradaTablaAlias) {
		.sig = tabla->entradas,
		.alias = alias,
		.alias_n = alias_n,
		.input = input,
		.expresion = expresion
	};
	tabla->entradas = nuevo;
	return nuevo;
}

// limpia 'input' y 'expresion'
static EntradaTablaAlias* ta_insertar_o_reemplazar(TablaAlias* tabla, char* input, char const* alias, int alias_n, Expresion* expresion) {
	EntradaTablaAlias* encontrado = ta_encontrar(tabla, alias, alias_n);

	if (encontrado == NULL)
		return ta_insertar(tabla, input, alias, alias_n, expresion);

	free(encontrado->input);
	encontrado->input = input;
	encontrado->alias = alias;

	expresion_limpiar(encontrado->expresion);
	encontrado->expresion = expresion;

	return encontrado;
}

static void ta_limpiar(TablaAlias* tabla) {
	EntradaTablaAlias* it = tabla->entradas;
	while (it) {
		EntradaTablaAlias* sig = it->sig;
		expresion_limpiar(it->expresion);
		free(it->input);
		free(it);
		it = sig;
	}
}



typedef struct {
	TablaAlias aliases;
	char* buffer_input;
	int tamano_buffer_input;
} Entorno;

static Entorno entorno_crear() {
	return (Entorno){};
}

static void leer_input(Entorno* entorno) {
	// NICETOHAVE que se banque renglones arbitrariamente grandes, usando un
	// buffer que crece dinamicamente si hace falta
	if (entorno->buffer_input == NULL) {
		entorno->buffer_input = malloc(BUFFER);
		entorno->tamano_buffer_input = BUFFER;
	}
	// NICETOHAVE: esto no se porta muy bien si se pasa de input: deja todo lo q
	// sobra en el buffer de entrada
	fgets(entorno->buffer_input, BUFFER - 1, stdin);
}

static void descartar_input(Entorno* entorno) {
	assert(entorno != NULL);
	free(entorno->buffer_input);
	entorno->buffer_input = NULL;
	entorno->tamano_buffer_input = 0;
}

static char* robar_input(Entorno* entorno) {
	char* buffer = entorno->buffer_input;
	entorno->buffer_input = NULL;
	entorno->tamano_buffer_input = 0;
	return buffer;
}

static void entorno_limpiar_datos(Entorno* entorno) {
	if (entorno->buffer_input != NULL)
		descartar_input(entorno);
	ta_limpiar(&entorno->aliases);
	return;
}

// TODO: testear evaluar_alias (y evaluar_arbol).
static int evaluar_arbol(Expresion* expresion, Entorno* entorno);

// encuentra la expresion correspondiente y llama a evaluar_arbol
static int evaluar_alias(Entorno* entorno, char const* alias, int alias_n) {
	EntradaTablaAlias* entradaAlias = ta_encontrar(&entorno->aliases, alias, alias_n);
	Expresion* expresion = entradaAlias->expresion;
	return evaluar_arbol(expresion, entorno);
}

// funcion auxliar a evaluar_alias; evalua el arbol de expresiones
// necesitamos el entorno en caso de encontrar un alias que evaluar									
static int evaluar_arbol(Expresion* expresion, Entorno* entorno) {
	if (expresion) {
		switch (expresion->tag) {
		case X_OPERACION: {
			// evaluamos los subarboles recursivamente y los usamos como args
			int args[2] =
				{evaluar_arbol(expresion->sub[0], entorno),
				evaluar_arbol(expresion->sub[1], entorno)};
			return expresion->op->eval(args);
		} break;
		case X_NUMERO:
			return expresion->valor;
			break;
		case X_ALIAS:
			return evaluar_alias(entorno, expresion->alias, expresion->valor);
			break;
		}
	}
	return 0;
} 

static Expresion* armar_expresion(Entorno* entorno, Expresion* expresion);

// devuelve la expresion asociada a un alias
static Expresion* leer_alias(Entorno* entorno, char const* alias, int alias_n) {
	Expresion* expresion = NULL;
	EntradaTablaAlias* entradaAlias = ta_encontrar(&entorno->aliases, alias, alias_n);
	if (entradaAlias) 
		expresion = armar_expresion(entorno, entradaAlias->expresion);
	// si el alias no existe, avisamos al usuario y devolvemos NULL
	else printf("El alias \'%.*s\' no esta definido.\n", alias_n, alias);
	return expresion;
}

// arma la expresion literal 
// se asegura de que no haya alias en el arbol de expresion
static Expresion* armar_expresion(Entorno* entorno, Expresion* expresion) {
	Expresion* expresionFinal = NULL;
	if (expresion) {
		switch (expresion->tag) {
		case X_OPERACION: {
			// paso recursivo
			Expresion* sub[2] = {
				armar_expresion(entorno, expresion->sub[0]),
				armar_expresion(entorno, expresion->sub[1])};
			// chequeamos la validez de los operandos
			// en caso de no ser validos, expresionFinal queda en NULL
			// si la operacion es unaria, alcanza con tener un unico operando
			if (expresion->op->aridad == 1 && sub[1] || sub[0] && sub[1]) {
				expresionFinal = malloc(sizeof(*expresionFinal));
				*expresionFinal = (Expresion){
					.tag = expresion->tag,
					.valor = expresion->valor,
					.sub = {sub[0], sub[1]},
					.op = expresion->op
				};
			}
		} break;
		case X_NUMERO:
			expresionFinal = malloc(sizeof(*expresionFinal));
			*expresionFinal = (Expresion){
				.tag = expresion->tag,
				.valor = expresion->valor};
			break;
		case X_ALIAS:
			// llamamos a leer_alias
			expresionFinal = leer_alias(entorno, expresion->alias, expresion->valor);
			break;
		}
	}
	return expresionFinal;
}

static void imprimir_expresion(Expresion* expresion, int precedencia, int izquierda) {
	// TODO
}

static void imprimir_alias(Entorno* entorno, char const* alias, int alias_n) {
	Expresion* expresion = leer_alias(entorno, alias, alias_n);
	if (expresion) {
		// si la expresion es un numero, no podemos buscar la precedencia
		if (expresion->tag == X_NUMERO) printf("%d\n", expresion->valor); 
		else imprimir_expresion(expresion, expresion->op->precedencia, 1);
	}
}

// limpia 'input' y 'expresion'
static void cargar(Entorno* entorno, char* input, char const* alias, int alias_n, Expresion* expresion) {
	ta_insertar_o_reemplazar(&entorno->aliases, input, alias, alias_n, expresion);
}

void interpretar(TablaOps* tabla_ops) {
	Entorno entorno = entorno_crear();
	while (1) {
		printf("> ");
		leer_input(&entorno);
		Parseado parseado = parsear(entorno.buffer_input, tabla_ops);
		Sentencia sentencia = parseado.sentencia;

		switch (sentencia.tag) {
		case S_CARGA:
			cargar(&entorno, robar_input(&entorno), sentencia.alias, sentencia.alias_n, sentencia.expresion);
			break;
		case S_IMPRIMIR:
			imprimir_alias(&entorno, sentencia.alias, sentencia.alias_n);
			break;
		case S_EVALUAR: {
			int resultado = evaluar_alias(&entorno, sentencia.alias, sentencia.alias_n);
			printf("%d\n", resultado);
			} break;
		case S_INVALIDO:
			puts("Error, saliendo."); // NICETOHAVE mejor error
		// fallthrough
		case S_SALIR:
			entorno_limpiar_datos(&entorno);
			return;
		}
	}
}

#ifndef PARSER_H
#define PARSER_H

typedef struct TablaOps TablaOps;
typedef struct EntradaTablaOps EntradaTablaOps;
typedef struct Expresion Expresion;

// En el contexto de un token, sirve para interpretar la informacion que este guarda
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

// Representa una secuencia de caracteres. Al consolidarlos en una sola entidad,
// podemos expresar algunas partes del codigo de forma mas clara.
typedef struct Token {
	TokenTag tag;
	char const* inicio;
	int valor;
	EntradaTablaOps* op;
} Token;

typedef struct Tokenizado {
	char const* resto;
	Token token;
} Tokenizado;


typedef enum {
	S_CARGA,    // ALIAS = cargar EXPR
	S_IMPRIMIR, // imprimir ALIAS
	S_EVALUAR,  // evaluar ALIAS
	S_SALIR,    // salir
	S_INVALIDO, // (un error)
} SentenciaTag;

typedef struct Sentencia {
	SentenciaTag tag;
	char const* alias;
	int alias_n;
	Expresion* expresion;
} Sentencia;

typedef struct {
	char const* resto;
	Sentencia sentencia;
} Parseado;

Tokenizado tokenizar(char const* str, TablaOps* tabla_ops);
Parseado parsear(char const* str, TablaOps* tabla_ops);

#endif // PARSER_H

#ifndef PARSER_H
#define PARSER_H

typedef struct TablaOps TablaOps;
typedef struct EntradaTablaOps EntradaTablaOps;
typedef struct Expresion Expresion;

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

// Analiza el pricipio del string, y extrae una pieza, dandole sentido.
// Luego, devuelve una representacion de esa pieza, y un puntero a donde esa
// pieza termina, y empieza el resto del string.
//
// # uso de memoria
// argumentos: No limpia nada
// resultado: Nada se debe limpiar
Tokenizado tokenizar(char const* str, TablaOps* tabla_ops);


// En el contexto de una Sentencia, sirve para interpretar la informacion de esta
typedef enum {
	S_CARGA,    // ALIAS = cargar EXPR
	S_IMPRIMIR, // imprimir ALIAS
	S_EVALUAR,  // evaluar ALIAS
	S_SALIR,    // salir
	S_INVALIDO, // (un error)
} SentenciaTag;

// Representa una accion que debe tomar el programa
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

// Analiza el principio del string, y reconoce la primera accion que debe tomar
// el interprete. Luego, devuelve una representacion de ella, junto a un puntero
// al resto del string, que todavia no fue analizado.
//
// # uso de memoria
// argumentos: No limpia nada
// resultado: depende de sentencia.tag
//  - si es S_CARGA, se debe limpiar la sentencia.expresion
//  - En el resto de los casos, nada se debe limpiar
Parseado parsear(char const* str, TablaOps* tabla_ops);

#endif // PARSER_H

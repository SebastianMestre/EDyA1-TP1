#include "tabla_ops.h"
#include "operadores.h"
#include "interprete/interpretar.h"

int main (int argc, char** argv) {
	TablaOps tabla = tabla_ops_crear();
	
	cargar_operador(&tabla, "+", 2, suma);
	cargar_operador(&tabla, "-", 2, resta);
	cargar_operador(&tabla, "--", 1, opuesto);
	cargar_operador(&tabla, "*", 2, producto);
	cargar_operador(&tabla, "/", 2, division);
	cargar_operador(&tabla, "%", 2, modulo);
	cargar_operador(&tabla, "^", 2, potencia);

	interpretar(&tabla);

	tabla_ops_limpiar(&tabla);
}

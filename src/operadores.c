#include "operadores.h"

int suma(int* args) {
	return args[1] + args[0];
}

int resta(int* args) {
	return args[1] - args[0];
}

int opuesto(int* args) {
	return -args[0];
}

int producto(int* args) {
	return args[1] * args[0];
}

int division(int* args) {
	return args[1] / args[0];
}

int modulo(int* args) {
	return args[1] % args[0];
}

int potencia(int* args) {
	int b = args[1];
	int n = args[0];
	if (n < 0)
		return b > 1 ? 0 : 1;
	int k = 1;
	while (n--)
		k *= b;
	return k;
}

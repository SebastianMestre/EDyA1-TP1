# EDyA I -- TP1

# Tests de manejo de memoria

En la carpeta `tests/` hay algunos archivos llamados `memory_test*`. Estos son
inputs que se pueden dar el interprete por stdin. Al ejecutar el interprete en
Valgrind, y pasarle cada uno de estos archivos, verificando que ninguno exponga
un error, se logra testear el manejo de memoria en el programa.

Para correr esos tests, se puede usar el script `run_memory_tests.sh`

Notar que se debe tener instalado valgrind y un shell UNIX-compatible

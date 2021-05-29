INTDIR = src/interprete

interprete: build/main.o build/interpretar.o build/tabla_ops.o build/operadores.o build/expresion.o build/parser.o
	gcc -o $@ $^

clean:
	rm -rf build/
	rm -f interprete
.PHONY: clean

build/main.o:        src/main.c $(INTDIR)/interpretar.h src/tabla_ops.h src/funcion_evaluacion.h src/operadores.h
build/interpretar.o: $(INTDIR)/interpretar.c $(INTDIR)/interpretar.h src/tabla_ops.h src/funcion_evaluacion.h $(INTDIR)/expresion.h $(INTDIR)/parser.h
build/tabla_ops.o:   src/tabla_ops.c src/tabla_ops.h src/funcion_evaluacion.h
build/operadores.o:  src/operadores.c src/operadores.h
build/expresion.o:   $(INTDIR)/expresion.c $(INTDIR)/expresion.h src/funcion_evaluacion.h
build/parser.o:      $(INTDIR)/parser.c $(INTDIR)/parser.h src/tabla_ops.h src/funcion_evaluacion.h

build/%.o: src/%.c
	mkdir -p build
	gcc -fno-omit-frame-pointer -g -c -o $@ $<

build/%.o: $(INTDIR)/%.c
	mkdir -p build
	gcc -fno-omit-frame-pointer -g -c -o $@ $<

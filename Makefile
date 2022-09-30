all:
	gcc taqueria.c simlib.c -o programa -lm
clean:
	rm -f taqueria.out programa


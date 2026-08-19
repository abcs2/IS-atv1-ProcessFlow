obj/main.o: source/main.c
	gcc -c source/main.c

processflow: objects/main.o
	gcc objects/main.o -o processflow

clean:
	rm -f *.o processflow

.PHONY: clean
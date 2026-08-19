objects/main.o: source/main.c
	gcc -c source/main.c -o objects/main.o

processflow: objects/main.o
	gcc objects/main.o -o processflow

clean:
	rm -f objects/*.o processflow

.PHONY: clean
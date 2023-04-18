all:
	gcc cl.c -pthread -ansi -pedantic -Wall -pthread -std=c17 -o cl
	./cl

all: hw4

hw4: hw4.o
	gcc -o hw4 hw4.o

hw4.o: hw4.c
	gcc -c hw4.c

clean:
	rm -f hw4.o

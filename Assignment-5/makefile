library: mysocket.c mysocket.h
	gcc -c mysocket.c -pthread -o mysocket.o
	ar rcs libmsocket.a mysocket.o

clean:
	rm -f libmsocket.a *.o
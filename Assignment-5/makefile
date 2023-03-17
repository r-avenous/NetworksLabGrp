library: mysocket.c mysocket.h
	gcc -c mysocket.c -pthread -o mysocket.o
	ar rcs libmysocket.a mysocket.o

clean:
	rm -f libmysocket.a *.o
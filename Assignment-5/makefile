library: mysocket.c mysocket.h
	gcc -c mysocket.c -pthread -o mysocket.o
	ar rcs libmysocket.a mysocket.o

test: mysocket.c mysocket.h
	gcc testserv.c mysocket.c -pthread -o ts
	gcc testcli.c mysocket.c -pthread -o tc

output: libmysocket.a
	gcc testserv.c -L. -lmysocket -o ts
	gcc testcli.c -L. -lmysocket -o tc

clean:
	rm -f ts tc libmysocket.a *.o
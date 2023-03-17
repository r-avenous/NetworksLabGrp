test: libmysocket.a
	gcc testserv.c -L. -lmysocket -o ts
	gcc testcli.c -L. -lmysocket -o tc

library: 
	gcc -c ../mysocket.c -pthread -o mysocket.o
	ar rcs libmysocket.a mysocket.o

clean:
	rm -f ts tc libmysocket.a *.o
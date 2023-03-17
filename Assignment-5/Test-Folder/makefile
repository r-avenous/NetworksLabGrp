test: libmsocket.a
	gcc testserv.c -L. -lmsocket -o ts
	gcc testcli.c -L. -lmsocket -o tc

library: 
	gcc -c ../mysocket.c -pthread -o mysocket.o
	ar rcs libmsocket.a mysocket.o

clean:
	rm -f ts tc libmsocket.a *.o
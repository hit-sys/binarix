PREFIX = /usr
CFLAGS = -Os -Wall
LIBS   = -lcurses -lcrypt

all: binarix.o secure.o
	gcc binarix.o secure.o -o binarix ${LIBS} ${CFLAGS}

binarix.o: binarix.c
	gcc -c binarix.c -o binarix.o ${CFLAGS}

secure.o: secure.c
	gcc -c secure.c -o secure.o ${CFLAGS}

install:
	install -m 4555 -o root binarix ${PREFIX}/bin/

uninstall:
	rm ${PREFIX}/bin/binarix

clean:
	find . -iname "*.o" -exec rm -f {} \;
	rm -f binarix

CPP=gcc
CFLAGS=-O3 -g
LDFLAGS=`pkg-config --cflags --libs gtk+-3.0`

all: 
	${CPP} $(CFLAGS) client.c -o client $(LDFLAGS)
	${CPP} $(CFLAGS) serveur.c -o serveur $(LDFLAGS)

clean:	
	rm -fr *.o

mrproper: clean
	rm -fr ${EXEC}

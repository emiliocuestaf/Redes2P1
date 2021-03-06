#Autores:
#	-Emilio Cuesta Fernandez
# 	-Luis Carabe Fernandez-Pedraza
#	2302 - pareja 05

# Makefile para generar el ejecutable server


CCFLAGS = -Wextra -std=gnu11 -Wall -g


SRC = ./src
INCLIB = ./includes

OBJS =	concurrent_server.o \
		socket_management.o \
		http_processing.o \
		threadPool.o \
		picohttpparser.o \
		daemon.o


all : concurrent_server
	rm -f *.o

concurrent_server: $(OBJS) 
	gcc $(CCFLAGS) -o server $^ -lpthread -lconfuse

concurrent_server.o:	$(SRC)/concurrent_server.c
	gcc $(CCFLAGS) -I./includes -c $(SRC)/concurrent_server.c

socket_management.o:	$(SRC)/socket_management.c $(INCLIB)/socket_management.h
	gcc $(CCFLAGS) -I./includes -c $(SRC)/socket_management.c

http_processing.o:	$(SRC)/http_processing.c $(INCLIB)/http_processing.h
	gcc $(CCFLAGS) -I./includes -c $(SRC)/http_processing.c

threadPool.o:	$(SRC)/threadPool.c $(INCLIB)/threadPool.h
	gcc $(CCFLAGS) -I./includes -c $(SRC)/threadPool.c

picohttpparser.o:	$(SRC)/picohttpparser.c $(INCLIB)/picohttpparser.h
	gcc $(CCFLAGS) -I./includes -c $(SRC)/picohttpparser.c

daemon.o:	$(SRC)/daemon.c $(INCLIB)/daemon.h
	gcc $(CCFLAGS) -I./includes -c $(SRC)/daemon.c

clean:
	rm -f *.o server
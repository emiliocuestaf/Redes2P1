/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Unidad de manejo de sockets
********************************************************/

#include "socket_management.h"

//Estas son las estructuras con la que vamos a trabajar. Pertenece a <netinet/in.h>

/*struct addrinfo {
    int     ai_flags;
    int     ai_family;
    int     ai_socktype;
    int     ai_protocol;
    size_t  ai_addrlen;
    struct  sockaddr* ai_addr;
    char*   ai_canonname;     // canonical name 
    struct  addrinfo* ai_next; // this struct can form a linked list 
};

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
}; */

/********
* FUNCIÓN: int socket_create(struct addrinfo* addr)
* ARGS_IN: struct addrinfo* addr - estructura que caracteriza al socket que queremos crear
* DESCRIPCIÓN: Crea un socket
* ARGS_OUT: int - devuelve el retorno de socket (-1 en caso de error, el descriptor de fichero en caso contrario)
********/

int socket_create(struct addrinfo* addr){
	if(addr == NULL)
        return -1;
    return socket(addr->ai_family, addr->ai_socktype,  addr->ai_protocol);
}

/********
* FUNCIÓN: 	int socket_bind(struct addrinfo* addr, int sockval)
* ARGS_IN: 	struct addrinfo* addr - estructura que contiene informacion con la que asociar el socket
*			int sockval - descriptor del socket
* DESCRIPCIÓN: Asociamos un socket con un puerto contenido en addr
* ARGS_OUT: int - devuelve el retorno de bind (-1 en caso de error, 0 en caso contrario)
********/

int socket_bind(struct addrinfo* addr, int sockval){
    if(addr == NULL || sockval < 0)
        return -1;
    return bind(sockval, addr->ai_addr, addr->ai_addrlen);
}

/********
* FUNCIÓN: 	int socket_listen(int sockval, int max_clients)
* ARGS_IN: 	int max_clients - numero maximo de conexiones permitidas
*			int sockval - descriptor del socket
* DESCRIPCIÓN: Espera conexiones entrantes del socket
* ARGS_OUT: int - devuelve el retorno de listen (-1 en caso de error, 0 en caso contrario)
********/

int socket_listen(int sockval, int max_clients){
    if(max_clients < 0 || sockval < 0)
        return -1;
    return listen (sockval, max_clients);
}

/********
* FUNCIÓN: 	int socket_connect(int clientsock, struct addrinfo* addr)
* ARGS_IN: 	struct addrinfo* addr - estructura que contiene informacion del socket al cual haces la peticion
*			int sockval - descriptor del socket
* DESCRIPCIÓN: Crea una peticion de conexion del socket clientsock a la direccion addr
* ARGS_OUT: int - devuelve el retorno de connect (-1 en caso de error, 0 en caso contrario)
********/

int socket_connect(int clientsock, struct addrinfo* addr){
    if(clientsock < 0 || addr == NULL)
        return -1;
    return connect(clientsock, addr->ai_addr, addr->ai_addrlen);
}

/********
* FUNCIÓN: 	int my_receive (int clientsock, char* inBuffer, long int buf_size)
* ARGS_IN: 	int clientsock - socket que da la informacion tenemos que leer
			char* inBuffer - buffer donde guardamos lo leido
			long int buf_size - tamanio maximo que podemos leer
* DESCRIPCIÓN: Lee los datos enviados por clientsock y los guarda en inBuffer
* ARGS_OUT: int - devuelve el retorno de recv (-1 en caso de error, bytes leidos en caso contrario)
********/

int my_receive (int clientsock, char* inBuffer, long int buf_size){
    if(inBuffer == NULL)
        return -1;
    return recv(clientsock, inBuffer, buf_size, 0);
}

/********
* FUNCIÓN: 	int my_send (int clientsock, char* ouBuffer, int length)
* ARGS_IN: 	int clientsock - socket donde vamos a enviar la informacion
			char* outBuffer - buffer donde esta almacenada la informacion
			int length - tamanio de la informacion a enviar
* DESCRIPCIÓN: Envia los datos almacenados en outBuffer al clientsock
* ARGS_OUT: int - devuelve el retorno de send (-1 en caso de error, bytes enviados en caso contrario)
********/

int my_send (int clientsock, char* outBuffer, int length){
    if(outBuffer == NULL)
        return -1;
    return send(clientsock, outBuffer, length, 0);
}

/********
* FUNCIÓN: 	int server_socket_setup(struct addrinfo* addr, int max_clients)
* ARGS_IN: 	struct addrinfo* addr - estructura que almacena la informacion que queremos asociar al socket
			int max_clients - numero maximo de conexiones permitidas
* DESCRIPCIÓN: Pone en marcha el socket del servidor
* ARGS_OUT: int - devuelve -1 en caso de error, el descriptor del socket en caso contrario
********/

int server_socket_setup(struct addrinfo* addr, int max_clients){
	int sockval;
	int opt;

	/*Creamos el socket*/

	syslog (LOG_INFO, "Creating socket");
	sockval = socket_create(addr);
	if (sockval < 0 ){
		syslog(LOG_ERR, "Error creating socket");
		return -1;
	}

	/*Ajustamos las opciones del socket que acabamos de crear, permitiendo que pueda reusar direcciones*/

	opt = 1;
	if (setsockopt(sockval, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(int)) < 0) {
      syslog(LOG_ERR, "[bindListener:43:setsockopt]");
      return -1;
    }

    /* Asociamos el socket creado con el puerto correspondiente guardado en addr*/

	syslog (LOG_INFO, "Binding socket");

	if (socket_bind(addr, sockval) < 0){
		syslog(LOG_ERR, "Error binding socket");
		return -1;
	}

	/*Hacemos que el socket se ponga a escuchar posibles peticiones que le lleguen*/

	syslog(LOG_INFO, "Listening connections");
	if (socket_listen(sockval, max_clients) < 0){
		syslog(LOG_ERR, "Error listenining");
		return -1;
	}

	return sockval;

}

/********
* FUNCIÓN: 	int accept_connection(int sockval)
* ARGS_IN: 	int sockval - descriptor del socket
* DESCRIPCIÓN: Acepta una conexion de la cola de conexiones del socket
* ARGS_OUT: int - devuelve -1 en caso de error, el descriptor del socket que hemos aceptado en caso contrario
********/

int accept_connection(int sockval){
	int desc;
    socklen_t len;
	struct sockaddr Conexion;

	/*Llamamos a la funcion que acepta una conexion de la cola de conexiones de sockval, guardamos la informacion del socket que
	se quiere comunicar con nosotros en Conexion*/

	len = sizeof(Conexion);
    desc = accept(sockval, &Conexion, &len);
	if (desc <0){
		syslog(LOG_ERR, "Error accepting connection");
		return -1;
	}

	return desc;
	
}
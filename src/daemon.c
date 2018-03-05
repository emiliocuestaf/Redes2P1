/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Modulo que se encarga de demonizar un proceso
********************************************************/
#include "daemon.h"

/********
* FUNCIÓN: int demonizar()
* DESCRIPCIÓN: Se encarga de demonizar un proceso
* ARGS_OUT: int - devuelve 1 en caso de error, 0 en caso contrario
********/

int demonizar(){
	pid_t pid;

	/*Creamos un proceso hijo que se encargara de ejecutar el servidor*/
	pid = fork(); 
	if (pid < 0) return -1;
	if (pid > 0) exit(EXIT_SUCCESS); /*Matamos al padre*/
	
	umask(0); /*Cambiamos los permisos de los archivos y directorios creados por el proceso */
	/*Es decir, cambiamos la mascara de modo de ficheros para que sean accesibles a cualquiera*/

	/*Abrimos los logs correspondientes, que usaremos para que el servidor nos comunique mensajes importantes*/

	setlogmask (LOG_UPTO (LOG_INFO)); 
	openlog ("Server system messages:", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL3);
	syslog (LOG_ERR, "Iniciando nuevo servidor.");
	
	if (setsid()< 0) { /* Creamos un nuevo SID para el proceso hijo */
		syslog (LOG_ERR, "Error creando un nuevo SID para el proceso hijo.");
		return -1;
	}
	
	/*Cambiamos el directorio de trabajo actual por el directorio raiz*/
	if ((chdir("/")) < 0) {
		syslog (LOG_ERR, "Error cambiando el directorio de trabajo = \"/\"");
		return -1;
	}
	
	/*Cerramos los descriptores de fichero usados*/
	syslog (LOG_INFO, "Cerramos descriptores de fichero");
	close(STDIN_FILENO);
	close(STDOUT_FILENO); 
	close(STDERR_FILENO); 

	return 0;
}

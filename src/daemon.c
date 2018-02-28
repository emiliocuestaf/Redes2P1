#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "daemon.h"


void do_daemon(){
	pid_t pid;

	/*Creamos un proceso hijo que se encargara de ejecutar el servidor*/
	pid = fork(); 
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid > 0) exit(EXIT_SUCCESS); /*Matamos al padre*/
	
	umask(0); /*Cambiamos los permisos de los archivos y directorios creados por el proceso */

	/*Abrimos los logs correspondientes, que usaremos para que el servidor nos comunique mensajes importantes*/

	setlogmask (LOG_UPTO (LOG_INFO)); 
	openlog ("Server system messages:", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL3);
	syslog (LOG_ERR, "Iniciando nuevo servidor.");
	
	if (setsid()< 0) { /* Creamos un nuevo SID para el proceso hijo */
		syslog (LOG_ERR, "Error creando un nuevo SID para el proceso hijo.");
		exit(EXIT_FAILURE);
	}
	
	/*Cambiamos el directorio de trabajo actual por el directorio raiz*/
	if ((chdir("/")) < 0) {
		syslog (LOG_ERR, "Error cambiando el directorio de trabajo = \"/\"");
		exit(EXIT_FAILURE);
	}
	
	/*Cerramos los descriptores de fichero usados*/
	syslog (LOG_INFO, "Cerramos descriptores de fichero");
	close(STDIN_FILENO);
	close(STDOUT_FILENO); 
	close(STDERR_FILENO); 
}

/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
* 	-Luis Carabe Fernandez-Pedraza
*	-Emilio Cuesta Fernandez
* Descripcion:
*	Modulo que se encarga de demonizar un proceso
********************************************************/

#ifndef DAEMON_H
	#define DAEMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>


/********
* FUNCIÓN: int demonizar()
* DESCRIPCIÓN: Se encarga de demonizar un proceso
* ARGS_OUT: int - devuelve -1 en caso de error, 0 en caso contrario
********/

int demonizar();

#endif

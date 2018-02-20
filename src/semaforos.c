/**
* @brief Programa correspondiente al ejercicio 4 de la práctica 3 de Sistemas Operativos
*
* En este fichero implementamos la librería semáforos.h, que nos dará las funciones básicas para manipular 
* semáforos.
*
* @file semaforos.c
* @author Luis Carabe y Emilio Cuesta
* @date 05-04-2017
*/

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>
#include "semaforos.h"

/**
* @brief Funcion que inicializa los semaforos indicados
* @param int semid: identificador del semaforo
* @param unsigned short* array: valores iniciales del semaforo
* @return OK si todo fue correcto, ERROR en caso de error
*/

int Inicializar_Semaforo(int semid, unsigned short *array){
    int n_sem;
    union semun {
        int val;
        struct semid_ds *semstat;
        unsigned short *array;
    } arg;

    if(semid < 0){
        return ERROR;
    }

    arg.array = array;
    /*Inicializamos los valores*/
    if(semctl (semid, 1, SETALL, arg)==-1){
        return ERROR;
    }
    return OK;
}
/**
* @brief Funcion que borra un semaforo
* @param int semid: identificador del semaforo a eliminar
* @return OK si todo fue correcto, ERROR en caso de error
*/
 
int Borrar_Semaforo(int semid){
    if(semid < 0){
        return ERROR;
    }    
    if(semctl (semid,0, IPC_RMID, 0) == -1){
        return ERROR;
    }
    return OK;
}
/**
* @brief Funcion que crea un semaforo(inicializándolo a 0) con la clave y el tamaño especificado.
* @param key_t key: clave precompartida del semaforo
* @param int *semid: identificador del semaforo
* @param int size: tamaño del semaforo
* @return ERROR en caso de error, 0 si ha creado el semáforo o 1 si ya estaba creado
*/

int Crear_Semaforo(key_t key, int size, int *semid){
    int i;
    union semun {
        int val;
        struct semid_ds *semstat;
        unsigned short *array;
    } arg;
    if (size < 0){
        return ERROR;
    }


    /*Creamos el semáforo*/

    *semid = semget(key, size, IPC_CREAT | IPC_EXCL | SHM_R | SHM_W);
    if (*semid == -1){
        if(errno == EEXIST){
            *semid = semget(key,size,SHM_R|SHM_W);
            return 1;
        }
        return ERROR;
    }
    /*Inicializamos el semáforo*/

    arg.array = (unsigned short*) malloc (size*sizeof(unsigned short));
    for(i = 0; i < size; i++){
        arg.array[i] = 0;
    }

    if(semctl (*semid, size, SETALL, arg)==-1){
        free(arg.array);
        return ERROR;
    }
    free(arg.array);
    return 0;
}

/**
* @brief baja el semaforo indicado  
* @param int id: identificador del semaforo. 
* @param int num_sem: semaforo dentro del array. 
* @param int undo: flag de modo persistente pese a finalización abrupta.
* @return OK si todo fue correcto, ERROR en caso de error.
*/

int Down_Semaforo(int id, int num_sem, int undo){
    struct sembuf sem_oper;
    if(id < 0 || num_sem < 0){
        return ERROR;
    }
    sem_oper.sem_num = num_sem;
    sem_oper.sem_op = -1;
    sem_oper.sem_flg = undo;
    if(semop (id, &sem_oper, 1) == -1){
        return ERROR;
    }
    return OK;
}

/**
* @brief baja todos los semaforos del array pasado como argumento
* @param int id: identificador del semaforo. 
* @param int size: numero de semaforos del array. 
* @param int undo: flag de modo persistente pese a finalización abrupta.
* @param int *active: semaforos involucrados
* @return OK si todo fue correcto, ERROR en caso de error.
*/

int DownMultiple_Semaforo(int id,int size,int undo, int *active){
    struct sembuf sem_oper;
    int i, tam;
    if(id < 0 || size < 0){
        return ERROR;
    }
    sem_oper.sem_op = -1;
    sem_oper.sem_flg = undo;
    /*Realizamos el down pasando por todos los semáforos del array*/
    for(i = 0; i < size; i++){
        sem_oper.sem_num = active[i];
        if(semop (id, &sem_oper, 1) == -1){
            return ERROR;
        }
    }

    return OK;
}

/**
* @brief sube el semaforo indicado  
* @param int id: identificador del semaforo. 
* @param int num_sem: semaforo dentro del array. 
* @param int undo: flag de modo persistente pese a finalización abrupta.
* @return OK si todo fue correcto, ERROR en caso de error.
*/

int Up_Semaforo(int id, int num_sem, int undo){
    struct sembuf sem_oper;
    if(id < 0 || num_sem < 0){
        return ERROR;
    }
    sem_oper.sem_num = num_sem;
    sem_oper.sem_op = 1;
    sem_oper.sem_flg = undo;
    if(semop (id, &sem_oper, 1) == -1){
        return ERROR;
    }
    return OK;
}
/**
* @brief sube todos los semaforos del array pasado como argumento
* @param int id: identificador del semaforo. 
* @param int size: numero de semaforos del array. 
* @param int undo: flag de modo persistente pese a finalización abrupta.
* @param int *active: semaforos involucrados
* @return OK si todo fue correcto, ERROR en caso de error.
*/

int UpMultiple_Semaforo(int id,int size, int undo, int *active){
    struct sembuf sem_oper;
    int i, tam;
    if(id < 0 || size < 0){
        return ERROR;
    }
    sem_oper.sem_op = 1;
    sem_oper.sem_flg = undo;

    /*Realizamos el up pasando por todos los semáforos del array*/
    for(i = 0; i < size; i++){
        sem_oper.sem_num = active[i];
        if(semop (id, &sem_oper, 1) == -1){
            return ERROR;
        }
    }


    return OK;
}

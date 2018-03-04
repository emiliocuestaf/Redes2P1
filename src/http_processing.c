#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include "http_processing.h"
#include "picohttpparser.h"
#include "socket_management.h"

#define DIREC_SIZE 100
#define COMMAND_SIZE 300

#define TIMEOUT 10

#define NO_SCRIPT 0
#define PYTHON_SCRIPT 1
#define PHP_SCRIPT 2 


/*Funcion que genera la fecha actual*/

char* get_date(){
  char *buf = malloc(sizeof(char)*35);
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  if(buf == NULL)
  	return NULL;
  strftime(buf, 35, "%a, %d %b %Y %H:%M:%S %Z", &tm);
  return buf;
}

/*Funcion que devuelve la fecha de la ultima modificacion*/

char* get_mod_time(struct stat* fStat) {
    char* buf = malloc(sizeof(char)*35);
    strftime(buf, 35, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&(fStat->st_ctime)));
    return buf;
}

/*Funcion que devuelve el Content-Type de un fichero, a excepcion de los 
ficheros .py o .php cuyo retorno es su misma extension*/

char *filename_ext(char *fname) {
    char *dot = strrchr(fname, '.');
    if(dot){

      if(strcmp(dot+1, "txt") == 0)
        return "text/plain";
      else if(strcmp(dot+1, "html") == 0 || strcmp(dot+1, "htm") == 0)
        return "text/html";
      else if(strcmp(dot+1, "gif") == 0 )
        return "image/gif";
      else if(strcmp(dot+1, "jpeg") == 0 || strcmp(dot+1, "jpg") == 0)
        return "image/jpeg";
      else if(strcmp(dot+1, "mpeg") == 0 || strcmp(dot+1, "mpg") == 0)
        return "video/mpeg";   
      else if(strcmp(dot+1, "doc") == 0 || strcmp(dot+1, "docx") == 0)
        return "application/msword";
      else if(strcmp(dot+1, "pdf") == 0)
        return "application/pdf";
      else if(strcmp(dot+1, "py") == 0)
        return "py";
      else if(strcmp(dot+1, "php") == 0)
        return "php";
      else
        return "";
    } 
    return "";
}


/*Funcion que devuelve los metodos validos en cada carpeta. 
  Retorno: Devuelve los datos que nos interesan dentro la estructura*/
int allowed_methods(allowedMethods* met, char* cleanpath){
  if( (met == NULL) || (cleanpath == NULL))
    return ERROR;

  char delim = '/';
  char * token1;
  char * token2;
  char* aux;


  aux = (char *) malloc (strlen(cleanpath)+1);
  strcpy(aux, cleanpath);

  if((strcmp(aux, "/*") == 0) || (strcmp(aux, "*") == 0)){
    met->nummethods = 3;
    strcpy(met->methods[0],"GET");
    strcpy(met->methods[1], "OPTIONS");
    strcpy(met->methods[2], "POST");
    strcpy(met->txtChain, "GET,OPTIONS,POST");
    free(aux);
    return OK;
  }


  token1=strtok(aux, &delim);
  if(token1 == NULL){
    free(aux);
    return ERROR;
  }
  token2=strtok(NULL, &delim);

  if((strcmp(token1, "files") && (token2 == NULL))){
    met->nummethods = 3;
    strcpy(met->methods[0],"GET");
    strcpy(met->methods[1], "OPTIONS");
    strcpy(met->methods[2], "POST");
    strcpy(met->txtChain, "GET,OPTIONS,POST");
    free(aux);
    return OK;
  }
  else if((strcmp(token1, "files") == 0) && (strcmp(token2, "docs") == 0)){
    met->nummethods = 2;
    strcpy(met->methods[0],"GET");
    strcpy(met->methods[1], "OPTIONS");
    strcpy(met->txtChain, "GET,OPTIONS");
    free(aux);
    return OK;
  }
  else if((strcmp(token1, "files") == 0) && (strcmp(token2, "images") == 0)){
    met->nummethods = 2;
    strcpy(met->methods[0],"GET");
    strcpy(met->methods[1], "OPTIONS");
    strcpy(met->txtChain, "GET,OPTIONS");
    free(aux);
    return OK;
  }
  else if((strcmp(token1, "files") == 0) && (strcmp(token2, "scripts") == 0)){
    met->nummethods = 3;
    strcpy(met->methods[0],"GET");
    strcpy(met->methods[1], "OPTIONS");
    strcpy(met->methods[2], "POST");
    strcpy(met->txtChain, "GET,OPTIONS,POST");
    free(aux);
    return OK;
  }
  else if((strcmp(token1, "files") == 0) && (strcmp(token2, "videos") == 0)){
    met->nummethods = 2;
    strcpy(met->methods[0],"GET");
    strcpy(met->methods[1], "OPTIONS");
    strcpy(met->txtChain, "GET,OPTIONS");
    free(aux);
    return OK;
  }
  else if((strcmp(token1, "files") == 0) && (strcmp(token2, "html") == 0)){
    met->nummethods = 2;
    strcpy(met->methods[0],"GET");
    strcpy(met->methods[1], "OPTIONS");
    strcpy(met->txtChain, "GET,OPTIONS");
    free(aux);
    return OK;
  }
  else{
    met->nummethods = 1;
    strcpy(met->methods[0], "OPTIONS");
    strcpy(met->txtChain, "OPTIONS");
    free(aux);
    return OK;
  }

  free(aux);
  syslog(LOG_ERR, "Error en HTTP_PROCESSING: Error en allowed methods");
  return ERROR;
}

/*Funcion que parsea una peticion HTTP*/

int parse_petition(int csock, char* inBuffer, char* signature, char* root, long int buf_size){
    char* server_signature = NULL;
    char direc[DIREC_SIZE];
    char *method, *path, *cleanpath, *args, *auxpath, *qptr, *body;
    int clientsock;
    int max_buffer;
    int i;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;
    allowedMethods am;
    
    server_signature = signature;
    max_buffer = buf_size;
    clientsock = csock;
    
    num_headers = sizeof(headers) / sizeof(headers[0]);
    
    /*LLamamos a la funcion que se encarga de parsear la peticion*/
    pret = phr_parse_request(inBuffer, (ssize_t) strlen(inBuffer), (const char**)&method, &method_len,(const char**) &path, &path_len, &minor_version, headers, &num_headers, (size_t) 0);
    
    if ((pret == -1) || (strlen(inBuffer) == sizeof(inBuffer))){
        if(error_response(server_signature, clientsock, path, 400, minor_version, max_buffer) == ERROR)
          return ERROR;
        return OK;
    }

    /* Guardamos el cuerpo de la petición, sumando el num de bytes de la cabecera (que devuelve phr_parse_request) a la dir inicial de inBuffer*/

    body = inBuffer + pret;

    /*Guardamos el método*/
    char aux[20];
    sprintf(aux, "%.*s", (int)method_len, method);


    /*Para limpiar el path, es necesario distinguir si se pasan argumentos detras de un ? o no,
      aunque luego tengan que ser ignorados si el fichero solicitado no es un script*/
    auxpath = (char*) malloc ((path_len+1)* sizeof(char));
    cleanpath = (char*) malloc ((path_len+1)* sizeof(char));
    args = (char*) malloc ((path_len+1)* sizeof(char));

    sprintf(auxpath, "%.*s", (int)path_len, path);
    qptr = strchr(auxpath, '?');
    /*Caso en que no hay ?, path_len funciona bien*/
    if(qptr == NULL){
      strcpy(cleanpath, auxpath);
      free(auxpath);
      strcpy(args,"");
    }
    else{
      /*Caso en que hay ?, con aritmetica de punteros se puede solucionar
        Tambien se guarda la cadena de argumentos, que mas tarde puede ser necesaria */
      sprintf(cleanpath, "%.*s", (int) (qptr-auxpath)*sizeof(char), auxpath);
      sprintf(args, "%.*s", (int) ((auxpath+path_len)-qptr)*sizeof(char), qptr+1);

      free(auxpath);
    }
      FILE* fu;
      fu = fopen("registrorutas.txt", "a");
      fprintf(fu, "%s\n", cleanpath);
      fclose(fu);

      fu = fopen("regsitroargumentos.txt", "a");
      fprintf(fu, "%s\n", args);
      fclose(fu);

    if(allowed_methods(&am, cleanpath) == ERROR){
        if(error_response(server_signature, clientsock, path, 400, minor_version, max_buffer) == ERROR)
          return ERROR;
        return OK;
    }
    /*Guardamos la ruta del fichero concatenando con server_root*/
    sprintf(direc, "%s%s", root, cleanpath);

  	if(strcmp(aux, "GET") == 0){
      for(i=0; i < am.nummethods; i++){
        if(strcmp(am.methods[i], "GET") == 0){
          if(get_response(server_signature, clientsock, direc, cleanpath, args, max_buffer, minor_version) == ERROR){
            syslog(LOG_ERR, "Error en HTTP Response GET.");
            free(cleanpath);
            free(args);
            return ERROR;
          }
          free(args);
          free(cleanpath);
          return OK;
        }
      }
      if(error_response(server_signature, clientsock, cleanpath, 405, minor_version, max_buffer) == ERROR){
          free(cleanpath);
          free(args);
          return ERROR;
      }
  	}
    else if(strcmp(aux, "POST") == 0){
      for(i=0; i < am.nummethods; i++){
        if(strcmp(am.methods[i], "POST") == 0){
          //CÒMPROBAR SI BODY TIENE ALGO 
          if(post_response(server_signature, clientsock, direc, cleanpath, body, args, max_buffer, minor_version) == ERROR){
              syslog(LOG_ERR, "Error en HTTP Response POST.");
              free(cleanpath);
              free(args);
              return ERROR;
          }
          free(args);
          free(cleanpath);
          return OK;
        }
      }
      if(error_response(server_signature, clientsock, cleanpath, 405, minor_version, max_buffer) == ERROR){
          free(cleanpath);
          free(args);
          return ERROR;
      } 
    }	
  	else if (strcmp(aux, "OPTIONS") == 0){
      for(i=0; i < am.nummethods; i++){
        if(strcmp(am.methods[i], "OPTIONS") == 0){
          if(options_response(server_signature, clientsock, max_buffer, minor_version, &am) == ERROR){
             syslog(LOG_ERR, "Error en HTTP Response OPTIONS.");
              free(cleanpath);
              free(args);
              return ERROR;
          }
          free(args);
          free(cleanpath);
          return OK;
        }
      }
      if(error_response(server_signature, clientsock, cleanpath, 405, minor_version, max_buffer) == ERROR){
          free(cleanpath);
          free(args);
          return ERROR;
      }	
    }
    else{
      if(error_response(server_signature, clientsock, cleanpath, 501, minor_version, max_buffer) == ERROR){
        syslog(LOG_ERR, "Error en HTTP Response ERROR");
        free(cleanpath);
        return ERROR;
      }
    }

    free(args);
    free(cleanpath);
    return OK;
}



/*Funcion que responde a un get*/

int get_response(char* server_signature, int clientsock, char* direc, char* cleanpath, char* args, int max_buffer, int minor_version){
  int f;
  int length;
  int scriptflag = NO_SCRIPT;
  char* date, *modDate, *ext;
  char outBufferAux[max_buffer];
  char outBuffer[max_buffer];
  struct stat fStat;
  struct timeval timeOut;
  char command[COMMAND_SIZE];
  FILE* pipe;

  memset(outBuffer,0,max_buffer);


  f = open(direc, O_RDONLY);
  if(f < 0){
    error_response(server_signature, clientsock, cleanpath, 404, minor_version, max_buffer);
    return OK;
  }

  if(fstat(f, &fStat) < 0){ 
    close(f);
    return ERROR;
  }

  timeOut.tv_sec = (time_t) TIMEOUT;
  /*Rellenamos campos necesarios para crear la respuesta*/

  length = fStat.st_size;
  date = get_date();
  modDate = get_mod_time(&fStat); 
  ext = filename_ext(direc);
  
  /*Cuando detectamos que el fichero a procesar se trata de un script, asignamos a la flag el valor correspondiente
    y, como la salida de los scrips podria ser html, asignamos ese tipo de mensaje para que el navegador lo pueda interpretar*/
  if(strcmp(ext, "py") == 0){
    scriptflag = PYTHON_SCRIPT;
    ext = "text/html";
  }
  else if (strcmp(ext, "php") == 0){
    scriptflag = PHP_SCRIPT;
    ext = "text/html";
  }
  else if(strcmp(ext, "") == 0){
    error_response(server_signature, clientsock, cleanpath, 404, minor_version, max_buffer);
    close(f);
    free (date);
    free (modDate);
    return OK;
  }




  if(scriptflag == NO_SCRIPT){
     sprintf(outBuffer, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nLast-Modified: %s\r\nContent-Length: %lu\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n", minor_version, date, server_signature, modDate, length*sizeof(char), ext);

      /*Enviamos cabeceras*/

      if(my_send(clientsock, outBuffer, strlen(outBuffer)*sizeof(char)) < 0){
        syslog(LOG_ERR, "Error enviando.\n");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
      }

      length = max_buffer;
      memset(outBuffer,0,max_buffer);

      /*Enviamos el fichero en trozos de tamaño MAX_BUFFER como maximo*/
      while(length == max_buffer){
        length = read(f, outBuffer, max_buffer);
        if(length < 0){
          syslog(LOG_ERR, "Error leyendo.\n");
          close(f);
          free (date);
          free (modDate);
          return ERROR;
        }
        else if(length > 0){
          if(my_send(clientsock, outBuffer, length) < 0){
            syslog(LOG_ERR, "Error enviando");
            close(f);
            free (date);
            free (modDate);
            return ERROR;
          }
        }

        memset(outBuffer,0,max_buffer);

      }
  } else if (scriptflag == PYTHON_SCRIPT){
      sprintf(command, "python %s \"%s\"", direc, args);
      pipe = popen(command, "r");
      if(pipe == NULL)
        return ERROR;

      length = fread((void *) outBuffer, 1,max_buffer, pipe);
      if(length < 0){
        syslog(LOG_ERR, "Error leyendo.\n");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
      }

      sprintf(outBufferAux, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nLast-Modified: %s\r\nContent-Length: %lu\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n", minor_version, date, server_signature, modDate, (long unsigned int) length, ext);
      
      /*Enviamos cabeceras*/


      if(my_send(clientsock, outBufferAux, strlen(outBufferAux)*sizeof(char)) < 0){
        syslog(LOG_ERR, "Error enviando.\n");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
      }

      /*Enviamos resultado de la ejecución*/

      if(my_send(clientsock, outBuffer, length) < 0){
          syslog(LOG_ERR, "Error enviando");
          close(f);
          free (date);
          free (modDate);
          return ERROR;
      }
      
      
      if(pclose(pipe) == -1){
        syslog(LOG_ERR, "Error en GET PYTHON SCRIPT: Error cerrando pipe");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
      }
  }
  else{ //Caso PHP_SCRIPT 
      sprintf(command, "php %s \"%s\"", direc, args);
      pipe = popen(command, "r");
     
      if(pipe == NULL)
        return ERROR;

      length = fread((void*)outBuffer, 1, max_buffer, pipe);
      if(length < 0){
          close(f);
          free (date);
          free (modDate);
          return ERROR;
        }
         
       sprintf(outBufferAux, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nLast-Modified: %s\r\nContent-Length: %lu\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n", minor_version, date, server_signature, modDate, (long unsigned int) length, ext);

      /*Enviamos cabeceras*/

      if(my_send(clientsock, outBufferAux, strlen(outBufferAux)*sizeof(char)) < 0){
        syslog(LOG_ERR, "Error enviando.\n");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
      }

      /*Enviamos resultado de la ejecución*/

      if(my_send(clientsock, outBuffer, length) < 0){
            syslog(LOG_ERR,"Error enviando");
            close(f);
            free (date);
            free (modDate);
            return ERROR;
      }
  
      if(pclose(pipe) == -1){
        syslog(LOG_ERR, "Error en GET PYTHON SCRIPT: Error cerrando pipe");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
      }
  }

  close(f);
  free (date);
  free (modDate);
  return OK;
}


/*Funcion que responde a un post*/
/*En nuestro caso solo sirve para ejecutar scripts*/

int post_response(char* server_signature, int clientsock, char* direc, char* cleanpath, char* body, char* args_url, int max_buffer, int minor_version){
  int f;
  int length;
  int scriptflag = NO_SCRIPT;
  char* date, *modDate, *ext;
  char outBufferAux[max_buffer];
  char outBuffer[max_buffer];
  struct timeval timeOut;
  fd_set descrSet;
  int timeFlag;
  struct stat fStat;
  char command[COMMAND_SIZE];
  FILE* pipe;
  int pipeDescr;

  memset(outBuffer,0,max_buffer);

  f = open(direc, O_RDONLY);
  if(f < 0){
    error_response(server_signature, clientsock, cleanpath, 404, minor_version, max_buffer);
    return OK;
  }

  if(fstat(f, &fStat) < 0){ 
    close(f);
    return ERROR;
  }

  timeOut.tv_sec = (time_t) TIMEOUT;
  timeOut.tv_usec = 0;
  /*Rellenamos campos necesarios para crear la respuesta*/

  date = get_date();
  modDate = get_mod_time(&fStat); 
  ext = filename_ext(direc);
  
  /*Cuando detectamos que el fichero a procesar se trata de un script, asignamos a la flag el valor correspondiente
    y, como la salida de los scrips podria ser html, asignamos ese tipo de mensaje para que el navegador lo pueda interpretar*/
  if(strcmp(ext, "py") == 0){
    scriptflag = PYTHON_SCRIPT;
    ext = "text/html";
  }
  else if (strcmp(ext, "php") == 0){
    scriptflag = PHP_SCRIPT;
    ext = "text/html";
  }
  else if(strcmp(ext, "") == 0){
    error_response(server_signature, clientsock, cleanpath, 404, minor_version, max_buffer);
    close(f);
    free (date);
    free (modDate);
    return OK;
  }

  if(scriptflag == NO_SCRIPT){
    error_response(server_signature, clientsock, cleanpath, 405, minor_version, max_buffer);
    close(f);
    free (date);
    free (modDate);
    return OK;
  } 
  else if (scriptflag == PYTHON_SCRIPT){

      sprintf(command, "echo \"%s\" | python %s \"%s\"", body, direc, args_url);
      pipe = popen(command, "r");
      if(pipe == NULL)
        return ERROR;

      pipeDescr = fileno(pipe);
      FD_ZERO(&descrSet);
      FD_SET(pipeDescr, &descrSet);

      timeFlag = select(pipeDescr + 1, &descrSet, NULL, NULL, &timeOut);
      if(timeFlag == -1){
          /*Error*/
          syslog(LOG_ERR,"POST-RESPONSE|PYTHON_SCRIPT: Error en select.\n");
          close(f);
          pclose(pipe);
          free (date);
          free (modDate);
          return ERROR;
      }
      else if(timeFlag == 0){
          syslog(LOG_ERR,"POST-RESPONSE|PYTHON_SCRIPT: Timeout de ejecucion excedido.\n");
      }
      else{
          /*Se ha escrito en el fichero antes del timeout*/
          length = fread(outBuffer, 1,max_buffer, pipe);
        
          if(length < 0){
            syslog(LOG_ERR,"Error leyendo.\n");
            close(f);
            pclose(pipe);
            free (date);
            free (modDate);
            return ERROR;
          }
          sprintf(outBufferAux, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nLast-Modified: %s\r\nContent-Length: %lu\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n", minor_version, date, server_signature, modDate, (long unsigned int) length, ext);
          
          /*Enviamos cabeceras*/
          if(my_send(clientsock, outBufferAux, strlen(outBufferAux)*sizeof(char)) < 0){
            syslog(LOG_ERR,"Error enviando.\n");
            close(f);
            pclose(pipe);
            free (date);
            free (modDate);
            return ERROR;
          }
          
          /*Enviamos resultado de la ejecución*/
          if(my_send(clientsock, outBuffer, length) < 0){
            syslog(LOG_ERR,"Error enviando");
            close(f);
            pclose(pipe);
            free (date);
            free (modDate);
            return ERROR;
          }
      }

      
      if(pclose(pipe) == -1){
        syslog(LOG_ERR, "Error en POST PYTHON SCRIPT: Error cerrando pipe");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
      }
  }
  else{ //Caso PHP_SCRIPT 
      sprintf(command, "echo \"%s\" | php %s \"%s\"", body, direc, args_url);
      pipe = popen(command, "r");
      if(pipe == NULL)
        return ERROR;

      pipeDescr = fileno(pipe);
      FD_ZERO(&descrSet);
      FD_SET(pipeDescr, &descrSet);

      timeFlag = select(pipeDescr+1, &descrSet, NULL, NULL, &timeOut);
      if(timeFlag == -1){
          /*Error*/
          syslog(LOG_ERR,"POST-RESPONSE|PHP_SCRIPT: Error en select.\n");
          close(f);
          pclose(pipe);
          free (date);
          free (modDate);
          return ERROR;
      }
      else if(timeFlag == 0){
          syslog(LOG_ERR,"POST-RESPONSE|PHP_SCRIPT: Timeout de ejecucion excedido.\n");
      }
      else{
      length = fread(outBuffer, 1, max_buffer, pipe);
        if(length < 0){
          syslog(LOG_ERR,"Error leyendo.\n");
          pclose(pipe);
          close(f);
          free (date);
          free (modDate);
          return ERROR;
          }
       
      sprintf(outBufferAux, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nLast-Modified: %s\r\nContent-Length: %lu\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n", minor_version, date, server_signature, modDate, (long unsigned int) length, ext);
      /*Enviamos cabeceras*/
      if(my_send(clientsock, outBufferAux, strlen(outBufferAux)*sizeof(char)) < 0){
        syslog(LOG_ERR,"Error enviando.\n");
        close(f);
        pclose(pipe);
        free (date);
        free (modDate);
        return ERROR;
      }
      /*Enviamos resultado de la ejecución*/
      if(my_send(clientsock, outBuffer, length) < 0){
        syslog(LOG_ERR,"Error enviando");
        close(f);
        pclose(pipe);
        free (date);
        free (modDate);
        return ERROR;
      }
    }

      if(pclose(pipe) == -1){
        syslog(LOG_ERR, "Error en POST PYTHON SCRIPT: Error cerrando pipe");
        close(f);
        pclose(pipe);
        free (date);
        free (modDate);
        return ERROR;
      }
  }

  close(f);
  free (date);
  free (modDate);
  return OK;
}




int options_response(char* server_signature, int clientsock, int buffer_size, int minor_version, allowedMethods* am){
  char* date;
  char outBuffer[buffer_size];

  date = get_date();
  sprintf(outBuffer, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nAllow: %s\r\nContent-Length: 0\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\n", minor_version, date, server_signature, am->txtChain);
  if(my_send(clientsock, outBuffer, strlen(outBuffer)*sizeof(char)) < 0){
    syslog(LOG_ERR,"Error enviando.\n");
    free(date);
    return ERROR;
  }
  free(date);
  return OK;
}
/*Funcion que da respuesta en caso de error*/

int error_response(char* server_signature, int clientsock, char* cleanpath, int errnum, int minor_version, int buf_size){
  char htmlCode[1000];
  char outBuffer[buf_size];
  char* date;
  date = get_date();
    switch (errnum){
        
        /*Caso Metodo No Implementado*/
        case 501:
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>501 Method Not Implemented</title>\n</head><body>\n<h1>Method Not Implemented</h1>\n<p>Only GET,POST,OPTIONS<br />\n</p>\n</body></html>");
          
          sprintf(outBuffer, "HTTP/1.%d 501 Method Not Implemented\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
          break;

        /*Caso archivo No Encontrado*/
        case 404:
          //sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\n<p>The requested URL %s was not found on this server</p>\n</body> <img src=\"https://t00.deviantart.net/cjaQeweEbpdyNDQ_P6tmBz2x0Mo=/fit-in/700x350/filters:fixed_height(100,100):origin()/pre00/d98e/th/pre/i/2016/162/a/3/dead_link_by_mr_sage-d78unho.png\"></html>", cleanpath);
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html lang = \"en\"><head>\n<title>%s could not be found</title>\n</head><body>\n<h1>404 RESOURCE NOT FOUND</h1>\n<p><img src = \"https://pre00.deviantart.net/d98e/th/pre/i/2016/162/a/3/dead_link_by_mr_sage-d78unho.png\"alt = \"Link not found\"/></p>\n</body></html>", cleanpath);


          sprintf(outBuffer, "HTTP/1.%d 404 Not Found\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
          break;

        case 400:
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>400 Bad Request</title>\n</head><body>\n<h1>Bad Request</h1>\n<p>The server could not understand the request due to invalid syntax.</p>\n</body></html>");
          
          sprintf(outBuffer, "HTTP/1.%d 400 Bad Request\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
          break;

        case 405:
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>405 Not allowed</title>\n</head><body>\n<h1>Not allowed</h1>\n<p>The method you are trying to use is not allowed in here.</p>\n</body></html>");
          
          sprintf(outBuffer, "HTTP/1.%d 405 Not Allowed\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
          break;

        default:
          printf("Error no implementado.\n");
    
    }
    if(my_send(clientsock, outBuffer, strlen(outBuffer)) < 0){
      free(date);
      syslog(LOG_ERR,"Error enviando");
      return ERROR;
    }
    free(date);
    return OK;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include "http_processing.h"
#include "picohttpparser.h"
#include "socket_management.h"

#define DIREC_SIZE 100

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

/*Funcion que devuelve el Content-Type de un fichero*/

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
      else if(strcmp(dot+1, "py") == 0 || strcmp(dot+1, "php") == 0)
        return "scripts"; //Habrá que cambiarlo
      else
        return "";
    } 
    return "";
}


/*Funcion que devuelve los metodos validos en cada carpeta. 
  Retorno: Devuelve los datos que nos interesan dentro la estructura*/
int allowed_methods(allowedMethods* met, char* path, int path_len){
  if( (met == NULL) || (path == NULL))
    return ERROR;

  char delim = '/';
  char* aux;
  char * token1;
  char * token2;


  aux = (char *) malloc ((path_len +1)* sizeof(char));
  sprintf(aux, "%.*s",  path_len, path);

  if((strcmp(aux, "/*") == 0) || (strcmp(aux, "*") == 0)){
    met->nummethods = 3;
    strcpy(met->methods[0],"GET");
    strcpy(met->methods[1], "OPTIONS");
    strcpy(met->methods[2], "POST");
    strcpy(met->txtChain, "GET,OPTIONS,POST");
    free(aux);
    return OK;
  }

  //TODO: RESOLVER CUANDO STRTOK DA NULL, NO DEVOLVEMOS NADA, ESTO DEBERIA DAR EL CASO DEFAULT, NO SEGFAULT
  FILE* f;
  f = fopen("debbuging.txt", "w");
  token1=strtok(aux, &delim);
  if(token1 == NULL){
    free(aux);
    return ERROR;
  }
  token2=strtok(NULL, &delim);
  fprintf(f, "%s\n", path);
  fprintf(f, "%s\n", aux);
  fprintf(f, "%s\n",token1);
  fprintf(f, "%s\n",token2);
  fclose(f);

  if((strcmp(token1, "files") == 0) && (strcmp(token2, "docs") == 0)){
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
  else if((strcmp(token1, "files") == 0)){
    met->nummethods = 3;
    strcpy(met->methods[0],"GET");
    strcpy(met->methods[1], "OPTIONS");
    strcpy(met->methods[2], "POST");
    strcpy(met->txtChain, "GET,OPTIONS,POST");
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

int parse_petition(int csock, char* inBuffer, char* outBuffer, char* signature, char* root, long int buf_size){
    char* server_signature = NULL;
    char direc[DIREC_SIZE];
    char* cleanpath;
    int clientsock;
    int max_buffer;
    int i;
    char *method, *path;
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
        if(error_response(server_signature, clientsock, path, outBuffer, 400, minor_version) == ERROR)
          return ERROR;
        return OK;
        }

    if(allowed_methods(&am, path, path_len) == ERROR){
        FILE* f;
        f = fopen("stilldebugging.txt", "w");
        fprintf(f, "es aqui donde el allowed\n" );
        fclose(f);
        if(error_response(server_signature, clientsock, path, outBuffer, 400, minor_version) == ERROR)
          return ERROR;
        return OK;
    }

    /*Guardamos el método*/
    char aux[20];
    sprintf(aux, "%.*s", (int)method_len, method);


    /*Guardamos la ruta del fichero concatenando con server_root*/
    cleanpath = (char*) malloc ((path_len+1)* sizeof(char));
    sprintf(cleanpath, "%.*s", (int)path_len, path);
    sprintf(direc, "%s%s", root, cleanpath);


  	if(strcmp(aux, "GET") == 0){
      for(i=0; i < am.nummethods; i++){
        if(strcmp(am.methods[i], "GET") == 0){
          if(get_response(server_signature, clientsock, direc, cleanpath, max_buffer, outBuffer, minor_version) == ERROR){
            perror("Error en HTTP Response GET.");
            free(cleanpath);
            return ERROR;
          }
          free(cleanpath);
          return OK;
        }
      }
      if(error_response(server_signature, clientsock, cleanpath, outBuffer, 405, minor_version) == ERROR){
          free(cleanpath);
          return ERROR;
        }
      free(cleanpath);
      return OK;
  	}

    else if(strcmp(aux, "POST") == 0){
      response_petition(inBuffer, outBuffer); // Respuesta genérica
    }
  	
  	else if (strcmp(aux, "OPTIONS") == 0){
      if(options_response(server_signature, clientsock,outBuffer, minor_version, &am) == ERROR){
        perror("Error en HTTP Response GET.");
        free(cleanpath);
        return ERROR;
      }  	
    }

    else{
      if(error_response(server_signature, clientsock, cleanpath, outBuffer, 501, minor_version) == ERROR){
        perror("Error en HTTP Response ERROR");
        free(cleanpath);
        return ERROR;
      }
    }

    free(cleanpath);
    return OK;
}



/*Funcion que responde a un get*/

int get_response(char* server_signature, int clientsock, char* direc, char* cleanpath, int max_buffer, char* outBuffer, int minor_version){
  int f;
  int length;
  char* date, *modDate, *ext;
  struct stat fStat;

  f = open(direc, O_RDONLY);
  if(f < 0){
    error_response(server_signature, clientsock, cleanpath, outBuffer, 404, minor_version);
    return OK;
  }

  if(fstat(f, &fStat) < 0){ 
    close(f);
    return ERROR;
  }

  /*Rellenamos campos necesarios para crear la respuesta*/

  length = fStat.st_size;
  date = get_date();
  modDate = get_mod_time(&fStat); 
  ext = filename_ext(direc);

  if(strcmp(ext, "") == 0){
    error_response(server_signature, clientsock, cleanpath, outBuffer, 404, minor_version);
    close(f);
    free (date);
    free (modDate);
    return OK;
  }

  sprintf(outBuffer, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nLast-Modified: %s\r\nContent-Length: %lu\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n", minor_version, date, server_signature, modDate, length*sizeof(char), ext);

  /*Enviamos cabeceras*/

  if(my_send(clientsock, outBuffer, strlen(outBuffer)*sizeof(char)) < 0){
    perror("Error enviando.\n");
    close(f);
    free (date);
    free (modDate);
    return ERROR;
  }

  length = max_buffer;

  /*Enviamos el fichero en trozos de tamaño MAX_BUFFER como maximo*/

  while(length == max_buffer){
    length = read(f, outBuffer, max_buffer);
    if(length < 0){
      perror("Error leyendo.\n");
      close(f);
      free (date);
      free (modDate);
      return ERROR;
    }
    else if(length > 0){
      if(my_send(clientsock, outBuffer, length) < 0){
        perror("Error enviando");
          close(f);
          free (date);
          free (modDate);
        return ERROR;
      }
    }
  }

  close(f);
  free (date);
  free (modDate);
  return OK;
}


/*Funcion que responde a un post*/
/*En nuestro caso solo sirve para ejecutar scripts*/

int post_response(char* server_signature, int clientsock, char* direc, char* outBuffer, int minor_version){
  return OK;
}


int options_response(char* server_signature, int clientsock, char* outBuffer, int minor_version, allowedMethods* am){
  char* date;

  date = get_date();
  sprintf(outBuffer, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nAllow: %s\r\nContent-Length: 0\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\n", minor_version, date, server_signature, am->txtChain);
  if(my_send(clientsock, outBuffer, strlen(outBuffer)*sizeof(char)) < 0){
    perror("Error enviando.\n");
    free(date);
    return ERROR;
  }
  free(date);
  return OK;
}
/*Funcion que da respuesta en caso de error*/

int error_response(char* server_signature, int clientsock, char* cleanpath, char* outBuffer, int errnum, int minor_version){
  char htmlCode[1000];
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
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\n<p>The requested URL %s was not found on this server</p>\n</body></html>", cleanpath);
          
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
      perror("Error enviando");
      return ERROR;
    }
    free(date);
    return OK;
}

/*Funcion que habra que cambiar ya que responde siempre lo mismo I think*/

int response_petition(char* inBuffer, char* outBuffer){
    
    sprintf(outBuffer, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: 88\r\n\r\n<html>\n<body>\n<h1>Happy New Millennium!</h1>\n</body>\n</html>\r\n");
    
    return 0;
}


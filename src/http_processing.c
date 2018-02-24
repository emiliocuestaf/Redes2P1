#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <error.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "http_processing.h"
#include "picohttpparser.h"

#define MAX_BODY 10000

static char* server_signature = NULL;
static char direc[100];

/*Funcion que genera la fecha actual*/

char* get_date(){
  char *buf = malloc(sizeof(char)*35);
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  if(buf == NULL)
  	return NULL;
  strftime(buf, 35, "%a, %d %b %Y %H:%M:%S %Z", &tm);
  printf("DATE: %s", buf);
  return buf;
}

/*Funcion que devuelve la fecha de la ultima modificacion*/

char* get_mod_time(char *path) {
    struct stat attrib;
    stat(path, &attrib);
    char* buf = malloc(sizeof(char)*35);
    strftime(buf, 35, "%a, %d %b %y %H:%M:%S %Z", localtime(&(attrib.st_ctime)));
    printf("\nThe file %s was last modified at %s\n", path, buf);
    return buf;
}

/*Funcion que da respuesta en caso de error*/

int error_response(char* outBuffer, int errnum, int minor_version){
	char htmlCode[10000];
	char* date;
	date = get_date();
    switch (errnum){
        
        /*Caso Metodo No Implementado*/
        case 501:
        	sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>501 Method Not Implemented</title>\n</head><body>\n<h1>Method Not Implemented</h1>\n<p>Only GET,POST,OPTIONS<br />\n</p>\n</body></html>>");
        	
        	sprintf(outBuffer, "HTTP/1.%d 501 Method Not Implemented\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
            break;

        /*Caso archivo No Encontrado*/
        case 404:
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\n<p>The requested URL %s was not found on this server</p>\n</body></html>>", direc);
          
          sprintf(outBuffer, "HTTP/1.%d 404 Not Found\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
            break;
            
       	default:
       		printf("Error no implementado.\n");
    
    }
    free(date);
    return 0;
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
      else
        return "";
    } 
    return "";
}

/*Funcion que responde a un get*/

int response_get(char* outBuffer, int minor_version){
  FILE* f = NULL;
  long length;
  char* body;
  char* date;
  char* modDate;
  f = fopen("c3.jpg", "rb");
  if(f == NULL){
    error_response(outBuffer, 404, minor_version);
    return 0;
  }

  fseek (f, 0, SEEK_END);
  length = ftell (f);
  rewind(f);
  body = malloc ((length+1) *sizeof(char));

  if(body != NULL){
    printf("ENTROOOO SUUU\n");
    fread(body, sizeof(char), length, f);
    body[length] = 0;
  }



  printf("\nAQUI VA MI BODY: %s\n", body);

  date = get_date();
  modDate = get_mod_time(direc); 

  sprintf(outBuffer, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nLast-Modified: %s\r\nContent-Length: %lu\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n%s\r\n", minor_version, date, server_signature, modDate, sizeof(char)*strlen(body), filename_ext(direc),body);

  fclose(f);
  free (date);
  free (modDate);
  free(body);
  return 0;
}

/*Funcion que parsea una peticion HTTP*/

int parse_petition(char* inBuffer, char* outBuffer, char* signature, char* root){
    char *method, *path;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;
    unsigned int i;
    
    server_signature = signature;

    
    num_headers = sizeof(headers) / sizeof(headers[0]);
    
    /*LLamamos a la funcion que se encarga de parsear la peticion*/
    pret = phr_parse_request(inBuffer, (ssize_t) strlen(inBuffer), (const char**)&method, &method_len,(const char**) &path, &path_len, &minor_version, headers, &num_headers, (size_t) 0);
    
    if ((pret == -1) || (strlen(inBuffer) == sizeof(inBuffer))){
        printf("Error parseando HTTP.\n");
        return -1;
        }
    

    printf("\n\nrequest is %d bytes long\n", pret);
    printf("method is %.*s\n", (int)method_len, method);
    printf("path is %.*s\n", (int)path_len, path);
    printf("HTTP version is 1.%d\n", minor_version);
    printf("headers:\n");
    
    for (i = 0; i != num_headers; ++i) {
        printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
               (int)headers[i].value_len, headers[i].value);
    }

    /*Guardamos el método*/
    char aux[20];
    sprintf(aux, "%.*s", (int)method_len, method);

    /*Guardamos la ruta del fichero*/
    sprintf(direc, "%s%.*s", root, (int)path_len, path);

	if(strcmp(aux, "GET") == 0){
	    response_get(outBuffer, minor_version);
	}
  else if(strcmp(aux, "POST") == 0){
    response_petition(inBuffer, outBuffer); // Respuesta genérica
  }
	
	else if (strcmp(aux, "OPTIONS") == 0){
		response_petition(inBuffer, outBuffer); // Respuesta genérica
	}

  else{
    error_response(outBuffer, 501, minor_version);
  }
	
	printf("%s", outBuffer);
    
    return 0;
    
}
/*
GET /path/file.html HTTP/1.0\r\nFrom: someuser@jmarshall.com\r\nUser-Agent: HTTPTool/1.0\r\n\r\n

*/

/*Funcion que habra que cambiar ya que responde siempre lo mismo I think*/

int response_petition(char* inBuffer, char* outBuffer){
    
    sprintf(outBuffer, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: 88\r\n\r\n<html>\n<body>\n<h1>Happy New Millennium!</h1>\n</body>\n</html>\r\n");
    
    return 0;
}


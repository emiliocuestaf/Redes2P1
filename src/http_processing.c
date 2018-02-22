#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "http_processing.h"
#include "picohttpparser.h"

static char* server_signature = NULL;

/*Funcion que genera la fecha actual*/

char* get_date(){
  char *buf = malloc(sizeof(char)*1000);
  time_t now = time(0);
  struct tm tm = *gmtime(&now);
  if(buf == NULL)
  	return NULL;
  strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
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
        	
        	sprintf(outBuffer, "HTTP/1.%d 405 Method Not Implemented\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
            break;
            
       	default:
       		printf("Error no implementado.\n");
    
    }
    free(date);
    return 0;
}

/*Funcion que parsea una peticion HTTP*/

int parse_petition(char* inBuffer, char* outBuffer, char* signature){
    char *method, *path;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;
    unsigned int i;
    
    server_signature = signature;

    
    num_headers = sizeof(headers) / sizeof(headers[0]);
    
    /*LLamamos a la funcion que se encarga de parsear la peticion*/
    pret = phr_parse_request(inBuffer, (ssize_t) strlen(inBuffer), (const char**)&method, &method_len,(const char**) &path, &path_len, &minor_version, headers, &num_headers, (size_t) 0);
    
    if ((pret == -1) /*|| (strlen(inBuffer) == sizeof(inBuffer))*/){
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
	printf("\n");
	
	if(strcmp(method, "GET") != 0 || strcmp(method, "POST") != 0 || strcmp(method, "OPTIONS") != 0){
	    error_response(outBuffer, 501, minor_version);
	
	}
	
	else{
		response_petition(inBuffer, outBuffer);
	}
	
	printf("%s", outBuffer);
    
    return 0;
    
}
/*
GET /path/file.html HTTP/1.0\r\nFrom: someuser@jmarshall.com\r\nUser-Agent: HTTPTool/1.0\r\n\r\n

*/

/*Funcion que habra que cambiar ya que responde siempre lo mismo I think*/

int response_petition(char* inBuffer, char* outBuffer){
    int minor_version;
    int status;
    const char *msg;
    size_t msg_len;
    struct phr_header headers[4];
    size_t num_headers;

	unsigned int i;
    
    sprintf(outBuffer, "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\nContent-Length: 88\r\n\r\n<html>\n<body>\n<h1>Happy New Millennium!</h1>\n</body>\n</html>\r\n");
    
    phr_parse_response(outBuffer, strlen(outBuffer), &minor_version, &status, &msg, &msg_len, headers, &num_headers, 0);

	for (i = 0; i != num_headers; ++i) {
        printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
               (int)headers[i].value_len, headers[i].value);
    }

	printf("%s", msg);
    return 0;
}


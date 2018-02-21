#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_processing.h"
#include "picohttpparser.h"



int parse_petition(char* inBuffer, char* outBuffer){
    char *method, *path;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;
    unsigned int i;

    
    num_headers = sizeof(headers) / sizeof(headers[0]);
    
    /*LLamamos a la funcion que se encarga de parsear la peticion*/
    
    printf("%s\n", inBuffer);
    pret = phr_parse_request(inBuffer, (ssize_t) strlen(inBuffer), (const char**)&method, &method_len,(const char**) &path, &path_len, &minor_version, headers, &num_headers, (size_t) 0);
    
    if ((pret == -1) || (strlen(inBuffer) == sizeof(inBuffer))){
        printf("Error parseando HTTP.\n");
        return -1;
        }
    

    printf("request is %d bytes long\n", pret);
    printf("method is %.*s\n", (int)method_len, method);
    printf("path is %.*s\n", (int)path_len, path);
    printf("HTTP version is 1.%d\n", minor_version);
    printf("headers:\n");
    
    for (i = 0; i != num_headers; ++i) {
        printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
               (int)headers[i].value_len, headers[i].value);
    }
    
    return 0;
    
}

/*Funcion que habra que cambiar ya que responde siempre lo mismo I think*/

int response_petition(char* inBuffer, char* outBuffer){
    int minor_version;
    int status;
    const char *msg;
    size_t msg_len;
    struct phr_header headers[4];
    size_t num_headers;
     
    sprintf(outBuffer, "HTTP/1.0 200 OK\r\n\r\n");
    
    phr_parse_response(outBuffer, strlen(outBuffer), &minor_version, &status, &msg, &msg_len, headers, &num_headers, 0);
    
    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_processing.h"
#include "picohttpparser.h"

/*Funcion que parsea una peticion HTTP*/

int parse_petition(char* inBuffer, char* outBuffer){
    char *method, *path;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;
    unsigned int i;

    
    num_headers = sizeof(headers) / sizeof(headers[0]);
    
    /*LLamamos a la funcion que se encarga de parsear la peticion*/
    
    printf("A VER QUE VIENE:%s\n", inBuffer);
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


#include "handlers.h"


//Funciones publicas
int handler(char* bufferIn, char* bufferOut);

//Funciones privadas
int get_handler(char*bufferIn, char* bufferOut);
int post_handler(char*bufferIn, char* bufferOut);
int option_handler(char*bufferIn, char* bufferOut);
int php_script_handler(char*bufferIn, char* bufferOut);
int py_script_handler(char*bufferIn, char* bufferOut);
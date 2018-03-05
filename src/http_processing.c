/*******************************************************
* PRÁCTICAS DE REDES 2
* Practica 1
* Autores:
*   -Luis Carabe Fernandez-Pedraza
* -Emilio Cuesta Fernandez
* Descripcion:
* Modulo que se encarga de procesar una peticion HTTP y responderla
********************************************************/
#include "http_processing.h"


/********
* FUNCIÓN:  char* get_date()
* DESCRIPCIÓN: Funcion que genera la fecha actual
* ARGS_OUT: char* - la fecha actual, NULL en caso de error
********/

char* get_date(){
    /*Reservamos memoria e inicializamos lo necesario*/
    char *buf = malloc(sizeof(char)*35);
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    if(buf == NULL) // En caso de mala reserva
      	return NULL;
    /*Copiamos la fecha con el fotmato adecuado y devolvemos*/
    strftime(buf, 35, "%a, %d %b %Y %H:%M:%S %Z", &tm);
    return buf;
}

/********
* FUNCIÓN:  get_mod_time(struct stat* fStat)
* ARGS_IN: struct stat* fStat - estructura que contiene informacion del fichero
* DESCRIPCIÓN: Funcion que devuelve la fecha de la ultima modificacion de un fichero
* ARGS_OUT: char* - la fecha modificada, NULL en caso de error
********/

char* get_mod_time(struct stat* fStat) {
    char* buf = malloc(sizeof(char)*35);
    if (buf == NULL)
      return NULL;
    /*Copiamos la fecha con el fotmato adecuado y devolvemos*/
    strftime(buf, 35, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&(fStat->st_ctime)));
    return buf;
}

/********
* FUNCIÓN:  char *filename_ext(char *fname)
* ARGS_IN: char* fname - nombre del fichero
* DESCRIPCIÓN: Funcion que devuelve el Content-Type de un fichero, a excepcion de los 
*              ficheros .py o .php cuyo retorno es su misma extension
* ARGS_OUT: char* - el Content-Type del fichero, "" si no está soportado
********/

char *filename_ext(char *fname) {
    /*Buscamos el ultimo punto del nombre y guardamos la posicion*/
    char *dot = strrchr(fname, '.');

    /*Si no es null, comparamos con posibles extensiones*/
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

    /*Si no es una extension permitida o no hay un punto en el nombre, devolvemos ""*/

    return "";
}

/********
* FUNCIÓN:  int allowed_methods(allowedMethods* met, char* cleanpath)
* ARGS_IN:  àllowedMethods* met - estructura donde guardaremos los metodos que podemos aplicar en la ruta
            char* cleanpath - ruta que analizamos
* DESCRIPCIÓN: Devuelve los metodos validos en cada carpeta
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int allowed_methods(allowedMethods* met, char* cleanpath){

    /*Comprobamos posibles errores*/

    if((met == NULL) || (cleanpath == NULL))
    return ERROR;

    char delim = '/'; // Delimitador de las carpetas
    char * token1;
    char * token2;
    char* aux;

    /*Rervamos memoria para guardar una copia de la ruta que usaremos para tokenizar*/

    aux = (char *) malloc (strlen(cleanpath)+1);
    strcpy(aux, cleanpath);

    /*Comprobamos si nos están mandando un directorio que incluye a todos (*), para devolver todos los metodos validos*/

    if((strcmp(aux, "/*") == 0) || (strcmp(aux, "*") == 0)){
        met->nummethods = 3;
        strcpy(met->methods[0],"GET");
        strcpy(met->methods[1], "OPTIONS");
        strcpy(met->methods[2], "POST");
        strcpy(met->txtChain, "GET,OPTIONS,POST");
        free(aux);
        return OK;
    }

    /*Tokenizamos, guardando en token1 el primer directorio y en token2 el segundo*/

    token1=strtok(aux, &delim);
    if(token1 == NULL){
        free(aux);
        return ERROR;
    }
    token2=strtok(NULL, &delim);

    /*Ahora miramos que directorio estamos analizando para guardar en met los metodos permitidos*/

    if((strcmp(token1, "files") && (token2 == NULL))){ // Caso /files
        met->nummethods = 3;
        strcpy(met->methods[0],"GET");
        strcpy(met->methods[1], "OPTIONS");
        strcpy(met->methods[2], "POST");
        strcpy(met->txtChain, "GET,OPTIONS,POST");
        free(aux);
        return OK;
    }
    else if((strcmp(token1, "files") == 0) && (strcmp(token2, "docs") == 0)){ // Caso /files/docs
      met->nummethods = 2;
      strcpy(met->methods[0],"GET");
      strcpy(met->methods[1], "OPTIONS");
      strcpy(met->txtChain, "GET,OPTIONS");
      free(aux);
      return OK;
    }
    else if((strcmp(token1, "files") == 0) && (strcmp(token2, "images") == 0)){ // Caso /files/images
      met->nummethods = 2;
      strcpy(met->methods[0],"GET");
      strcpy(met->methods[1], "OPTIONS");
      strcpy(met->txtChain, "GET,OPTIONS");
      free(aux);
      return OK;
    }
    else if((strcmp(token1, "files") == 0) && (strcmp(token2, "scripts") == 0)){ // Caso /files/scripts
      met->nummethods = 3;
      strcpy(met->methods[0],"GET");
      strcpy(met->methods[1], "OPTIONS");
      strcpy(met->methods[2], "POST");
      strcpy(met->txtChain, "GET,OPTIONS,POST");
      free(aux);
      return OK;
    }
    else if((strcmp(token1, "files") == 0) && (strcmp(token2, "videos") == 0)){ // Caso /files/videos
      met->nummethods = 2;
      strcpy(met->methods[0],"GET");
      strcpy(met->methods[1], "OPTIONS");
      strcpy(met->txtChain, "GET,OPTIONS");
      free(aux);
      return OK;
    }
    else if((strcmp(token1, "files") == 0) && (strcmp(token2, "html") == 0)){ // Caso /files/html
      met->nummethods = 2;
      strcpy(met->methods[0],"GET");
      strcpy(met->methods[1], "OPTIONS");
      strcpy(met->txtChain, "GET,OPTIONS");
      free(aux);
      return OK;
    }
    else{ // Cualquier otro caso no cubierto ya
      met->nummethods = 1;
      strcpy(met->methods[0], "OPTIONS");
      strcpy(met->txtChain, "OPTIONS");
      free(aux);
      return OK;
    }

    /*Si hemos llegado hasta aqui, es que ha habido un error*/

    free(aux);
    syslog(LOG_ERR, "Error en HTTP_PROCESSING: Error en allowed methods");
    return ERROR;
}

/********
* FUNCIÓN:  int parse_petition(int csock, char* inBuffer, char* signature, char* root, long int buf_size, long int timeout)
* ARGS_IN:  int csock - descriptor del socket al cual debemos enviar la respuesta
            char* inBuffer - peticion a parsear
            char* signature - nombre del servidor
            char* root - directorio raiz del servidor
            long int buf_size - longitud maxima del buffer de entrada/salida
            long int timeout - timeout maximo del servidor
* DESCRIPCIÓN: Parsea una peticion HTTP y crea la respuesta con funciones auxiliares
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int parse_petition(int csock, char* inBuffer, char* signature, char* root, long int buf_size, long int timeout){
    char direc[DIREC_SIZE];
    char *method, *path, *cleanpath, *args, *auxpath, *qptr, *body;
    int i;
    int pret, minor_version;
    struct phr_header headers[100];
    size_t method_len, path_len, num_headers;
    allowedMethods am;
        
    /*Parseamos la peticion*/

    num_headers = sizeof(headers) / sizeof(headers[0]);
    pret = phr_parse_request(inBuffer, (ssize_t) strlen(inBuffer), (const char**)&method, &method_len,(const char**) &path, &path_len, &minor_version, headers, &num_headers, (size_t) 0);
    
    if ((pret == -1) || (strlen(inBuffer) == sizeof(inBuffer))){
        if(error_response(signature, csock, path, 400, minor_version, buf_size) == ERROR){
            syslog(LOG_ERR, "Error en HTTP Response error.");
            return ERROR;
        }
        return OK;
    }

    /* Guardamos el cuerpo de la petición, sumando el num de bytes de la cabecera (que devuelve phr_parse_request) a la dir inicial de inBuffer*/

    body = inBuffer + pret;

    /*Guardamos el método sin la URL*/

    char met[20];
    sprintf(met, "%.*s", (int)method_len, method);


    /*Para limpiar el path, es necesario distinguir si se pasan argumentos detras de un ? o no,
      aunque luego tengan que ser ignorados si el fichero solicitado no es un script*/

    auxpath = (char*) malloc ((path_len+1)* sizeof(char));
    cleanpath = (char*) malloc ((path_len+1)* sizeof(char));
    args = (char*) malloc ((path_len+1)* sizeof(char));

    sprintf(auxpath, "%.*s", (int)path_len, path); // auxpath contiene la direccion entera

    /*Buscamos si hay ?, en tal caso, qptr apunta a la primera ?*/
    qptr = strchr(auxpath, '?');

    /*Caso en que no hay ?, path_len funciona bien*/
    if(qptr == NULL){
      strcpy(cleanpath, auxpath); //Guardamos la direccion en cleanpath
      free(auxpath);
      strcpy(args,""); //No hay argumentos de entrada
    }
    else{
        /*Caso en que hay ?, con aritmetica de punteros se puede solucionar
        Tambien se guarda la cadena de argumentos, que mas tarde puede ser necesaria */
        int auxiliar;
        auxiliar = (int) (qptr-auxpath) *sizeof(char);
        sprintf(cleanpath, "%.*s", auxiliar, auxpath); // cleanpath contiene la direccion sin los argumentos de script
        auxiliar = (int) ((auxpath+path_len)-qptr) *sizeof(char);
        sprintf(args, "%.*s", auxiliar , qptr+1); //args tiene los argumentos del script
        free(auxpath);
    }

    /*Miramos que metodos estan permitidos en esa rutaa*/

    if(allowed_methods(&am, cleanpath) == ERROR){
        if(error_response(signature, csock, path, 400, minor_version, buf_size) == ERROR){
            free(cleanpath);
            free(args);
            syslog(LOG_ERR, "Error en HTTP Response error.");
            return ERROR;
        }
        return OK;
    }
    /*Guardamos la ruta del fichero concatenando con server_root*/
    sprintf(direc, "%s%s", root, cleanpath);

    /*Ahora comparamos el metodo requerido con cada uno de los posibles*/

  	if(strcmp(met, "GET") == 0){ //El metodo es GET
      for(i=0; i < am.nummethods; i++){
        if(strcmp(am.methods[i], "GET") == 0){ //Comprobamos si es valido usarlo en dicho directorio, si lo es, formamos la respuesta
          if(get_response(signature, csock, direc, cleanpath, args, buf_size, minor_version, timeout) == ERROR){
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
      /*Si no podemos usar el metodo en este directorio, enviamos el error correspondiente*/
    if(error_response(signature, csock, cleanpath, 405, minor_version, buf_size) == ERROR){
        syslog(LOG_ERR, "Error en HTTP Response error.");
        free(cleanpath);
        free(args);
        return ERROR;
      }
  	}
    else if(strcmp(met, "POST") == 0){ //El metodo es POST
      for(i=0; i < am.nummethods; i++){
        if(strcmp(am.methods[i], "POST") == 0){
          if(post_response(signature, csock, direc, cleanpath, body, args, buf_size, minor_version, timeout) == ERROR){
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
      /*Si no podemos usar el metodo en este directorio, enviamos el error correspondiente*/
      if(error_response(signature, csock, cleanpath, 405, minor_version, buf_size) == ERROR){
          syslog(LOG_ERR, "Error en HTTP Response error.");
          free(cleanpath);
          free(args);
          return ERROR;
      } 
    }	
  	else if (strcmp(met, "OPTIONS") == 0){ //El metodo es OPTIONS
      for(i=0; i < am.nummethods; i++){
        if(strcmp(am.methods[i], "OPTIONS") == 0){
          if(options_response(signature, csock, buf_size, minor_version, &am) == ERROR){
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
      /*Si no podemos usar el metodo en este directorio, enviamos el error correspondiente*/
      if(error_response(signature, csock, cleanpath, 405, minor_version, buf_size) == ERROR){
          syslog(LOG_ERR, "Error en HTTP Response error.");
          free(cleanpath);
          free(args);
          return ERROR;
      }	
    }
    else{ /*Método no implementado*/
      if(error_response(signature, csock, cleanpath, 501, minor_version, buf_size) == ERROR){
        syslog(LOG_ERR, "Error en HTTP Response ERROR");
        free(args);
        free(cleanpath);
        return ERROR;
      }
    }

    /*No deberiamos llegar hasta aqui nunca*/
    free(args);
    free(cleanpath);
    return OK;
}

/********
* FUNCIÓN:  int get_response(char* server_signature, int csock, char* direc, char* cleanpath, char* args, int max_buffer, int minor_version, long int timeout)
* ARGS_IN:  char* server_signature - nombre del servidor
            int csock - descriptor del socket al cual debemos enviar la respuesta
            char* direc - direccion del archivo completa
            char* cleanpath - archivo del fichero (sin aniadir el directorio raiz)
            char* args - argumentos del script
            long int max_buffer - longitud maxima del buffer de entrada/salida
            int minor_version - version de HTTP (1.0 o 1.1)
            long int timeout - timeout maximo del servidor
* DESCRIPCIÓN: responde a una peticion GET
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int get_response(char* server_signature, int csock, char* direc, char* cleanpath, char* args, int max_buffer, int minor_version, long int timeout){
  int f;
  int length;
  int scriptflag = NO_SCRIPT;
  char* date, *modDate, *ext;
  char outBufferAux[max_buffer];
  char outBuffer[max_buffer];
  struct stat fStat;
  struct timeval timeOut;
  fd_set descrSet;
  int timeFlag;
  char command[COMMAND_SIZE];
  FILE* pipe;
  int pipeDescr;

  /*Limpiamos outBuffer (lo rellenamos de ceros)*/
  memset(outBuffer,0,max_buffer);

  /*Abrimos el fichero en modo lectura*/
  f = open(direc, O_RDONLY);
  if(f < 0){ //Si no se abre, enviamos error 404
    if(error_response(server_signature, csock, cleanpath, 404, minor_version, max_buffer) == ERROR){
        syslog(LOG_ERR, "Error en HTTP Response ERROR");
        return ERROR;
    }
    return OK;
  }

  /*Ahora vamos a consultar la informacion del archivo abierto*/

  if(fstat(f, &fStat) < 0){ 
    close(f);
    syslog(LOG_ERR, "Error en HTTP response GET");
    return ERROR;
  }

  /*Rellenamos estructura para controlar timeout de ejecucion*/

  timeOut.tv_sec = (time_t) timeout;
  timeOut.tv_usec = 0;

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
  /*Si la extension no esta soportada, devolvemos error*/
  else if(strcmp(ext, "") == 0){
    if(error_response(server_signature, csock, cleanpath, 404, minor_version, max_buffer) == ERROR){
        syslog(LOG_ERR, "Error en HTTP Response ERROR");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
    }
    close(f);
    free (date);
    free (modDate);
    return OK;
  }

  /*Procesamos la peticion dependiendo de si enviamos un archivo o ejecutamos un script*/

  if(scriptflag == NO_SCRIPT){
     sprintf(outBuffer, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nLast-Modified: %s\r\nContent-Length: %lu\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n", minor_version, date, server_signature, modDate, length*sizeof(char), ext);

      /*Enviamos cabeceras*/

      if(my_send(csock, outBuffer, strlen(outBuffer)*sizeof(char)) < 0){
        syslog(LOG_ERR, "Error enviando.\n");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
      }
      /*Nos preparamos para ir leyendo el fichero en un bucle, para esto, reseteamos el contenido de outBuffer*/

      length = max_buffer;
      memset(outBuffer,0,max_buffer);

      /*Enviamos el fichero en trozos de tamaño MAX_BUFFER como maximo*/
      while(length == max_buffer){
        length = read(f, outBuffer, max_buffer);
        /*Si length ha leido el maximo, significa que continuamos en el bucle*/
        if(length < 0){
          syslog(LOG_ERR, "Error leyendo.\n");
          close(f);
          free (date);
          free (modDate);
          return ERROR;
        }
        else if(length > 0){
          if(my_send(csock, outBuffer, length) < 0){
            syslog(LOG_ERR, "Error enviando");
            close(f);
            free (date);
            free (modDate);
            return ERROR;
          }
        }
        /*Borramos contenido de outBuffer*/
        memset(outBuffer,0,max_buffer);

      }
  } 
  /*Caso de script .py*/
  else if (scriptflag == PYTHON_SCRIPT){
    /*Creamos el comando a ejecutar y lo guardamos en command*/
    sprintf(command, "python %s \"%s\"", direc, args);

    /*Abrimos una tuberia en modo lectura para enviar el comando y que se ejecute*/
    pipe = popen(command, "r");
    if(pipe == NULL){
        syslog(LOG_ERR,"Error creando la tuberia\n");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
    }

    /*Llamamos a funciones necesarias para usar select, que se encargara del timeout*/

    pipeDescr = fileno(pipe);
    FD_ZERO(&descrSet);
    FD_SET(pipeDescr, &descrSet);
    timeFlag = select(pipeDescr + 1, &descrSet, NULL, NULL, &timeOut);
    if(timeFlag == -1){
        //Error en el select, no se puede comprobar timeout, se devuelve error
        syslog(LOG_ERR,"GET-RESPONSE|PYTHON_SCRIPT: Error en select.\n");
        close(f);
        pclose(pipe);
        free (date);
        free (modDate);
        return ERROR;
    }
    else if(timeFlag == 0){ 
        //Se excede el timeout. Se devuelve un error de timeout.
        syslog(LOG_ERR,"GET-RESPONSE|PYTHON_SCRIPT: Timeout de ejecucion excedido.\n");
        if(error_response(server_signature, csock, cleanpath, 408, minor_version, max_buffer) == ERROR){
            syslog(LOG_ERR, "Error en HTTP Response ERROR");
            close(f);
            pclose(pipe);
            free (date);
            free (modDate);
            return ERROR;
        }
        close(f);
        pclose(pipe);
        free (date);
        free (modDate);
        return ERROR;
    }
    else{
        //La ejecucion ha finalizado antes de que se excediera el timeout

        /*Leemos el contenido de la tuberia(resultado de la ejecucion) y lo guardamos en outBuffer*/

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


        if(my_send(csock, outBufferAux, strlen(outBufferAux)*sizeof(char)) < 0){
          syslog(LOG_ERR, "Error enviando.\n");
          close(f);
          free (date);
          free (modDate);
          return ERROR;
        }

        /*Enviamos resultado de la ejecución*/

        if(my_send(csock, outBuffer, length) < 0){
            syslog(LOG_ERR, "Error enviando");
            close(f);
            free (date);
            free (modDate);
            return ERROR;
        }
    }

    /*Cerramos la tuberia*/

    if(pclose(pipe) == -1){
        syslog(LOG_ERR, "Error en GET PYTHON SCRIPT: Error cerrando pipe");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
      }
  }
  /*Caso de script .php*/
  else{ 
    /*Creamos el comando a ejecutar y lo guardamos en command*/

    sprintf(command, "php %s \"%s\"", direc, args);

    /*Abrimos una tuberia en modo lectura para enviar el comando y que se ejecute*/

    pipe = popen(command, "r");
   
    if(pipe == NULL){
        syslog(LOG_ERR,"Error creando la tuberia\n");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
    }

    /*Llamamos a funciones necesarias para usar select, que se encargara del timeout*/

    pipeDescr = fileno(pipe);
    FD_ZERO(&descrSet);
    FD_SET(pipeDescr, &descrSet);
    timeFlag = select(pipeDescr + 1, &descrSet, NULL, NULL, &timeOut);

    if(timeFlag == -1){
        //Error en el select, no se puede comprobar timeout, se devuelve error
        syslog(LOG_ERR,"GET-RESPONSE|PHP_SCRIPT: Error en select.\n");
        close(f);
        pclose(pipe);
        free (date);
        free (modDate);
        return ERROR;
    }
    else if(timeFlag == 0){ 
        //Se excede el timeout. Se devuelve un error de timeout.
        syslog(LOG_ERR,"GET-RESPONSE|PHP_SCRIPT: Timeout de ejecucion excedido.\n");
        if(error_response(server_signature, csock, cleanpath, 408, minor_version, max_buffer)==ERROR){
            syslog(LOG_ERR, "Error en HTTP Response ERROR");
            close(f);
            pclose(pipe);
            free(date);
            free(modDate);
            return ERROR;
        }
        close(f);
        pclose(pipe);
        free (date);
        free (modDate);
        return ERROR;
    }
    else{
        //La ejecucion ha ido correctamente. No se excede timeout

        /*Leemos el contenido de la tuberia (resultado de la ejecucion), que guardamos en outBuffer*/

        length = fread((void*)outBuffer, 1, max_buffer, pipe);
        if(length < 0){
            close(f);
            free (date);
            free (modDate);
            return ERROR;
          }
           
         sprintf(outBufferAux, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nLast-Modified: %s\r\nContent-Length: %lu\r\nConnection: keep-alive\r\nContent-Type: %s\r\n\r\n", minor_version, date, server_signature, modDate, (long unsigned int) length, ext);

        /*Enviamos cabeceras*/

        if(my_send(csock, outBufferAux, strlen(outBufferAux)*sizeof(char)) < 0){
          syslog(LOG_ERR, "Error enviando.\n");
          close(f);
          free (date);
          free (modDate);
          return ERROR;
        }

        /*Enviamos resultado de la ejecución*/

        if(my_send(csock, outBuffer, length) < 0){
              syslog(LOG_ERR,"Error enviando");
              close(f);
              free (date);
              free (modDate);
              return ERROR;
        }
    }

    /*Cerramos la tuberia*/

    if(pclose(pipe) == -1){
      syslog(LOG_ERR, "Error en GET PYTHON SCRIPT: Error cerrando pipe");
      close(f);
      free (date);
      free (modDate);
      return ERROR;
    }
  }

  /*No debería llegar hasta aqui*/

  close(f);
  free (date);
  free (modDate);
  return OK;
}

/********
* FUNCIÓN:  int post_response(char* server_signature, int csock, char* direc, char* cleanpath, char* body, char* args_url, int max_buffer, int minor_version, long int timeout)
* ARGS_IN:  char* server_signature - nombre del servidor
            int csock - descriptor del socket al cual debemos enviar la respuesta
            char* direc - direccion del archivo completa
            char* cleanpath - archivo del fichero (sin aniadir el directorio raiz)
            char* body - cuerpo de la peticion (donde hay argumentos de stdin para el script)
            char* args_url - argumentos del script pasados por el URL
            long int max_buffer - longitud maxima del buffer de entrada/salida
            int minor_version - version de HTTP (1.0 o 1.1)
            long int timeout - timeout maximo del servidor
* DESCRIPCIÓN: Responde una peticion POST, ejecuta scripts unicamente
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int post_response(char* server_signature, int csock, char* direc, char* cleanpath, char* body, char* args_url, int max_buffer, int minor_version, long int timeout){
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

    /*Limpiamos outBuffer (lo rellenamos de ceros)*/
    memset(outBuffer,0,max_buffer);

    /*Abrimos el fichero en modo lectura*/

    f = open(direc, O_RDONLY);
    if(f < 0){ //Si no ha podido abrir el fichero, enviamos error
      if(error_response(server_signature, csock, cleanpath, 404, minor_version, max_buffer) == ERROR){
        syslog(LOG_ERR, "Error en HTTP Response ERROR");
        return ERROR;
      }
      return OK;
    }

    /*Ahora vamos a consultar la informacion del archivo abierto*/

    if(fstat(f, &fStat) < 0){ 
      close(f);
      return ERROR;
    }

    /*Rellenamos estructura para controlar timeout de ejecucion*/

    timeOut.tv_sec = (time_t) timeout;
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

  /*Si la extension no esta soportada, devolvemos error*/
  else if(strcmp(ext, "") == 0){
    if(error_response(server_signature, csock, cleanpath, 404, minor_version, max_buffer) == ERROR){
        syslog(LOG_ERR, "Error en HTTP Response ERROR");
        close(f);
        free (date);
        free (modDate);
        return ERROR;
    }  
    close(f);
    free (date);
    free (modDate);
    return OK;
    }

    /*Si no es un script, enviamos una respuesta de error*/

    if(scriptflag == NO_SCRIPT){
        if(error_response(server_signature, csock, cleanpath, 405, minor_version, max_buffer) == ERROR){
            syslog(LOG_ERR, "Error en HTTP Response ERROR");
            close(f);
            free (date);
            free (modDate);
            return ERROR;
        }  
        close(f);
        free (date);
        free (modDate);
        return OK;
    } 

    /*Caso en el que es un script .py*/

    else if (scriptflag == PYTHON_SCRIPT){

        /*Creamos el comando a ejecutar (pasamos los argumentos de stdin con un pipe de echo)y lo guardamos en command*/

        sprintf(command, "echo \"%s\" | python %s \"%s\"", body, direc, args_url);

        /*Abrimos una tuberia en modo lectura para enviar el comando y que se ejecute*/

        pipe = popen(command, "r");
        if(pipe == NULL){
            syslog(LOG_ERR,"Error creando la tuberia\n");
            close(f);
            free (date);
            free (modDate);
            return ERROR;
        }

        /*Llamamos a funciones necesarias para usar select, que se encargara del timeout*/

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
        else if(timeFlag == 0){ //Se excede el timeout
            syslog(LOG_ERR,"POST-RESPONSE|PYTHON_SCRIPT: Timeout de ejecucion excedido.\n");
            if(error_response(server_signature, csock, cleanpath, 408, minor_version, max_buffer) == ERROR){
                syslog(LOG_ERR, "Error en HTTP Response ERROR");
                close(f);
                pclose(pipe);
                free (date);
                free (modDate);
                return ERROR;
            }
            close(f);
            pclose(pipe);
            free (date);
            free (modDate);
            return ERROR;
        }
        else{
            /*Se ha escrito en el fichero antes del timeout*/

            /*Leemos el contenido de la tuberia(resultado de la ejecucion) y lo guardamos en outBuffer*/
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
            if(my_send(csock, outBufferAux, strlen(outBufferAux)*sizeof(char)) < 0){
              syslog(LOG_ERR,"Error enviando.\n");
              close(f);
              pclose(pipe);
              free (date);
              free (modDate);
              return ERROR;
            }
            
            /*Enviamos resultado de la ejecución*/
            if(my_send(csock, outBuffer, length) < 0){
              syslog(LOG_ERR,"Error enviando");
              close(f);
              pclose(pipe);
              free (date);
              free (modDate);
              return ERROR;
            }
        }

        /*Cerramos la tuberia*/
        
        if(pclose(pipe) == -1){
          syslog(LOG_ERR, "Error en POST-RESPONSE|PYTHON_SCRIPT: Error cerrando pipe");
          close(f);
          free (date);
          free (modDate);
          return ERROR;
        }
    }

    /*Caso en el que sea un script .php*/

    else{ 

        /*Creamos el comando a ejecutar (pasamos los argumentos de stdin con un pipe de echo)y lo guardamos en command*/

        sprintf(command, "echo \"%s\" | php %s \"%s\"", body, direc, args_url);

        /*Abrimos una tuberia en modo lectura para enviar el comando y que se ejecute*/

        pipe = popen(command, "r");

        if(pipe == NULL){
            syslog(LOG_ERR,"Error creando la tuberia\n");
            close(f);
            free (date);
            free (modDate);
            return ERROR;
        }


        /*Llamamos a funciones necesarias para usar select, que se encargara del timeout*/

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
            if(error_response(server_signature, csock, cleanpath, 408, minor_version, max_buffer) == ERROR){
                syslog(LOG_ERR, "Error en HTTP Response ERROR");
                close(f);
                pclose(pipe);
                free (date);
                free (modDate);
                return ERROR;
            }
            close(f);
            pclose(pipe);
            free (date);
            free (modDate);
            return ERROR;
        }
        else{
            /*Se ha escrito en el fichero antes del timeout*/

            /*Leemos el contenido de la tuberia(resultado de la ejecucion) y lo guardamos en outBuffer*/
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
            if(my_send(csock, outBufferAux, strlen(outBufferAux)*sizeof(char)) < 0){
                syslog(LOG_ERR,"Error enviando.\n");
                close(f);
                pclose(pipe);
                free (date);
                free (modDate);
                return ERROR;
            }
            /*Enviamos resultado de la ejecución*/
            if(my_send(csock, outBuffer, length) < 0){
                syslog(LOG_ERR,"Error enviando");
                close(f);
                pclose(pipe);
                free (date);
                free (modDate);
                return ERROR;
            }   
        }

        /*Cerramos la tubería*/

        if(pclose(pipe) == -1){
          syslog(LOG_ERR, "Error en POST PYTHON SCRIPT: Error cerrando pipe");
          close(f);
          pclose(pipe);
          free (date);
          free (modDate);
          return ERROR;
        }
    }

    /*No deberia llegar nunca hasta aqui*/

    close(f);
    free (date);
    free (modDate);
    return OK;
}


/********
* FUNCIÓN:  int options_response(char* server_signature, int csock, int buffer_size, int minor_version, allowedMethods* am)
* ARGS_IN:  char* server_signature - nombre del servidor
            int csock - descriptor del socket al cual debemos enviar la respuesta
            long int buffer_size - longitud maxima del buffer de entrada/salida
            int minor_version - version de HTTP (1.0 o 1.1)
            allowedMethods* am - metodos permitidos en este directorio
* DESCRIPCIÓN: Responde una peticion OPTIONS
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int options_response(char* server_signature, int csock, int buffer_size, int minor_version, allowedMethods* am){
  char* date;
  char outBuffer[buffer_size];

  /*Rellenamos los parametros necesarios para la respuesta y enviamos respuesta*/
  /*En am, tenemos los metodos disponibles que habrá que mostrar en el allow*/

  date = get_date();
  sprintf(outBuffer, "HTTP/1.%d 200 OK\r\nDate: %s\r\nServer: %s\r\nAllow: %s\r\nContent-Length: 0\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\n", minor_version, date, server_signature, am->txtChain);
  if(my_send(csock, outBuffer, strlen(outBuffer)*sizeof(char)) < 0){
    syslog(LOG_ERR,"Error enviando.\n");
    free(date);
    return ERROR;
  }
  free(date);
  return OK;
}

/********
* FUNCIÓN:  int error_response(char* server_signature, int csock, char* cleanpath, int errnum, int minor_version, int buf_size)
* ARGS_IN:  char* server_signature - nombre del servidor
            int csock - descriptor del socket al cual debemos enviar la respuesta
            char* cleanpath - ruta del archivo sin el directorio raiz
            int errnum - numero de error
            int minor_version - version de HTTP (1.0 o 1.1)
            long int buf_size - longitud maxima del buffer de entrada/salida
* DESCRIPCIÓN: Genera una respuesta de error
* ARGS_OUT: ERROR (-1) en caso de error, OK (0) en caso contrario
********/

int error_response(char* server_signature, int csock, char* cleanpath, int errnum, int minor_version, int buf_size){
    char htmlCode[1000];
    char outBuffer[buf_size];
    char* date;
    date = get_date();

    /*Comprobamos el numero de error y enviamos la respuesta que proceda*/
    switch (errnum){
        
        /*Caso en el que la peticion este mal formada*/

        case 400:
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>400 Bad Request</title>\n</head><body>\n<h1>Bad Request</h1>\n<p>The server could not understand the request due to invalid syntax.</p>\n</body></html>");
          
          sprintf(outBuffer, "HTTP/1.%d 400 Bad Request\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
          break;

        /*Caso archivo No Encontrado*/

        case 404:
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html lang = \"en\"><head>\n<title>%s could not be found</title>\n</head><body>\n<h1>404 RESOURCE NOT FOUND</h1>\n<p><img src = \"https://pre00.deviantart.net/d98e/th/pre/i/2016/162/a/3/dead_link_by_mr_sage-d78unho.png\"alt = \"Link not found\"/></p>\n</body></html>", cleanpath);

          sprintf(outBuffer, "HTTP/1.%d 404 Not Found\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
          break;

        /*Caso en el que un metodo no este permitido*/

        case 405:
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>405 Not Allowed</title>\n</head><body>\n<h1>Not Allowed</h1>\n<p>The method you are trying to use is not allowed in here.</p>\n</body></html>");
          
          sprintf(outBuffer, "HTTP/1.%d 405 Not Allowed\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
          break;

        /*Caso en el que exista un timeout*/

        case 408:
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>408 Request Timeout</title>\n</head><body>\n<h1>Request Timeout</h1>\n<p>The server did not receive a complete request message within the time that it was prepared to wait.</p>\n</body></html>");
          
          sprintf(outBuffer, "HTTP/1.%d 408 Request Timeot\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
          break;

        /*Caso Metodo No Implementado*/

        case 501:
          sprintf(htmlCode, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n<html><head>\n<title>501 Method Not Implemented</title>\n</head><body>\n<h1>Method Not Implemented</h1>\n<p>Only GET,POST,OPTIONS<br />\n</p>\n</body></html>");
          
          sprintf(outBuffer, "HTTP/1.%d 501 Method Not Implemented\r\nDate: %s\r\nServer: %s\r\nAllow: GET,POST,OPTIONS\r\nContent-Length: %lu\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n%s\r\n", minor_version, date, server_signature, sizeof(char)*strlen(htmlCode), htmlCode);
          break;

        default:
          syslog(LOG_ERR, "Error no implementado");
          free(date);
          return ERROR;
    
    }

    /*Enviamos la respuesta*/

    if(my_send(csock, outBuffer, strlen(outBuffer)) < 0){
      free(date);
      syslog(LOG_ERR,"Error enviando");
      return ERROR;
    }
    free(date);
    return OK;
}

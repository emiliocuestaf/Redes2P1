#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <signal.h>
#include <syslog.h>
#include "confuse.h"
#include "socket_management.h"
#include "http_processing.h"
#include "threadPool.h"
#include "daemon.h"


int sock;
cfg_t *cfg;
threadPool* pool;

//Seniales parseo de server conf
static char* server_root = NULL;
static long int max_clients = 0;
static long int buf_size = 0;
static char* listen_port = NULL;
static char* server_signature = NULL;

int main(){

	sigset_t set;
    struct cfg_opt_t cfgopt;
	struct addrinfo* addr;

    //Parseo de server.conf
    cfg_opt_t opts[] = {
		CFG_SIMPLE_STR("server_root", &server_root),
		CFG_SIMPLE_INT("max_clients", &max_clients),
		CFG_SIMPLE_STR("listen_port", &listen_port),
		CFG_SIMPLE_STR("server_signature", &server_signature),
		CFG_SIMPLE_INT("buf_size", &buf_size),
		CFG_END()
	};
	
    cfg = cfg_init(opts, 0);

    if(cfg_parse(cfg, "./src/server.conf") == CFG_PARSE_ERROR){
        syslog(LOG_ERR, "Error en Concurrent Server: Error en cfg_parse()");
        return -1;
    }

    cfg_free(cfg);
    free(server_root);
    free(server_signature);
    free(listen_port);

    return 0;
}
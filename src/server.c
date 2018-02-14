int main(int argc, char ∗∗argv){

	int listenfd, connfd;
	socklen_t clilen, addrlen;
	struct sockaddr ∗cliaddr;

	/∗ Contiene las llamadas a socket(), bind() y listen() ∗/
	listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
	
	for ( ; ; ) {
	
		connfd = Accept(listenfd, cliaddr, &clilen);

		process_request(connfd);

		Close(connfd);
	}

}

int initiate_server(void){
	int sockval;
	struct sockaddr_in Direccion;

	syslog (LOG_INFO, "Creating socket");
	if ( (sockval = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
		syslog(LOG_ERR, "Error creating socket");
		exit(EXIT_FAILURE);
	}

	Direccion.sin_family=AF_INET; /∗ TCP/IP family ∗/
	Direccion.sin_port=htons(NFC_SERVER_PORT); /∗ Asigning port ¿Por que este puerto∗/
	Direccion.sin_addr.s_addr=htonl(INADDR_ANY); /∗ Accept all adresses ∗/
	bzero((void ∗)&(Direccion.sin_zero), 8);

	syslog (LOG_INFO, "Binding socket");

	if (bind (sockval, (struct sockaddr ∗)&Direccion, sizeof(Direccion))<0){
		syslog(LOG_ERR, "Error binding socket");
		exit(EXIT_FAILURE);
	}

	syslog (LOG_INFO, "Listening connections");
	if (listen (sockval, MAX_CONNECTIONS)<0){
		syslog(LOG_ERR, "Error listenining");
		exit(EXIT_FAILURE);
	}

	return sockval;

}

void accept_connection(int sockval){
	int desc, len;
	struct sockaddr Conexion;

	len =sizeof(Conexion);

	if ((desc = accept(sockval, &Conexion, &len))<0){
		syslog(LOG_ERR, "Error accepting connection");
		exit(EXIT_FAILURE);
	}

	launch_service(desc);
	wait_finished_services();

	return;
}

void launch_service(int connval){
	int pid;
	long type, aux;

	pid = fork();
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid == 0) return;
	
	syslog (LOG_INFO, "New access");
	recv(connval, &aux, sizeof(long), 0);
	type = ntohl(aux);
	
	database_access(connval, type, NULL);
	close(connval);
	syslog (LOG_INFO, "Exiting service");
	exit(EXIT_SUCCESS);
}
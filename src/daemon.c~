void do_daemon(void){
	pid_t pid;

	pid = fork(); /∗ Fork o the parent process ∗/
	if (pid < 0) exit(EXIT_FAILURE);
	if (pid > 0) exit(EXIT_SUCCESS); /∗ Exiting the parent process. ∗/
	
	umask(0); /∗ Change the le mode mask ∗/
	setlogmask (LOG_UPTO (LOG_INFO)); /∗ Open logs here ∗/
	openlog ("Server system messages:", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL3);
	syslog (LOG_ERR, "Initiating new server.");
	
	if (setsid()< 0) { /∗ Create a new SID for the child process ∗/
		syslog (LOG_ERR, "Error creating a new SID for the child process.");
		exit(EXIT_FAILURE);
	}
	
	if ((chdir("/")) < 0) { /∗ Change the current working directory ∗/
		syslog (LOG_ERR, "Error changing the current working directory = \"/\"");
		exit(EXIT_FAILURE);
	}
	
	syslog (LOG_INFO, "Closing standard le descriptors");
	close(STDIN_FILENO); close(STDOUT_FILENO); close(STDERR_FILENO); /∗ Close out the standard le descriptors ∗/
	return;
}
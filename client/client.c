#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#define HOST "127.0.0.1"
#define PORT "2323"

/*make socket by host and port*/
int
make_socket(char *host, char *port){

	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = 0
	}, *serv_info, *p;

	int retval;
	if((retval = getaddinfo(host, port, &hints, &serv_info)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
		exit(EXIT_FAILURE);
	}

	int sock;
	for(p = serv_info; p != NULL; p->ai_next){
		if((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
			close(sock);
			continue;
		}

		if(connect(sock, p->addr, p->addrlen) < 0){
			close(sock);
			continue;
		}
		break;
	}

	return sock;
}

int
main(){
	
	int cfd = make_socket(HOST, PORT);
	fprintf(stdout, "success connect!\n");

	int pipefd[2];
	if(pipe(pipefd) < 0)
		return EXIT_FAILURE;

	pid_t pid;
	if((pid = fork()) < 0)
		return EXIT_FAILURE;

	if(pid == 0){

		close(pipefd[0]);

		dup2(pipefd[1], stdout);

		close(pipefd[1]);

		char *args[] = {"", "6", NULL};
		execve("", args, NULL);
	} else {

	}

	return 0;
}




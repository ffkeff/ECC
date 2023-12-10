#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <assert.h>
#include <string.h>


#define HOST "127.0.0.1"
#define PORT "7777"

char *secure_key = "1234353452";

/*make socket by host and port*/
int
make_socket(char *host, char *port){

	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = 0
	}, *serv_info, *p;

	int retval;
	if((retval = getaddrinfo(host, port, &hints, &serv_info)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
		exit(EXIT_FAILURE);
	}

	int sock;
	for(p = serv_info; p != NULL; p->ai_next){
		if((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
			close(sock);
			continue;
		}

		if(connect(sock, p->ai_addr, p->ai_addrlen) < 0){
			close(sock);
			continue;
		}
		break;
	}

	return sock;
}

int
main(void){
	
	/*create connection with server*/
	int cfd = make_socket(HOST, PORT);

	/*create the pipe*/
	int pipefd[2];
	if(pipe(pipefd) < 0){
		fprintf(stderr, "[Error] pipe()\n");
		return EXIT_FAILURE;
	}

	/*create a child process*/
	pid_t pid;
	if((pid = fork()) < 0){
		fprintf(stderr, "[Error] fork()\n");
		return EXIT_FAILURE;
	}

	if (pid == 0) {

        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        char *args[] = {"../ecc", secure_key, NULL};
        execve("../ecc", args, NULL);
    } else {

        close(pipefd[1]);
        char buf;

		/*send public key to server*/
        while(read(pipefd[0], &buf, 1) > 0)
            send(cfd, &buf, 1, 0);

		/*read public key from server*/
		while(recv(cfd, &buf, 1, 0) > 0)
			write(STDOUT_FILENO, &buf, 1);

        close(pipefd[0]);
    }

	return 0;
}




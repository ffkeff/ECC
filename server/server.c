#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define SERV_PORT 7777
#define BACKLOG 1

char *secure_key = "423543453453645";

int
main(void){

	/*create and bind server socket*/
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd < 0){
		fprintf(stderr, "[Error] socket()\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(SERV_PORT)
	};

	socklen_t addrlen = sizeof(addr);
	if(bind(sfd, (struct sockaddr*) &addr, addrlen) < 0){
		fprintf(stderr, "[Error] bind()\n");
		return EXIT_FAILURE;
	}

	if(listen(sfd, BACKLOG) < 0){
		fprintf(stderr, "[Error] listen()\n");
		return EXIT_FAILURE;
	}

	int fd = accept(sfd, (struct sockaddr *) &addr, &addrlen);
	if(fd < 0){
		fprintf(stderr, "[Error] accept()\n");
		return EXIT_FAILURE;
	}

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

		/*read public key from client*/
		while(recv(fd, &buf, 1, 0) > 0)
			write(STDOUT_FILENO, &buf, 1);

		/*send public key to client*/
        while(read(pipefd[0], &buf, 1) > 0)
            send(fd, &buf, 1, 0);

        close(pipefd[0]);
    }

	return 0;
}

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define SERV_PORT 2323
#define BACKLOG 1

int
main(){

	int sfd;
	if((sfd = socket(AF_INET, SOCK_STREAM, 0) < 0))
			return EXIT_FAILURE;

	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(SERV_PORT)
	};

	socklen_t addrlen = sizeof(addr);
	if(bind(sfd, (struct sockaddr*) &addr, addrlen) < 0)
		return EXIT_FAILURE;

	if(listen(sfd, BACKLOG) < 0)
		return EXIT_FAILURE;

	int fd;
	if((fd = accept(sfd, (struct sockaddr *) &addr, &addrlen) < 0))
		return EXIT_FAILURE;

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

		close(pipefd[1]);
		close(pipefd[0]);


	}

	return 0;
}

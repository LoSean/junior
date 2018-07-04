#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define DEFAULTPORT 7122

struct req {
	uint32_t seq;
	uint32_t pingNum;
};

struct clientInfo {
	int connFd;
	char *IP;
	unsigned short port;
};

static void chld_handler(int sig) {
	while(waitpid(-1, NULL, WNOHANG) > 0){
		int i;
//		for(i = 0; i < 100000; i++){}
#ifdef DEBUG
		fprintf(stderr, "===========================\n");
#endif
	}
	return;
}

int server_start(int listenPort) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket open error: ");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(listenPort);
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind error");
		close(fd);
		return -1;
	}
#ifdef DEBUG
	else {
		printf("server listen on port %d\n", listenPort);
	}
#endif
	if (listen(fd, 1024) < 0) {
		perror("listen error");
		close(fd);
		return -1;
	}
	return fd;
}

void *handle_request(void *ptr) {
	struct clientInfo *client = ptr;
	struct req receive;
	int error = 0;
	if (recv(client->connFd, &receive, sizeof(receive), MSG_WAITALL) != sizeof(receive)) {
		fprintf(stderr, "receive packet error\n");
	}
	else {
		printf("recv from %s:%u, seq = %d\n", client->IP, client->port, (int)receive.seq);
		if (send(client->connFd, &receive, sizeof(receive), 0) != sizeof(receive)) {
			if (errno == EPIPE) {
				printf("broken pipe\n");
				errno = 0;
			}
			fprintf(stderr, "send packet error\n");
		}
#ifdef DEBUG
		else {
			printf("server send success\n");
		}
#endif
	}
	close(client->connFd);
	free(client);
#ifdef DEBUG
	printf("fork end\n");
#endif
	exit(0);
}

void server_run(int fd) {
	struct sigaction pipe_act;
	memset(&pipe_act, 0, sizeof(struct sigaction));
	pipe_act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &pipe_act, NULL);
	struct sigaction chld_act;
	memset(&chld_act, 0, sizeof(struct sigaction));
	chld_act.sa_handler = chld_handler;
	sigaction(SIGCHLD, &chld_act, NULL);
	
	int connFd;
	socklen_t connLen;
	struct sockaddr_in addr;
	pid_t pid;
	while(1) {
		memset(&addr, 0, sizeof(addr));
		connLen = sizeof(addr);
		do{
			errno = 0;
			connFd = accept(fd, (struct sockaddr *) &addr, &connLen);
		}while(connFd < 0 && (errno == EINTR || errno == ECHILD));
		if (connFd < 0) {
			perror("server accept error");
			break;
		}
		else {
			pid = fork();
			//child process
			if (pid == 0) {
				struct clientInfo client;
				client.connFd = connFd;
				client.IP = inet_ntoa(addr.sin_addr);
				client.port = ntohs(addr.sin_port);
				handle_request(&client);
			}
			else if (pid < 0) {
#ifdef DEBUG
				fprintf(stderr, "fork error\n");
#endif
				close(connFd);
			//	return;
			}
			else {
				close(connFd);
			}
		}
	}
	close(fd);
	return;
}

int main(int argc, char *argv[]) {
	int listenPort = DEFAULTPORT;
	if (argv[1] != NULL) {
		listenPort = atoi(argv[1]);
	}

	int fd = server_start(listenPort);
	if (fd < 0) {
		fprintf(stderr, "server start error\n");
		return 0;
	}
	server_run(fd);
	return 0;
}

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
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
		fprintf(stderr, "recv from %s:%u, seq = %d\n", client->IP, client->port, (int)receive.seq);
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
	printf("thread end\n");
#endif
	pthread_exit(NULL);
	return NULL;
}

void server_run(int fd) {
	struct sigaction pipe_act;
	memset(&pipe_act, 0, sizeof(struct sigaction));
	pipe_act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &pipe_act, NULL);
//	signal(SIGPIPE, SIG_IGN);
	int connFd;
	socklen_t connLen;
	struct sockaddr_in addr;
	pthread_t thread;
	while(1) {
		memset(&addr, 0, sizeof(addr));
		connLen = sizeof(addr);
		connFd = accept(fd, (struct sockaddr *) &addr, &connLen);
		if (connFd < 0) {
			perror("server accept error");
			break;
		}
		else {
			struct clientInfo *client = malloc(sizeof(struct clientInfo));
			client->connFd = connFd;
			client->IP = inet_ntoa(addr.sin_addr);
			client->port = ntohs(addr.sin_port);
			if (pthread_create(&thread, NULL, handle_request, client) != 0) {
				fprintf(stderr, "handle request create error\n");
				close(client->connFd);
				free(client);
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

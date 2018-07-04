#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#define DEFAULTNUM 0
#define DEFAULTTIMEOUT 1000
#define DEFAULTPORT 7122
#define ADDRESSMAX 1024


struct req {
	uint32_t seq;
	uint32_t pingNum;
};

struct pingTask {
	int pingNum;
	int pingTimeout;
	char *addressPort;
};

int set_parameter(char **par, int *num, int *timeout) {
	char *end;
	int count = 1;
	int numChange = 0;
	int timeChange = 0;
	if (par[count] == NULL) {
		return 0;
	} 
	while (par[count] != NULL) {
		if (!strcmp("-n", par[count]) && par[count+1] != NULL) {
			*num = strtol(par[count+1], &end, 10);
			count += 2;
			numChange = 1;
		}
		else if (!strcmp("-t", par[count]) && par[count+1] != NULL) {
			*timeout = strtol(par[count+1], &end, 10);
			count += 2;
			timeChange = 1;
		}
		else {
			count++;
		}
	}
	if (numChange) {
		count -= 2;
	}
	if (timeChange) {
		count -= 2;
	}
	return count - 1;
} 

int connect_to(char *address, int port) {
	struct hostent *server = NULL;
#ifdef DEBUG
	printf("%s\n", address);
#endif
	server = gethostbyname(address);
	if (server == NULL) {
		fprintf(stderr, "invalid server address\n");
		return -1;
	}
	
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket open error");
		return -1;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	
	memcpy(&addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
	strcpy(address, inet_ntoa(addr.sin_addr)); // to ipv4
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("server connect error");
		close(fd);
		return -1;
	}
	return fd;
}

void my_ping(int fd, int pingNum, int pingTimeout, char *address, int port) {//fix this function <----------------
	int forever = 0, seq = 0;
	if (pingNum == DEFAULTNUM) {
		forever = 1;
	}

	struct req packet;
	
	struct timeval timeout;
	timeout.tv_sec = pingTimeout / 1000 * 2;
	timeout.tv_usec = pingTimeout % 1000 * 1000 + 999;
	if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
		perror("setsockopt error");
		return;
	}

	struct timespec t0, t1;
	while (pingNum || forever) {
#ifdef DEBUG
		printf("send ping seq = %d\n", seq);
#endif
		int matchseq = seq;
		packet.seq = seq;
		packet.pingNum = pingNum;
		
		clock_gettime(CLOCK_MONOTONIC_RAW, &t0);
		if (send(fd, &packet, sizeof(packet), 0) != sizeof(packet)) {
			if (errno == EPIPE) {
				fprintf(stderr, "broken pipe\n");
				errno = 0;
			}
			fprintf(stderr, "send packet error\n");
			return;
		}
		do {
			if (recv(fd, &packet, sizeof(packet), 0) != sizeof(packet) && errno != EAGAIN) {
				fprintf(stderr, "receive packet error\n");
				return;
			}
			else if (errno == EAGAIN) {
#ifdef DEBUG
				printf("EAGAIN\n");
#endif
				errno = 0;
				break;
			}
#ifdef DEBUG
			else {
				printf("receive success packet.seq = %d\nmatchseq = %d\n", (int)packet.seq, matchseq);
			}
#endif
		} while ((int)packet.seq != matchseq);		//think twice<------------------------------
		clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

		int time = (t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_nsec - t0.tv_nsec) / 1000000;
		if (time > pingTimeout) {
			printf("timeout when connect to %s:%d, seq = %d\n", address, port, matchseq);
		}
		else {
			printf("recv from %s:%d, seq = %d, RTT = %d msec\n", 
					address,
					port,
					(int)packet.seq, 
					time);
		}
		seq++;
		pingNum--;
	}
	return;
}

void *client_start(void *ptr) {
	struct pingTask *task = ptr;
	char *colon;
	char address[ADDRESSMAX];
	int port = DEFAULTPORT;
	colon = strrchr(task->addressPort, ':');
	if (colon == NULL) {
		strcpy(address, task->addressPort);
	}
	else {
		memcpy(address, task->addressPort, (colon - task->addressPort));
		address[colon-task->addressPort] = '\0';
		port = atoi(colon+1);
	}
	
	int fd = connect_to(address, port);
	if (fd < 0) {
		fprintf(stderr, "connection fail\n");
		return NULL;
	}

	my_ping(fd, task->pingNum, task->pingTimeout, address, port);
	free(task);
	close(fd);
	return NULL;
}

int main(int argc, char *argv[]) {
	char *end;
	int pingNum = DEFAULTNUM;
	int pingTimeout = DEFAULTTIMEOUT;
	
	int paraCount = set_parameter(argv, &pingNum, &pingTimeout);
#ifdef DEBUG
	printf("%d\n%d\n", pingNum, pingTimeout);
	printf("paraCount = %d\n", paraCount);
#endif
	if (paraCount == 0) {
		printf("usage: ./client [-n number] [-t timeout] host_1:port_1 host_2:port_2 ...\n");
		return 0;
	}
	
	struct sigaction pipe_act;
	memset(&pipe_act, 0, sizeof(struct sigaction));
	pipe_act.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &pipe_act, NULL);
	
	pthread_t *threadArr = malloc(sizeof(pthread_t) * paraCount);
	int traverse = 1;
	int threadNum = 0;
	while (argv[traverse] != NULL) {
		if (!strcmp(argv[traverse], "-n") || !strcmp(argv[traverse], "-t") && argv[traverse+1] != NULL) {
			traverse += 2;
			continue;
		}
		struct pingTask *task = malloc(sizeof(struct pingTask));
		task->addressPort = argv[traverse];
		task->pingNum = pingNum;
		task->pingTimeout = pingTimeout;
		if (pthread_create(&threadArr[threadNum], NULL, client_start, task) != 0) {
			fprintf(stderr, "client start error\n");
			free(task);	
		}
		threadNum++;
		traverse++;
	}
	int i;
	for (i = 0; i < paraCount; i++) {
		pthread_join(threadArr[i], NULL);	
	}
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iconv.h>

#define MAXBUF 32

int pttTelnet() {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
#ifdef DEBUG
		perror("socket open error");
#endif
		return -1;
	}
	
	struct sockaddr_in addr;
	memset(&addr,0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(23);
	addr.sin_addr.s_addr = inet_addr("140.112.172.2");

	if (connect(fd, 
		(struct sockaddr *) &addr,
		sizeof(struct sockaddr_in)) < 0) {
#ifdef DEBUG
		perror("server connect error");
#endif
		close(fd);
		return -1;
	}
	return fd;
}

int utf8ChineseCount(char *buff, int len) {
	int i, count;
	for (i = 0, count = 0; i < len; i++) {
		if (!isprint(buff[i]) && buff[i] != '\n')
			count++;
	}
	return (3-count%3) % 3;
}

int parseOption(char *readBuff, char *writeBuff, int *pure, int *chineseChar) {
	char startTag[][16] = {	"<ID>", 
				"<PASS>",
				"<BOARD>",
				"<P>",
				"<CONTENT>",
				"<EXIT>"};
	char endTag[][16] = {	"</ID>", 
				"</PASS>",
				"</BOARD>",
				"</P>",
				"</CONTENT>",
				"</EXIT>"};
	char *start, *end;
	int i;
	for (i = 0; i < 6; i++) {
		if (strstr(readBuff, startTag[i]) != NULL) {
			if (i == 5)
				return 5;
			start = readBuff + strlen(startTag[i]);
			*chineseChar = 0;
			end = strstr(readBuff, endTag[i]);
			*pure = (end == NULL)? 1:
				(end - readBuff >= MAXBUF-1)? 1: 0;
			end = (end == NULL)? start+strlen(start): end;
			end = (end-start >= MAXBUF)? readBuff+MAXBUF-1: end;
			*chineseChar = utf8ChineseCount(start, end-start);
			printf("*chineseChar = %d\n", *chineseChar);
			end += *chineseChar;
			memcpy(writeBuff, start, (end-start));
			writeBuff[end-start] = '\0';
#ifdef DEBUG
			printf("%s", writeBuff);
#endif
			return i;
		}
	}
	if (*pure) {
		start = readBuff + ((*pure)? *chineseChar: strlen(startTag[i]));
		*chineseChar = 0;
		end = strstr(readBuff, endTag[4]);
		*pure = (end == NULL)? 1: 
			(end - start >= MAXBUF-1)? 1: 0;
		end = (end == NULL)? start+strlen(start): end;
		end = (end-start >= MAXBUF)? readBuff+MAXBUF-1: end;
		*chineseChar = utf8ChineseCount(start, end-start);
		printf("*chineseChar = %d\n", *chineseChar);
		end += *chineseChar;
		memcpy(writeBuff, start, (end-start));
		writeBuff[end-start] = '\0';
#ifdef DEBUG
		printf("%s", writeBuff);
#endif
		return 4;
	}
	return -1;
}

void sendID(int serverfd, char *buffer) {
	write(serverfd, buffer, strlen(buffer));
	write(serverfd, "\r\n", 1);
	return;
}

void sendPASS(int serverfd, char *buffer) {
	write(serverfd, buffer, strlen(buffer));
	write(serverfd, "\r\n", 1);
	sleep(1);
	write(serverfd, " ", 1);
	write(serverfd, " ", 1);
	write(serverfd, " ", 1);
	write(serverfd, " ", 1);
	return;
}

void sendBOARD(int serverfd, char *buffer) {
	write(serverfd, "s", 1);
	write(serverfd, buffer, strlen(buffer));
	write(serverfd, "\r\n", 1);
	sleep(1);
	write(serverfd, " ", 1);
	write(serverfd, " ", 1);
	return;
}

char *utf8ToBig5(char *buffer) {
	iconv_t cd;
	cd = iconv_open("BIG-5", "UTF8");
	
	size_t in_s, out_s;
	in_s = strlen(buffer);
	out_s = 3 * in_s;
	
	char *output = malloc(out_s);
	memset(output, 0, out_s);
	char *inptr, *outptr;
	inptr = buffer;
	outptr = output;
	
	iconv(cd, &inptr, &in_s, &outptr, &out_s);
	outptr = '\0';
	
	iconv_close(cd);
	return output;
}

void sendP(int serverfd, char *buffer) {
	write(serverfd, "\x10", 1);
	sleep(1);
	write(serverfd, "\r\n", 1);
	sleep(1);
	char *output = utf8ToBig5(buffer);
	write(serverfd, output, strlen(output));
	free(output);
	write(serverfd, "\r\n", 1);
	return;
}

void sendCONTENT(int serverfd, char *buffer, int pure) {
	int lineFeed = 0;
	if (pure) {
		if (buffer[strlen(buffer)-1] == '\n') {
			lineFeed = 1;
			buffer[strlen(buffer)-1] = '\0';
		}
		char *output = utf8ToBig5(buffer);
		printf("%s\n", output);
		write(serverfd, output, strlen(output));
		if (lineFeed)
			write(serverfd, "\r\n", 1);
		free(output);
	}
	else {
		char *output = utf8ToBig5(buffer);
		printf("%s\n", output);
		write(serverfd, output, strlen(buffer));
		write(serverfd, "\x18", 1);
		sleep(1);
		write(serverfd, "s", 1);
		write(serverfd, "\r\n", 1);
		sleep(1);
		write(serverfd, " ", 1);
		free(output);
	}
	return;
}

void logout(int serverfd) {
	write(serverfd, "qqqqqqqqqqqqqqqqg", 17);
	write(serverfd, "\r\n", 1);
	sleep(1);
	write(serverfd, "y", 1);
	write(serverfd, "\r\n", 1);
	sleep(1);
	write(serverfd, " ", 1);
	return;
}

void sendData(int serverfd, char *buffer, int option, int pure) {
	switch(option) {
		case 0:
			printf("sending ID...\n");
			sendID(serverfd, buffer);
			break;
		case 1:
			sleep(1);
			printf("sending PASSWORD...\n");
			sendPASS(serverfd, buffer);
			sleep(1);
			break;
		case 2:
			printf("searching BOARD...\n");
			sendBOARD(serverfd, buffer);
			sleep(1);
			break;
		case 3:
			printf("writing TITLE...\n");
			sendP(serverfd, buffer);
			sleep(1);
			break;
		case 4:
			printf("writing CONTENT...\n");
			sendCONTENT(serverfd, buffer, pure);
			break;
		case 5:
			printf("LOGOUT...\n");
			logout(serverfd);
			break;
		default:
			break;
	}
	return;
}

void processInput(int serverfd) {
	FILE *fp = fopen("P_input.txt", "r");
	char readBuff[2*MAXBUF];
	char writeBuff[MAXBUF+8];
	char moveBuff[MAXBUF+8];
	char *empty;
	int pure, option, chineseChar;
	pure = 0;
	memset(readBuff, 0, 2*MAXBUF);
	memset(writeBuff, 0, MAXBUF+8);
	if (fp != NULL) {
		do {
			empty = fgets(readBuff+MAXBUF-1, MAXBUF, fp);
			option = parseOption(readBuff, writeBuff, &pure, &chineseChar);
			sendData(serverfd, writeBuff, option, pure);
			strcpy(moveBuff, readBuff+MAXBUF-1);
			strcpy(readBuff, moveBuff);
		} while (empty != NULL);
		fclose(fp);
	}
	return;
}

int main(void) {
	int serverfd = pttTelnet();
	if (serverfd) {
		printf("connection established\n");	
	}
	processInput(serverfd);
	return 0;
}

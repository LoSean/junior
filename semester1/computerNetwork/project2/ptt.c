#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iconv.h>

#define MAXBUF 40

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

void sendID(int serverfd, char *buffer, int pure) {
	write(serverfd, buffer, strlen(buffer));
	if (!pure)
		write(serverfd, "\r", 1);
	return;
}

void sendPASS(int serverfd, char *buffer, int pure) {
	write(serverfd, buffer, strlen(buffer));
	if (!pure) {
		write(serverfd, "\r", 1);
		sleep(1);
		write(serverfd, " ", 1);
		write(serverfd, " ", 1);
		write(serverfd, " ", 1);
		write(serverfd, " ", 1);
	}
	return;
}

void sendBOARD(int serverfd, char *buffer, int pure) {
	write(serverfd, buffer, strlen(buffer));
	if (!pure) {
		write(serverfd, "\r", 1);
		sleep(1);
		write(serverfd, " ", 1);
		write(serverfd, " ", 1);
	}
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

void sendP(int serverfd, char *buffer, int pure) {
	char *output = utf8ToBig5(buffer);
	write(serverfd, output, strlen(output));
	free(output);
	if (!pure) 
		write(serverfd, "\r", 1);
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
		//printf("%s\n", output);
		write(serverfd, output, strlen(output));
		if (lineFeed)
			write(serverfd, "\r", 1);
		free(output);
	}
	else {
		char *output = utf8ToBig5(buffer);
		//printf("%s\n", output);
		write(serverfd, output, strlen(buffer));
		write(serverfd, "\x18", 1);
		sleep(1);
		write(serverfd, "s", 1);
		write(serverfd, "\r", 1);
		sleep(1);
		write(serverfd, " ", 1);
		free(output);
	}
	return;
}

void sendStart(int *serverfd, int option) {
	switch(option) {
		case 0:
			*serverfd = pttTelnet();
			if (*serverfd) {
				printf("connection established\n");	
			}
			break;
		case 2:
			printf("\n---->sending BOARD control code...\n");
			write(*serverfd, "s", 1);
			sleep(1);
			break;
		case 3:
			printf("\n---->sending TITLE control code...\n");
			write(*serverfd, "\x10", 1);
			sleep(1);
			write(*serverfd, "\r", 1);
			sleep(1);
			break;
		case 5:
			printf("\n---->LOGOUT...\n");
			write(*serverfd, "qqqqqqqqqqqqqqqqg", 17);
			write(*serverfd, "\r", 1);
			sleep(1);
			write(*serverfd, "y", 1);
			write(*serverfd, "\r", 1);
			sleep(1);
			write(*serverfd, " ", 1);
			sleep(1);
			close(*serverfd);
			break;
		default:
			break;
	}
	return;
}

void sendData(int serverfd, char *buffer, int option, int pure) {
	switch(option) {
		case 0:
			printf("\n---->sending id...\n");
			sendID(serverfd, buffer, pure);
			break;
		case 1:
			sleep(1);
			printf("\n---->sending password...\n");
			sendPASS(serverfd, buffer, pure);
			sleep(1);
			break;
		case 2:
			printf("\n---->searching BOARD...\n");
			sendBOARD(serverfd, buffer, pure);
			sleep(1);
			break;
		case 3:
			printf("\n---->writing TITLE...\n");
			sendP(serverfd, buffer, pure);
			sleep(1);
			break;
		case 4:
			printf("\n---->writing CONTENT...\n");
			sendCONTENT(serverfd, buffer, pure);
			break;
		default:
			break;
	}
	return;
}

void parseOption(int *serverfd, char *readBuff, char *writeBuff, int *pure, int *chineseChar, int *currOption) {
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
				"</CONTENT>"};
	char *start, *end;
	start = readBuff + *chineseChar;
	int i;
	while (start[0] != '\0' && start - readBuff < MAXBUF - 1) {
		if (!(*pure)) {
			char *tmp, *minPos;
			minPos = readBuff + 3*MAXBUF;
			for (i = 0; i < 6; i++) {
				tmp = strstr(start, startTag[i]);
				if (tmp != NULL && minPos - start > tmp - start) {
					minPos = tmp;
					*currOption = i;
				}
			}
			if (minPos - readBuff != 3*MAXBUF && minPos - readBuff < MAXBUF - 1) {
				start = minPos;
				start += strlen(startTag[*currOption]);
				sendStart(serverfd, *currOption);
				*pure = 1;
				if (start - readBuff >= MAXBUF - 1) {
					*chineseChar = (start - (readBuff + MAXBUF - 1));
					return;
				}
			}
			else {
				return;
			}
		}
		if (*currOption == 5) {
			*pure = 0;
			continue;
		}
		if ((*pure) || (*currOption >= 0 && *currOption < 5)) {
			end = strstr(start, endTag[*currOption]);
			*pure = (end == NULL)? 1: (end - readBuff >= MAXBUF - 1)? 1: 0;
			end = (end == NULL)? start+strlen(start): end;
			end = (end - readBuff >= MAXBUF)? readBuff+MAXBUF-1: end;
			*chineseChar = utf8ChineseCount(start, end-start);
			end += *chineseChar;
			memcpy(writeBuff, start, (end-start));
			writeBuff[end-start] = '\0';
			sendData(*serverfd , writeBuff, *currOption, *pure);
			printf("%s", writeBuff);
			start = end + ((*pure)? 0: strlen(endTag[*currOption]));
		}
		else {
			return;
		}
	}
	return;
}

int main(void) {
	FILE *fp = fopen("P_input.txt", "r");
	char readBuff[2*MAXBUF];
	char writeBuff[MAXBUF+8];
	char moveBuff[MAXBUF+8];
	char *empty;
	int serverfd;
	int pure, chineseChar, currOption;
	pure = chineseChar = 0;
	currOption = 5;
	memset(readBuff, 0, 2*MAXBUF);
	memset(writeBuff, 0, MAXBUF+8);
	if (fp != NULL) {
		do {
			empty = fgets(readBuff+MAXBUF-1, MAXBUF, fp);
			parseOption(&serverfd, readBuff, writeBuff, &pure, &chineseChar, &currOption);
			strcpy(moveBuff, readBuff+MAXBUF-1);
			strcpy(readBuff, moveBuff);
		} while (empty != NULL);
		fclose(fp);
	}
	return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
	char buff[3];
	FILE *fp = fopen("P_input.txt", "r");
	while(fgets(buff, sizeof(buff), fp) != NULL) {
		printf("%s\n", buff);
	}
	return 0;
}

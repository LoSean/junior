#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <iconv.h>

int main(void){
	iconv_t cd;
	size_t in_s, out_s;
	cd = iconv_open("BIG-5", "UTF8");
	if (cd == (iconv_t)-1) {
		perror("iconv error");
		exit(1);
	}
	char input[] = "abc我是中文abc";
	char *in_ptr, *out_ptr;
	char *output;

	for (int i = 0; i < strlen(input); i++) {
		if (isprint(input[i])) {
			printf("input[%d] = %c\n", i, input[i]);
		}
		else {
			printf("input[%d] cannot be printed\n", i);
		}
	}
	in_s = strlen(input);
	in_ptr = input;
	printf("in_s = %d\n", (int)in_s);
	
	out_s = in_s*3;
	output = malloc(out_s);
	out_ptr = output;
	if (iconv(cd, &in_ptr, &in_s, &out_ptr, &out_s) == -1) {
		perror("convert error");
		exit(1);
	}

	*out_ptr = '\0';
	printf("output = %s", output);
	iconv_close(cd);
	free(output);
	putchar('\n');
	return 0;
}

#include <stdio.h>

int recurFunc(int n, int c){
	if (n < 2)
		return c;
	int tmp = recurFunc(n/2, c);
	return 2 * tmp + c * n;
}

int main(void){
	int n, c;
	scanf("%d%d", &n, &c);
	printf("%d\n", recurFunc(n, c));
	return 0;
}
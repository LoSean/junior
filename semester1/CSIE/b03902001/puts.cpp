#include <cstdio>
#include <cmath>
#include <queue>
#include <cstring>
using namespace std;

struct putsOption{
	double S;
	double european;
	double american;
};

double payoff_puts(double S, double X) {
	return (X-S > 0)? X-S: 0;
}

struct putsOption binomial_tree(double S, double X, double u, double d, double rHat, int n) {
	queue<struct putsOption> leaves;
	struct putsOption putsTmp;
	for (int i = 0; i < n + 1; i++) {			//initialize expire-day payoff
		memset(&putsTmp, 0, sizeof(struct putsOption));
		putsTmp.S = S*pow(u, i)*pow(d, n-i);
		putsTmp.european = payoff_puts(putsTmp.S, X);
		putsTmp.american = putsTmp.european;
		leaves.push(putsTmp);
	}
	
	double p = (exp(rHat) - d) / (u - d);
	while(n--) {
		struct putsOption last = leaves.front();
		leaves.pop();
		struct putsOption curr;
		for (int i = 0; i < n + 1; i++) {
			curr = leaves.front();
			leaves.pop();
			memset(&putsTmp, 0, sizeof(struct putsOption));
			putsTmp.S = S*pow(u, i)*pow(d, n-i);
			putsTmp.european = (p * curr.european + (1-p) * last.european) / exp(rHat);
			putsTmp.american = (p * curr.american + (1-p) * last.american) / exp(rHat);
			putsTmp.american = (putsTmp.american > X - putsTmp.S)? 	putsTmp.american:	
										X - putsTmp.S;
			leaves.push(putsTmp);
			last = curr;
		}
	}
	return putsTmp;
}

int main(void) {
	double S, X, maturity, volatility, r;
	int n;
	scanf("%lf%lf%lf%lf%lf%d", &S, &X, &maturity, &volatility, &r, &n);
	double u = exp(volatility*sqrt(maturity/n));
	double d = exp(-volatility*sqrt(maturity/n));
	double rHat = r*maturity/n;
	struct putsOption result = binomial_tree(S, X, u, d, rHat, n);
	printf("European puts = %lf, American puts = %lf\n", result.european, result.american);
	return 0;
}

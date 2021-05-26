#include <stdio.h>
#include <stdlib.h>

void p(int n, int (*f)(int))
{
	for (int i = -1; i < n; ++i)
		printf("%d%c", f(i), i == n - 1 ? '\n' : ' ');
}

int F(int n)
{
	if (n < 2)
		return n;

	return F(n - 1) + F(n - 2);
}

int f(int n)
{
	if (n < 2)
		return n;

	int x = 0, y = 1;
	while (--n > 0) {
		y = x + y;
		x = y - x;
	}

	return y;
}

int main(int c, char **v)
{
	if (c != 2)
		return 1;

	int n = atoi(v[1]);
	p(n, f);
	p(n, F);
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void swap(int *x, int *y)
{
	int i = *x;
	*x = *y;
	*y = i;
}

void sort(int *a, size_t n)
{
	if (n < 2)
		return;

	size_t j = 0;
	for (size_t i = 1; i < n; ++i) {
		if (a[i] < a[0] && ++j != i)
			swap(a + i, a + j);
	}

	swap(a, a + j);
	sort(a, j);
	sort(a + j + 1, n - j - 1);
}

int main(void)
{
	size_t i, n = 10;
	int a[n];

	srand(time(NULL));
	for (i = 0; i < n; ++i) {
		a[i] = rand() % 20;
		printf("%d%c", a[i], i == n - 1 ? '\n' : ' ');
	}

	sort(a, n);
	for (i = 0; i < n; ++i)
		printf("%d%c", a[i], i == n - 1 ? '\n' : ' ');
}

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

struct type {
	struct type *next;
	int i;
};

void print(const struct type *t)
{
	while (t) {
		printf("%d%c", t->i, t->next ? ' ' : '\n');
		t = t->next;
	}
}

void insert(struct type **head, int i)
{
	struct type *t = malloc(sizeof(struct type));
	t->i = i;
	t->next = *head;
	*head = t;
}

void erase(struct type **next, int i)
{
	struct type *t;
	while ((t = *next)) {
		if (t->i == i) {
			*next = t->next;
			free(t);
		} else {
			next = &t->next;
		}
	}
}

int main(void)
{
	struct type *head = NULL;
	for (int i = 0; i < 10; ++i)
		insert(&head, i);

	srand(time(NULL));
	while (head) {
		print(head);
		erase(&head, rand() % 10);
	}
}

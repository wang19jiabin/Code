#include <stdio.h>
#include <unistd.h>
#include <assert.h>

void attach(pid_t p, const char *b)
{
	pid_t gdb = fork();
	assert(gdb != -1);
	if (gdb == 0) {
		char pid[128], cmd[128] = "b ";
		sprintf(pid, "%d", p);
		sprintf(cmd + 2, "%s", b);
		if (execlp("gdb", "gdb", "-p", pid, "-ex", cmd, "-ex", "c", NULL) < 0)
			perror("");
	} else {
		close(STDOUT_FILENO);
	}
}

int main(int c, char **v)
{
	if (c != 2) {
		fprintf(stderr, "Usage: %s breakpoint\n", v[0]);
		return -1;
	}

	pid_t child = fork();
	assert(child != -1);
	if (child == 0) {
		if (execl("child", "child", NULL) < 0)
			perror("");

		return -1;
	}

	attach(child, v[1]);
	while (1) {
		printf("%s\n", __FILE__);
		sleep(1);
	}
}

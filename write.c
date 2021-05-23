#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int fd;
const char *file = "log";
pthread_once_t once = PTHREAD_ONCE_INIT;

void open_file(void)
{
	fd = open(file, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0600);
	assert(fd != -1);
}

void *thread(void *p)
{
	int err = pthread_once(&once, open_file);
	assert(err == 0);
	char buf[1024] = "abcdefghijklmopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
	size_t len = strlen(buf);
	ssize_t n = write(fd, buf, len);
	assert(n == len);
	return NULL;
}

int process(size_t n)
{
	pthread_t tids[n];

	for (size_t i = 0; i < n; ++i) {
		int err = pthread_create(&tids[i], NULL, thread, NULL);
		assert(err == 0);
	}

	for (size_t i = 0; i < n; ++i) {
		int err = pthread_join(tids[i], NULL);
		assert(err == 0);
	}

	return 0;
}

int main(void)
{
	unlink(file);

	for (int i = 0; i < 10; ++i) {
		pid_t pid = fork();
		assert(pid != -1);
		if (pid == 0)
			return process(10);
	}

	while (wait(NULL) != -1) ;

	perror("wait");
}

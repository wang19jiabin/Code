#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/un.h>
#include <sys/socket.h>

#define LEN 100

socklen_t addr(struct sockaddr_un *sun)
{
	sun->sun_family = AF_LOCAL;
	strcpy(sun->sun_path, "@path");
	socklen_t len = SUN_LEN(sun);
	sun->sun_path[0] = 0;
	return len;
}

void server(void)
{
	int fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	struct sockaddr_un sun;
	socklen_t len = addr(&sun);
	if (bind(fd, (struct sockaddr *)&sun, len) < 0) {
		perror("bind");
		return;
	}

	for (size_t i = 1;; ++i) {
		char buf[4096];
		ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
		if (n < 0) {
			perror("recvfrom");
			exit(EXIT_FAILURE);
		}

		assert(n == LEN);
		printf("recvfrom %lu\n", i);
		usleep(10000);
	}
}

void client(void)
{
	int fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	struct sockaddr_un sun;
	socklen_t len = addr(&sun);
	sleep(1);

	for (size_t i = 1; i < 10000; ++i) {
		ssize_t n = sendto(fd, "", LEN, 0, (struct sockaddr *)&sun, len);
		if (n < 0) {
			perror("sendto");
			exit(EXIT_FAILURE);
		}

		assert(n == LEN);
		printf("sendto %lu\n", i);
	}
}

int main(void)
{
	pid_t pid = fork();
	if (pid < 0) {
		perror("fork");
		return 1;
	}

	if (pid == 0)
		server();
	else
		client();
}

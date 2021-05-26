#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <assert.h>

int main(void)
{
	int fd = socket(AF_LOCAL, SOCK_DGRAM, 0);
	struct sockaddr_un sun = { AF_LOCAL };
	strcpy(sun.sun_path, "@path");
	socklen_t len = SUN_LEN(&sun);
	sun.sun_path[0] = 0;

	for (int i = 0; i < 5; ++i) {
		char s[2] = { };
		sprintf(s, "%d", i);
		ssize_t n = sendto(fd, s, sizeof(s), 0, (struct sockaddr *)&sun, len);
		if (n < 0)
			perror("sendto");

		assert(n == sizeof(s));
	}
}

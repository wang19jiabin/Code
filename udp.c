#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include "ntop.h"

struct {
	struct pollfd fds[10];
	nfds_t n;
} fds;

void Read(int fd)
{
	char buf[10];
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf) - 1
	};

	struct sockaddr_storage ss;
	struct msghdr msg = {
		.msg_name = &ss,
		.msg_namelen = sizeof(ss),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};

	ssize_t n = recvmsg(fd, &msg, 0);
	if (n < 0) {
		perror("recvmsg");
		return;
	}

	if (msg.msg_flags & MSG_TRUNC) {
		fprintf(stderr, "recvmsg: MSG_TRUNC\n");
		return;
	}

	buf[n] = '\0';
	char p[12];
	printf("recv %ld:%s from %s\n", n, buf, ntop(&ss, p, sizeof(p), NULL));
}

void Poll(void)
{
	if (fds.n == 0) {
		fprintf(stderr, "%s: no fd\n", __func__);
		return;
	}

	while (1) {
		int n = poll(fds.fds, fds.n, 5 * 1000);
		if (n < 0) {
			perror("poll");
			continue;
		}

		if (n == 0) {
			fprintf(stderr, "poll: timeout\n");
			return;
		}

		for (nfds_t i = 0; i < fds.n; ++i) {
			if (fds.fds[i].revents & POLLIN)
				Read(fds.fds[i].fd);
		}
	}
}

int main(int c, char *v[])
{
	if (c != 4)
		return -1;

	struct addrinfo *ais, hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_DGRAM
	};

	int e = getaddrinfo(v[1], v[2], &hints, &ais);
	if (e != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
		return -1;
	}

	for (struct addrinfo * ai = ais; ai; ai = ai->ai_next) {
		int fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (fd < 0) {
			perror("socket");
			continue;
		}

		ssize_t n = sendto(fd, v[3], strlen(v[3]), 0, ai->ai_addr, ai->ai_addrlen);
		if (n < 0) {
			perror("sendto");
			close(fd);
			continue;
		}

		char p[12];
		printf("send %ld:%s to %s\n", n, v[3], ntop(ai->ai_addr, p, sizeof(p), NULL));

		fds.fds[fds.n].fd = fd;
		fds.fds[fds.n].events = POLLIN;
		if (++fds.n == sizeof(fds.fds) / sizeof(fds.fds[0])) {
			printf("fds is full\n");
			break;
		}
	}

	freeaddrinfo(ais);
	Poll();
}

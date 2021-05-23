#pragma once

#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

static const char *ntop(const void *sa, char *str, socklen_t len)
{
	int af = ((struct sockaddr *)sa)->sa_family;
	void *addr;
	in_port_t port;
	struct sockaddr_in *sin;
	struct sockaddr_in6 *sin6;

	switch (af) {
	case AF_INET:
		sin = (struct sockaddr_in *)sa;
		addr = &sin->sin_addr;
		port = sin->sin_port;
		break;
	case AF_INET6:
		sin6 = (struct sockaddr_in6 *)sa;
		addr = &sin6->sin6_addr;
		port = sin6->sin6_port;
		break;
	default:
		return NULL;
	}

	if (!inet_ntop(af, addr, str, len)) {
		perror("inet_ntop");
		return NULL;
	}

	size_t size = strlen(str);
	len -= size;
	int n = snprintf(str + size, len, ":%hu", ntohs(port));
	if (n < 0 || n >= len)
		return NULL;

	return str;
}

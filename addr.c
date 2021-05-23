#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include "ntop.h"

const char *family[] = {
	[AF_INET] = "AF_INET",
	[AF_INET6] = "AF_INET6"
};

const char *type[] = {
	[SOCK_STREAM] = "SOCK_STREAM",
	[SOCK_DGRAM] = "SOCK_DGRAM",
	[SOCK_RAW] = "SOCK_RAW",
};

const char *protocol[] = {
	[IPPROTO_IP] = "IPPROTO_IP",
	[IPPROTO_TCP] = "IPPROTO_TCP",
	[IPPROTO_UDP] = "IPPROTO_UDP",
	[IPPROTO_ICMP] = "IPPROTO_ICMP",
	[IPPROTO_ICMPV6] = "IPPROTO_ICMPV6"
};

int main(int c, char **v)
{
	const char *host = NULL, *port = NULL;
	struct addrinfo *ais, hints = { };

	for (int i; (i = getopt(c, v, "h:p:P")) != -1;) {
		switch (i) {
		case 'h':
			host = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 'P':
			hints.ai_flags = AI_PASSIVE;
			break;
		default:
			return -1;
		}

	}

	int e = getaddrinfo(host, port, &hints, &ais);
	if (e != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(e));
		return -1;
	}

	for (const struct addrinfo * ai = ais; ai; ai = ai->ai_next) {
		printf("%s\n", family[ai->ai_family]);
		printf("%s\n", type[ai->ai_socktype]);
		printf("%s\n", protocol[ai->ai_protocol]);
		char str[INET6_ADDRSTRLEN + 10];
		printf("%s\n\n", ntop(ai->ai_addr, str, sizeof(str)));
	}
}

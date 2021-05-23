#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "ntop.h"

int main()
{
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	struct ifconf ifc;
	for (int i = 1, len = 0;; i *= 2) {
		ifc.ifc_len = i * sizeof(struct ifreq);
		ifc.ifc_buf = malloc(ifc.ifc_len);
		if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
			perror("ioctl");
			return -1;
		}

		if (ifc.ifc_len == len)
			break;

		len = ifc.ifc_len;
		free(ifc.ifc_buf);
	}

	struct ifreq *ifr = ifc.ifc_req;
	char p[128];
	for (int i = 0; i < ifc.ifc_len / sizeof(struct ifreq); ++i) {
		printf("%s\n", ifr[i].ifr_name);
		printf("%s\n", ntop(&ifr[i].ifr_addr, p, sizeof(p), NULL));
	}

	free(ifc.ifc_buf);
	close(fd);
}

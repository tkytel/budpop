#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/param.h>

#include <netinet/ip.h>

#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
usage(void)
{
	(void)fputs("usage: budpop <listen> <destination> <port>\n", stderr);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int c;
	int bflag;
	bflag = 0;
	while ((c = getopt(argc, argv, "b")) != -1)
		switch (c) {
		case 'b':
			bflag++;
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 3)
		usage();

	struct addrinfo hints;
	(void)memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *res;
	int error;
	error = getaddrinfo(NULL, argv[0], &hints, &res);
	if (error != 0)
		if (error == EAI_SYSTEM)
			err(EXIT_FAILURE, "getaddrinfo");
		else
			errx(EXIT_FAILURE, "getaddrinfo: %s",
					gai_strerror(error));

	struct addrinfo *ai;
	int fd;
	const char *cause;
	for (ai = res; ai != NULL; ai = ai->ai_next) {
		fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		if (fd == -1) {
			cause = "socket";
			continue;
		}
		if (bind(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
			cause = "bind";
			error = errno;
			(void)close(fd);
			errno = error;
			continue;
		}
		if (bflag && setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &bflag,
					sizeof(bflag)) == -1) {
			cause = "SO_BROADCAST";
			error = errno;
			(void)close(fd);
			errno = error;
			continue;
		}
		break;
	}
	if (ai == NULL)
		err(EXIT_FAILURE, "%s", cause);
	freeaddrinfo(res);

loop:;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);
	FD_SET(fd, &rfds);

	int nfds;
	nfds = MAX(STDIN_FILENO, fd) + 1;
	if (select(nfds, &rfds, NULL, NULL, NULL) == -1)
		err(EXIT_FAILURE, "select");

	struct sockaddr_in sin;
	socklen_t sinlen;
	ssize_t bytes;
#define BUFLEN	1024
	char buf[BUFLEN];
	if (FD_ISSET(fd, &rfds)) {
		sinlen = sizeof(sinlen);
		bytes = recvfrom(fd, buf, sizeof(buf), 0,
				(struct sockaddr *)&sin, &sinlen);
		if (bytes == -1)
			err(EXIT_FAILURE, "recvfrom");
		if (write(STDOUT_FILENO, buf, bytes) == -1)
			err(EXIT_FAILURE, "stdout");
	} else {
		bytes = read(STDIN_FILENO, buf, sizeof(buf));
		if (bytes == -1)
			err(EXIT_FAILURE, "stdin");

		(void)memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;

		error = getaddrinfo(argv[1], argv[2], &hints, &res);
		if (error != 0)
			if (error == EAI_SYSTEM)
				err(EXIT_FAILURE, "getaddrinfo");
			else
				errx(EXIT_FAILURE, "getaddrinfo: %s",
						gai_strerror(error));

		if (sendto(fd, buf, bytes, 0, res->ai_addr, res->ai_addrlen)
				== -1)
			err(EXIT_FAILURE, "sendto");

		freeaddrinfo(res);
	}

	if (bytes > 0)
		goto loop;

	(void)close(fd);

	return 0;
}

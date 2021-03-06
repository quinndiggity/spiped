#include <netinet/in.h>

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "warnp.h"

#include "simple_server.h"

#define MAX_CONNECTIONS 2
#define SHUTDOWN_AFTER 1

struct nc_cookie {
	FILE * out;
};

/* A client sent a message. */
static int
callback_snc_response(void * cookie, uint8_t * buf, size_t buflen)
{
	struct nc_cookie * C = cookie;

	/* Write buffer to the previously-opened file. */
	if (fwrite(buf, sizeof(uint8_t), buflen, C->out) != buflen) {
		warnp("fwrite");
		goto err0;
	}

	/* Success! */
	return (0);

err0:
	/* Failure! */
	return (-1);
}

int
main(int argc, char ** argv)
{
	struct nc_cookie cookie;
	struct nc_cookie * C = &cookie;
	unsigned long port_long;
	in_port_t port;
	const char * filename;

	WARNP_INIT;

	/* Parse command-line arguments. */
	if (argc < 3) {
		fprintf(stderr, "usage: %s PORT FILENAME\n", argv[0]);
		goto err0;
	}

	/* Parse port number. */
	errno = 0;
	port_long = strtoul(argv[1], NULL, 0);
	if (errno || port_long > UINT16_MAX) {
		warnp("strtoul");
		goto err0;
	}
	port = (in_port_t)port_long;

	/* Get output filename. */
	filename = argv[2];

	/* Open the output file; can be /dev/null. */
	if ((C->out = fopen(filename, "wb")) == NULL) {
		warnp("fopen");
		goto err0;
	}

	/* Run the server. */
	if (simple_server(port, MAX_CONNECTIONS, SHUTDOWN_AFTER,
	    &callback_snc_response, C)) {
		warn0("simple_server failed");
		goto err1;
	}

	/* Write the output file. */
	if (fclose(C->out) != 0) {
		warnp("fclose");
		goto err0;
	}

	/* Success! */
	exit(0);

err1:
	fclose(C->out);
err0:
	/* Failure! */
	exit(1);
}

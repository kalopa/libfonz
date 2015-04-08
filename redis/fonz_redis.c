/*
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <signal.h>

#include "hiredis/hiredis.h"
#include "hiredis/async.h"
#include "hiredis/adapters/libevent.h"

#include "libfonz.h"

/*
 * Table to convert baud rate into a code.
 */
struct  speed   {
        int     value;
        int     code;
} speeds[] = {
        {50, B50},
        {75, B75},
        {110, B110},
        {134, B134},
        {150, B150},
        {200, B200},
        {300, B300},
        {600, B600},
        {1200, B1200},
        {1800, B1800},
        {2400, B2400},
        {4800, B4800},
        {9600, B9600},
        {19200, B19200},
        {38400, B38400},
        {57600, B57600},
        {115200, B115200},
        {230400, B230400},
        {0, 0}
};

void	redis_receiver(char *hostname, int port);
void	redis_transmitter(char *hostname, int port);
void	pkt_recv(redisAsyncContext *, void *, void *);
void	usage();

int	filedes;
FILE	*txlfp;
FILE	*rxlfp;

/*
 *
 */
int
main(int argc, char *argv[])
{
	int i, verbose, speed, port;
	char *device, *hostname;
	struct termios tios;

	verbose = 0;
	port = 6379;
	speed = 9600;
	hostname = "127.0.0.1";
	device = "/dev/ttyU0";
	txlfp = rxlfp = NULL;
	while ((i = getopt(argc, argv, "h:p:d:s:v")) != EOF) {
		switch (i) {
		case 'h':
			hostname = optarg;
			break;

		case 'p':
			port = atoi(optarg);
			break;

		case 'd':
			if (strncmp(optarg, "/dev/", 5) != 0) {
				device = malloc(strlen(optarg) + 6);
				strcpy(device, "/dev/");
				strcat(device, optarg);
			} else
				device = strdup(optarg);
			break;

		case 's':
			speed = atoi(optarg);
			break;

		case 'v':
			verbose = 1;
			break;

		default:
			usage();
			break;
		}
	}
	/*
	 * Open the serial device.
	 */
	signal(SIGPIPE, SIG_IGN);
	if (verbose)
		printf("Serial device: %s, speed: %d\n", device, speed);
	if ((filedes = open(device, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
		fprintf(stderr, "serial device open: ");
		perror(device);
		exit(1);
	}
	/*
	 * Enable read blocking.
	fcntl(filedes, F_SETFL, fcntl(filedes, F_GETFL, 0) & ~O_NONBLOCK);
	 */
	fcntl(filedes, F_SETFL, 0);
        /*
         * Get and set the tty parameters.
         */
        if (tcgetattr(filedes, &tios) < 0) {
                perror("tcgetattr failed");
		exit(1);
	}
        for (i = 0; speeds[i].value > 0; i++)
                if (speeds[i].value == speed)
                        break;
        if (speeds[i].value == 0) {
                fprintf(stderr, "?Error - invalid baud rate.\n");
		exit(1);
	}
        cfsetispeed(&tios, speeds[i].code);
        cfsetospeed(&tios, speeds[i].code);
        tios.c_cflag &= ~(CSIZE|PARENB|CSTOPB);
        tios.c_cflag |= (CLOCAL | CREAD | CS8);
        tios.c_lflag = tios.c_iflag = tios.c_oflag = 0;
        if (tcsetattr(filedes, TCSANOW, &tios) < 0) {
                perror("tcsetattr failed");
		exit(1);
	}
	/*
	 * Split into sender and receiver.
	 */
	if (verbose) {
		printf("Connecting to REDIS at %s:%d...\n", hostname, port);
		rxlfp = fopen("fonz-out.log", "w");
		txlfp = fopen("fonz-in.log", "w");
	}
	if ((i = fork()) < 0) {
		perror("fork");
		exit(1);
	}
	if (i == 0) {
		/*
		 * Child process - do the transmission work.
		 */
		redis_transmitter(hostname, port);
	} else {
		/*
		 * Parent process - do the receiver work.
		 */
		redis_receiver(hostname, port);
	}
	exit(0);
}

/*
 * Responsible for reading from the serial port, assembling packets and
 * sending them via a Redis PUBLISH command.
 */
void
redis_transmitter(char *hostname, int port)
{
	int i, n;
	unsigned char rdbuffer[32];
	char wrbuffer[32];
	struct fonz *fp;
	redisContext *ctxt;

	/*
	 * Initialize the Fonz packet receiver, and the Redis connection.
	 */
	fp_init(4, 0);
	ctxt = redisConnect(hostname, port);
	if (ctxt->err) {
		fprintf(stderr, "?redis (tx) error: %s\n", ctxt->errstr);
		exit(1);
	}
	/*
	 * Loop forever, pulling packets from the serial wire and sending
	 * them out via Redis PUB/SUB.
	 */
	while (1) {
		/*
		 * Read a pile of data from the serial device, and stuff
		 * it into the packet receiver.
		 */
		if ((n = read(filedes, rdbuffer, sizeof(rdbuffer))) < 0) {
			perror("redis_tx: read");
			exit(1);
		}
		for (i = 0; i < n; i++)
			fp_indata(rdbuffer[i]);
		/*
		 * Process any packets on the receive queue.
		 */
		while ((fp = fp_receive()) != NULL) {
			if (fp->cmd & FONZ_RESPONSE)
				sprintf(wrbuffer, "[%u,%u,%u]", fp->cmd & 0xff, fp->arg1 & 0xff, fp->arg2 & 0xff);
			else
				sprintf(wrbuffer, "%u", fp->cmd & 0xff);
			if (txlfp != NULL)
				fprintf(txlfp, "< {%s}\n", wrbuffer);
			redisCommand(ctxt, "PUBLISH fonz-in %s", wrbuffer);
			fp_free(fp);
		}
	}
}

/*
 * Responsible for subscribing to the Redis "fonz-out" channel, receiving
 * PUB messages, assembling packets and forwarding them to the serial port.
 */
void
redis_receiver(char *hostname, int port)
{
	redisAsyncContext *ctxt;
	struct event_base *base;

	/*
	 * Initialize the Fonz packet transmitter, and the Redis connection.
	 */
	fp_init(0, 4);
	ctxt = redisAsyncConnect(hostname, port);
	if (ctxt->err) {
		fprintf(stderr, "?redis (rx) error: %s\n", ctxt->errstr);
		exit(1);
	}
	base = event_base_new();
	redisLibeventAttach(ctxt, base);
	/*
	 * Subscribe to the fonz-out channel, and use the libevent
	 * dispatcher to do the work.
	 */
	redisAsyncCommand(ctxt, pkt_recv, NULL, "SUBSCRIBE fonz-out");
	event_base_dispatch(base);
}

/*
 * Callback function for asynchronous Redis. Receive a block of data,
 * and if it looks like a message, process it and assemble a libfonz
 * packet for transmission serially.
 */
void
pkt_recv(redisAsyncContext *ctxt, void *reply, void *privdata)
{
	int n, cmd, opt1, opt2;
	char *cp, *json;
	unsigned char wrbuffer[32];
	redisReply *rp = reply;

	if (reply == NULL)
		return;

	if (rp->type != REDIS_REPLY_ARRAY || rp->elements != 3 || strcmp(rp->element[0]->str, "message") != 0)
		return;

	cmd = opt1 = opt2 = 0;
	json = rp->element[2]->str;
	if (rxlfp != NULL)
		fprintf(rxlfp, "> [%s]\n", json);
	if (*json == '[') {
		/*
		 * We have an array. If the array has a single element,
		 * treat it as a REQUEST command. If it has a comma,
		 * it's a RESPONSE command and the second (and possibly
		 * the third) arguments are the command options.
		 */
		if ((cp = strchr(++json, ']')) != NULL)
			*cp = '\0';
		if ((cp = strchr(json, ',')) != NULL) {
			*cp++ = '\0';
			cmd = atoi(json) | FONZ_RESPONSE;
			if ((cp = strchr(json = cp, ',')) != NULL) {
				*cp++ = '\0';
				opt1 = atoi(json);
				opt2 = atoi(cp);
			} else
				opt1 = atoi(json);
		} else
			cmd = atoi(json) & ~FONZ_RESPONSE;
	} else {
		/*
		 * A single integer argument. This is a REQUEST command.
		 */
		cmd = atoi(json) & ~FONZ_RESPONSE;
	}
	if (rxlfp != NULL) {
		if (cmd & FONZ_RESPONSE)
			fprintf(rxlfp, "CMD: 0x%02x, Opts=(%d/%d)\n", cmd, opt1, opt2);
		else
			fprintf(rxlfp, "CMD: 0x%02x\n", cmd);
	}
	fp_sendcmd(cmd, opt1, opt2, 0);
	while ((n = fp_outbuffer(wrbuffer, sizeof(wrbuffer))) > 0) {
		if (write(filedes, wrbuffer, n) != n) {
			perror("redis_rx: write");
			exit(1);
		}
	}
}

/*
 *
 */
void
usage()
{
	fprintf(stderr, "Usage: fonz_redis [-h <host>][-p <port>][-d <serial_dev>][-s baud][-v]\n");
	exit(2);
}

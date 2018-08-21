/**
	Custom Netcat Clone
	by Ethan Boswell
**/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

int port;

void helpMessage(char *);

void convertHostToIP(char *, char *);

void nc_listen(int, int, int);
void nc_connect(int, int, char *);
void basicConnection(char *, int);

int main(int argc, char *argv[]) {
	int totalFlags = 0, sock = 0, valread;
	char *message = "hey";
	char reply[1024];

	struct sockaddr_in address, server;

	int ipv4 = 0, ipv6 = 0, udpbroad = 0, socketDebug = 0, noStdin = 0, help = 0, listenIndefinite = 0, listenInstead = 0, randomly = 0, md5sig = 0, telpos = 0, unixdomain = 0, udpused = 0, verbose = 0, scanning = 0, lineending = 0;

	int i, length1 = 0, length2 = 0, interval = 0, src_port = 0, seconds = 0, timeout = 0;

	char flag, *proxy_username = NULL, *source = NULL, *toskeyword = NULL, *rtable = NULL, *proxy_protocol = NULL, *proxy_address = NULL, *destination = NULL;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s [-46bCDdhjklnrStUuvZz] [-I length] [-i interval] [-O length]\n [-P proxy_username] [-p source_port] [-q seconds] [-s source]\n [-T toskeyword] [-V rtable] [-w timeout] [-X proxy_protocol]\n [-x proxy_address[:port]] [destination] [port]\n", argv[0]);

		exit(0);
	}

	opterr = 0;

	memset(&server, '0', sizeof(server));

	while ((flag = getopt(argc, argv, "46bDdhklnrStUuvzCIiOPpqsTVwXx")) != -1) {
		switch (flag) {
			case '4': ipv4 = 1; break;
			case '6': ipv6 = 1; break;
			case 'b': udpbroad = 1; break;
			case 'C': lineending = 1; break;
			case 'D': socketDebug = 1; break;
			case 'd': noStdin = 1; break;
			case 'h': help = 1; break;
			case 'k': listenIndefinite = 1; break;
			case 'l': listenInstead = 1; break;
			case 'r': randomly = 1; break;
			case 'S': md5sig = 1; break;
			case 't': telpos = 1; break;
			case 'U': unixdomain = 1; break;
			case 'u': udpused = 1; break;
			case 'v': verbose = 1; break;
			case 'z': scanning = 1; break;

			case 'I': length1 = atoi(optarg); break;
			case 'i': interval = atoi(optarg); break;
			case 'O': length2 = atoi(optarg); break;
			case 'P': proxy_username = optarg; break;
			case 'p': src_port = atoi(optarg); break;
			case 'q': seconds = atoi(optarg); break;
			case 's': source = optarg; break;
			case 'T': toskeyword = optarg; break;
			case 'V': rtable = optarg; break;
			case 'w': timeout = atoi(optarg); break;
			case 'X': proxy_protocol = optarg; break;
			case 'x': proxy_address = optarg; break;

			case '?': {
				fprintf(stderr, "Usage: %s [-46bCDdhjklnrStUuvZz] [-I length] [i interval] [-O length]\n [-P proxy_username] [-p source_port] [-q seconds] [-s source]\n [-T toskeyword] [-V rtable] [-w timeout] [-X proxy_protocol]\n [-x proxy_address[:port]] [destination] [port]\n", argv[0]);

				exit(0);
			} break;

			default: abort(); break;
		}

		totalFlags++;
	}

	if (help == 1) helpMessage(argv[0]);

	destination = argv[argc - 2];
	port = atoi(argv[argc - 1]);

	printf("%s\n", destination);

	if (listenIndefinite == 1 && listenInstead == 0) {
		fprintf(stderr, "Error: -k option doesn\'t work without -l option.\n");
		exit(EXIT_FAILURE);
	}

	if (listenInstead == 1) {
		nc_listen(ipv6, port, listenIndefinite);
	}

	if (totalFlags == 0) basicConnection(destination, port);

	return 0;
}

void helpMessage(char *a) {
	printf("usage: %s [-46bCDdhjklnrStUuvZz] [-I length] [-i interval] [-O length]\n", a);
	printf("  [-P proxy_username] [-p source_port] [-q seconds] [-s source]\n");
	printf("  [-T toskeyword] [-V rtable] [-w timeout] [-X proxy_protocol]\n");
	printf("  [-x proxy_address[:port]] [destination] [port]\n");
	printf("Command Summary:\n");
	printf("	-4		Use IPv4\n");
	printf("	-6		Use IPv6\n");
	printf("	-b		Allow broadcast\n");
	printf("	-C		Send CRLF as line-ending\n");
	printf("	-D		Enable the debug socket option\n");
	printf("	-d		Detach from stdin\n");
	printf("	-h		This help text\n");
	printf("	-I length	TCP receive buffer length\n");
	printf("	-i secs		Delay interval for lines sent, ports scanned\n");
	printf("	-j		Use jumbo frame\n");
	printf("	-k		Keep inbound sockets open for multiple connects\n");
	printf("	-l		Listen mode, for inbound connects\n");
	printf("	-n		Suppress name/port resolutions\n");
	printf("	-O length	TCP send buffer length\n");
	printf("	-P proxyuser	Username for proxy authentication\n");
	printf("	-p port		Specify local port for remote connects\n");
        printf("	-q secs		quit after EOF on stdin and delay of secs\n");
	printf("	-r		Randomize remote ports\n");
	printf("	-S		Enable the TCP MD5 signature option\n");
	printf("	-s addr		Local source address\n");
	printf("	-T toskeyword	Set IP Type of Service\n");
	printf("	-t		Answer TELNET negotiation\n");
	printf("	-U		Use UNIX domain socket\n");
	printf("	-u		UDP mode\n");
	printf("	-V rtable	Specify alternate routing table\n");
	printf("	-v		Verbose\n");
	printf("	-w secs		Timeout for connects and final net reads\n");
	printf("	-X proto	Proxy protocol: \"4\", \"5\" (SOCKS) or \"connect\"\n");
	printf("	-x addr[:port]	Specify proxy address and port\n");
	printf("	-Z		DCCP mode\n");
	printf("	-z		Zero-I/O mode [used for scanning]\n");
	printf("Port numbers can be individual or ranges: lo-hi [inclusive]\n");
}

void *writeToDestination(void *args) {
	int sockfd = *((int *) args);
	char message[100000];

	while (1) {
		message[0] = '\0';

		fgets(message, 100000, stdin);

		if (port == 80) message[strlen(message)] = '\n';

		if ((write(sockfd, message, 100000)) < 0) {
			printf("\n\nOther party has disconnected. goodboye\n\n");
			break;
		}
	}

	free(args);
}

void *readFromDestination(void *args) {
	int sockfd = *((int *) args);
	char reply[100000];

	while (1) {
		reply[0] = '\0';

		if ((read(sockfd, reply, 100000)) < 0) {
			printf("\n\nOther party has disconnected. goodboye\n\n");
			break;
		}

		printf("%s", reply);
	}

	free(args);
}

void basicConnection(char *destination, int port) {
	int sockfd, result, *passSock = malloc(sizeof(*passSock));
	struct sockaddr_in server;
	
	pthread_t readThread, writeThread;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error creating socket.\n");
		exit(EXIT_FAILURE);
	}

	printf("%s\n", destination);

	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (inet_pton(AF_INET, destination, &server.sin_addr) < 0) {
		perror("Error due to invalid address.\n");
		exit(EXIT_FAILURE);
	}

	if ((result = connect(sockfd, (struct sockaddr *) &server, sizeof(server))) < 0) {
		perror("Error connecting to address.\n");
		exit(EXIT_FAILURE);
	}

	*passSock = sockfd;

	printf("I am not in the threads yet.\n");

	pthread_create(&writeThread, NULL, writeToDestination, passSock);
	pthread_create(&readThread, NULL, readFromDestination, passSock);

	pthread_join(writeThread, NULL);
	pthread_join(readThread, NULL);
/*
	while (1) {
		message[0] = '\0';

		fgets(message, 1024, stdin);
		if (port == 80) message[strlen(message)] = '\n';

		write(sockfd, message, strlen(message));

		if (read(sockfd, reply, 100000) < 0) {
			perror("Error reading reply.\n");
			exit(EXIT_FAILURE);
		}

		printf("%s", reply);
	}
*/
	printf("Out of threads now\n");

	close(sockfd);
}

void convertHostToIP(char *hostname, char *host) {
	struct addrinfo hints, *infoptr, *p;
	int result;

	hints.ai_family = AF_INET;

	result = getaddrinfo(hostname, NULL, &hints, &infoptr);

	if (result) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
		exit(1);
	}

	for (p = infoptr; p != NULL; p = p->ai_next) {
		getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
		puts(host);
	}

	freeaddrinfo(infoptr);
}

void nc_listen(int ipv6, int port, int indefinitely) {
	int opt = 1, server_sockfd, client_sockfd, server_len, client_len;
	int *passSock = malloc(sizeof(*passSock));
	struct sockaddr_in server;
	struct sockaddr_in client;
	char ch, text[1024], response[1024];

	pthread_t readThread, writeThread;

	if (ipv6 == 1) server.sin_family = AF_INET6;
	else server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	if ((server_sockfd = socket(server.sin_family, SOCK_STREAM, 0)) < 0) {
		perror("\n Socket creation error.\n");
		exit(EXIT_FAILURE);
	}

	server_len = sizeof(server);
	client_len = sizeof(client);
/*
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
							    &opt, sizeof(opt)) < 0) {
		fprintf(stderr, "Error: setsockopt failed for server.\n");
		exit(EXIT_FAILURE);
	}
*/
	bind(server_sockfd, (struct sockaddr *) &server, server_len);

	if (listen(server_sockfd, 5) < 0) {
		fprintf(stderr, "Error listening on server.\n");
		exit(EXIT_FAILURE);
	}

	printf("Listening on port %d...\n", port);

	if ((client_sockfd = accept(server_sockfd, (struct sockaddr *) &client, &client_len)) < 0) {
		fprintf(stderr, "Error on accepting client.\n");
		exit(EXIT_FAILURE);
	}

	*passSock = client_sockfd;

	if (indefinitely == 1) {
		while (1) {
			pthread_create(&writeThread, NULL, writeToDestination, passSock);
			pthread_create(&readThread, NULL, readFromDestination, passSock);

			pthread_join(writeThread, NULL);
			pthread_join(readThread, NULL);

			if ((client_sockfd = accept(server_sockfd, (struct sockaddr *) &client, &client_len)) < 0) {
				fprintf(stderr, "Error on accepting client.\n");
				exit(EXIT_FAILURE);
			}

			*passSock = client_sockfd;
		}
	} else {
		pthread_create(&writeThread, NULL, writeToDestination, passSock);
		pthread_create(&readThread, NULL, readFromDestination, passSock);

		pthread_join(writeThread, NULL);
		pthread_join(readThread, NULL);
	}
}


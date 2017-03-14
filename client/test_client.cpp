#include <iostream>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "DNSMsg.h"

using namespace std;

#define MAX_QLEN 120
#define MAX_RLEN 220


int sendall(int s, char *buf, int *len) {
	int total = 0;
	int bytesleft = *len;
	int n;

	while (total < *len) {
		n = send(s, buf + total, bytesleft, 0);
		if (n == - 1) { break; }
		total += n;
		bytesleft -= n;
	}

	*len = total;

	return n == -1 ? -1 : 0;
}

void get_ip(char *server_name, char *port_num, char *ip_from_dns) {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(server_name, port_num, &hints, &servinfo)) != 0) {
		perror("getaddrinfo");
		abort();
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client:connect");
			continue;
		}
		break;
	}

	if (p == NULL) {
		perror("client: fail to connect\n");
		abort();
	}

	freeaddrinfo(servinfo);

	DNSHeader header;
	DNSQuestion question;
	DNSRecord record;
	char qmsg[MAX_QLEN];
	char rmsg[MAX_RLEN];

	header.ID = 1;
	header.QR = 0;
	header.OPCODE = 1;
	header.AA = 0;
	header.TC = 0;
	header.RD = 0;
	header.RA = 0;
	header.Z = 0;
	header.RCODE = 0;
	header.QDCOUNT = 1;
	header.ANCOUNT = 0;
	header.NSCOUNT = 0;
	header.ARCOUNT = 0;
	strcpy(question.QNAME, "video.cse.umich.edu");
	question.QTYPE = 1;
	question.QCLASS = 1;


	to_qmsg(qmsg, &header, &question);

	int len = MAX_QLEN;
	if (sendall(sockfd, qmsg, &len) == -1) {
		cout << "Error on sendall" << endl;
		exit(1);
	}

	int bytesRecvd = 0;
	int numBytes;
	while(bytesRecvd < MAX_RLEN) {
		numBytes = recv(sockfd, rmsg + bytesRecvd, MAX_RLEN - bytesRecvd, 0);
		if (numBytes < 0) {
			cout << "Error receiving bytes" << endl;
			exit(1);
		}
		bytesRecvd += numBytes;
	}
	
	parse_rmsg(rmsg, &header, &record);

	if (header.RCODE == 0) {
		cout << "client data" << record.RDATA << endl;
		cout << (int)record.RDATA[0] << endl;
		cout << (int)record.RDATA[1] << endl;
		cout << (int)record.RDATA[2] << endl;
		cout << (int)record.RDATA[3] << endl;
		parse_ip(record.RDATA, ip_from_dns);
		cout << ip_from_dns << endl;
	}


	close(sockfd);
}

int main(int argc, char* argv[])
{
	char ip[16];
	get_ip(argv[1], argv[2], ip);
	cout << ip << endl;
	return 0;
}
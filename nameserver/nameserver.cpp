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
#include <signal.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <queue>
#include "DNSMsg.h"

using namespace std;

#define MAX_QLEN 120
#define MAX_RLEN 220

#define BACKLOG 20

struct path {
	int id;
	int cost; // cost from client to the node
	path(int i, int c) : id(i), cost(c) {}
	path() {}
};

struct comp {
	bool operator()(path p1, path p2) {
		return p1.cost > p2.cost;
	}
};

class round_robin {
	vector<string> ip_addrs;
	int idx;

public:
	void init(char * file) {
		idx = 0;

		string ip_addr;
		ifstream fin;
		fin.open(file);
		if (!fin.is_open()) {
			cout << "open failed" << endl;
			exit(1);
		}
		while (fin >> ip_addr) {
			ip_addrs.push_back(ip_addr);
		}
	}

	string get_ip() {
		string ip = ip_addrs[idx];
		idx = (idx + 1) % (int) ip_addrs.size();
		return ip;
	}
};

class geo_based {
	vector<string> ips;
	vector<string> types;
	unordered_map<string, int> id_table;
	unordered_map<int, vector<pair<int, int>>> network; // first is id, second is cost

public:
	void init(char * file) {
		int num_nodes, num_links, id, id1, id2, cost;
		string ip, type, tok;

		ifstream fin;
		fin.open("geo_dist.txt");
		if (!fin.is_open()) {
			cout << "open failed" << endl;
			exit(1);
		}
		
		fin >> tok >> num_nodes;
		for (int i = 0; i < num_nodes; i++) {
			fin >> id >> type >> ip;
			ips.push_back(ip);
			types.push_back(type);
			id_table[ip] = i;
		}

		fin >> tok;
		fin >> num_links;
		for (int i = 0; i < num_links; i++) {
			fin >> id1 >> id2 >> cost;
			network[id1].push_back(make_pair(id2, cost));
			network[id2].push_back(make_pair(id1, cost));
		}
	}

	string get_ip(string client_ip) {
		priority_queue<path, vector<path>, comp> pq;
		vector<bool> visited(ips.size(), false);
		int start_id = id_table[client_ip];
		path start(start_id, 0);
		path cur_path, next_path;

		pq.push(start);
		while (!pq.empty()) {
			cur_path = pq.top();
			pq.pop();
			if (visited[cur_path.id]) {
				continue;
			}
			visited[cur_path.id] = true;

			if (types[cur_path.id] == "SERVER") {
				return ips[cur_path.id];
			}

			for (pair<int, int> next_node : network[cur_path.id]) {
				 // avoid clients and visited switches added to the queue
				if (visited[next_node.first] || types[next_node.first] == "CLIENT") {
					continue;
				}
				next_path.id = next_node.first;
				next_path.cost = cur_path.cost + next_node.second;
				pq.push(next_path);
			}
		}
		return ""; // sever is not found, return an empty string
	}
};





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

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main (int argc, char *argv[]) {
	ofstream fout;
	fout.open(argv[1]);

	round_robin r;
	geo_based g;
	bool is_geo = *argv[3] - '0';
	if (is_geo) {
		g.init(argv[4]);
	} else {
		r.init(argv[4]);
	}

	int sockfd, clientsd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	char ipstr[INET6_ADDRSTRLEN];
	int rv, received_total, received_bytes, len;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, argv[2], &hints, &servinfo)) != 0) {
		perror("getaddrinfo");
		abort();
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			abort();
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);

	if (p == NULL) {
		perror("failed to bind");
		abort();
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	while (true) {
		sin_size = sizeof(their_addr);
		clientsd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (clientsd == -1) {
			perror("accept");
			abort();
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			ipstr, sizeof(ipstr));
		string client_ip(ipstr);
		
		// check all new sockets
		int bytesRecvd, numBytes;
		bytesRecvd = 0;
		char qmsg[MAX_QLEN];
		while(bytesRecvd < MAX_QLEN) {
			numBytes = recv(clientsd, qmsg + bytesRecvd, MAX_QLEN - bytesRecvd, 0);
			if (numBytes < 0) {
				cout << "Error receiving bytes" << endl;
				exit(1);
			}
			bytesRecvd += numBytes;
		}
		DNSHeader header;
		DNSQuestion question;
		parse_qmsg(qmsg, &header, &question);

		char rmsg[MAX_RLEN];
		string response_ip;

		header.QR = 1; // reuse header
		header.AA = 1;

		if (strcmp(question.QNAME, "video.cse.umich.edu") != 0) {
			header.RCODE = 3;
			header.ANCOUNT = 0;
			int i = 0;
			header_to_msg(rmsg, &header, i);
		} else {
			if (is_geo) {
				response_ip = g.get_ip(client_ip);
			} else {
				response_ip = r.get_ip();
			}
			
			if (response_ip.empty()) {
				header.RCODE = 3;
				header.ANCOUNT = 0;
				int i = 0;
				header_to_msg(rmsg, &header, i);
			} else {
				header.RCODE = 0;
				header.ANCOUNT = 1;

				DNSRecord record;
				strcpy(record.NAME, question.QNAME);
				record.TYPE = 1;
				record.CLASS = 1;
				record.TTL = 0;
				record.RDLENGTH = 4;
				ip_to_data(record.RDATA, response_ip);
		
				to_rmsg(rmsg, &header, &record);
			}
		}
		int len = MAX_RLEN;
		if (sendall(clientsd, rmsg, &len) == -1) {
			cout << "Error on sendall" << endl;
			exit(1);
		}

		close(clientsd);

		// write log
		fout << client_ip << '\t' << question.QNAME << '\t' << response_ip << endl;
		cout << client_ip << '\t' << question.QNAME << '\t' << response_ip << endl;
	}

	close(sockfd);
	fout.close();
	return 0;
}
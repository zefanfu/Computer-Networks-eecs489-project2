#include <iostream>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/select.h>
#include <vector>
#include <algorithm>
#include <cassert>
#include <errno.h>
#include <ctime>

typedef struct client_data
{
	char http_request[8192];
	char chunk_name[50]; 
	char* server_ip; 	
	client_data* server_pointer;
	std::vector<int> bitrate_vector;
	struct timeval start_receive;
	struct timeval end_receive;
	double average_throughput;
	int server_fd;
	int is_server;
	int content_length;
	int total_received;
	int request_bitrate;
	bool is_f4m;
	bool is_chunk;
	bool first_f4m;
}client_data; 

typedef struct client_data_fd
{
	client_data* cli_data;
	int fd;

}client_data_fd; 



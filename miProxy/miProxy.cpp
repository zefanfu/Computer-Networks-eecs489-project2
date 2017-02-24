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
#include <sys/time.h> 
#include <errno.h>
#include <cstdio>
#include "miProxy.h"
//#include "DNSMsg.h"

int main(int argc, char *argv[])
{
	int buffer_size=8192;
	char *server_name;
	FILE *pFile;
	if (argc >=6)
    {
    	bool www_ip=0;
    	if (argc ==6) //www-ip is not provided
    	{
    		char video_name[] = "video.cse.umich.edu";
    		server_name=video_name;
    	}
    	else if (argc ==7)
    	{
    		server_name=argv[6];
    		www_ip=1;
    	}
    	else
    	{
    		std::cout << "Error: too many arguments\n";
		    return -1;
    	}
    	char *log_path=argv[1];
    	double alpha=atof(argv[2]);
    	int server_port=atoi(argv[3]);
    	char *dns_ip=argv[4]; 
    	int dns_port=atoi(argv[5]);

    	// init socket and listen
		// create a socket
		int browser_listen_sd;
		struct sockaddr_in sever_addr, client_addr;
		if ((browser_listen_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
		{
			std::cout << "Error in opening TCP socket\n";
			return -1;
		}
		memset(&sever_addr, 0, sizeof (sever_addr));
		sever_addr.sin_family = AF_INET;
		sever_addr.sin_addr.s_addr = INADDR_ANY;
		sever_addr.sin_port = htons(server_port);
		//label the socket with a port 
		if (bind(browser_listen_sd, (struct sockaddr *) &sever_addr, sizeof (sever_addr)) < 0) 
		{
			std::cout << "Cannot bind socket to address\n";
			return -1;
		}
		//listen the port
		if (listen(browser_listen_sd, 10) < 0) 
		{
			std::cout << "Cannot bind socket to address\n";
			std::cout << strerror(errno) << std::endl;
			return -1;
		}
		memset(&client_addr, 0, sizeof (client_addr));		
		socklen_t client_addr_len = sizeof(client_addr);
		int ip_sd,dns_sd;
		//dns socket
		if ((dns_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) 
		{
			std::cout << "Error on ip socket" << std::endl;
			return -1;
		}
		fd_set read_set;
		// Keep track of each file descriptor accepted
		std::vector<client_data_fd*> fds;
		int client_sd;
		while(true)
		{
			// Set up the read_set
			int maxfd = 0;
			FD_ZERO(&read_set);
			FD_SET(browser_listen_sd, &read_set);
			for(int i = 0; i < (int) fds.size(); ++i)
			{
				FD_SET(fds[i]->fd, &read_set);
				if (maxfd<fds[i]->fd)
				{
					maxfd=fds[i]->fd;
				}
			}
			maxfd = std::max(maxfd, browser_listen_sd);
			// maxfd + 1 is important
			int number_fd = select(maxfd + 1, &read_set, NULL, NULL, NULL);
			if(number_fd== -1)
			{
				std::cout << "Error on select" << std::endl;
			}
			if(FD_ISSET(browser_listen_sd, &read_set))
			{
				client_sd = accept(browser_listen_sd, (struct sockaddr *) &client_addr, &client_addr_len);
				if(client_sd == -1)
				{
					std::cout << "Error on accept" << std::endl;
				}
				else
				{
					// add client to vector
					if (www_ip==0)
					{
						/*resolve video url*/
						// as client
						struct sockaddr_in dns_addr;
						memset(&dns_addr, 0, sizeof(dns_addr));
					    dns_addr.sin_family  = AF_INET;
					    dns_addr.sin_port    = htons(dns_port);
					    dns_addr.sin_addr.s_addr = inet_addr(dns_ip);
					    if (connect(dns_sd, (struct sockaddr *) &dns_addr, sizeof (dns_addr)) < 0) 
						{
							std::cout << "Error on dns connect" << std::endl;
							return -1;
						}

						/*DNSHeader header;
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
							cout << record.RDATA << endl;
							strcpy(server_name, record.RDATA);
						}*/

						
					    /*send things to dns*/
					    //to_qmsg()
					}
					/*connect to www_ip*/
					struct addrinfo hints; 
				    struct addrinfo* result; 
				    memset(&hints, 0, sizeof (hints));
				    hints.ai_family = AF_INET;    
				    hints.ai_socktype = SOCK_STREAM; 
				    hints.ai_flags = AI_PASSIVE;     
				    if (getaddrinfo(server_name, "80", &hints, &result) < 0)
				    {
				        std::cout << "Error on getaddrinfo" << std::endl;
						return -1;
				    }
				    int cli_2_server_sd;
				    if((cli_2_server_sd = socket(result->ai_family, result->ai_socktype,result->ai_protocol))<0)
				    {
				        std::cout << "Error on client to server socket" << std::endl;
						return -1;
				    }
					int c_return;
				    if (c_return=connect (cli_2_server_sd, result->ai_addr, result->ai_addrlen) == -1)
				    {
				        std::cout << "Error on client to server connect" << std::endl;
						return -1;
				    }
					ip_sd=cli_2_server_sd;
					client_data* ip_client_data;
					ip_client_data=(client_data*)malloc(sizeof(struct client_data));
					memset(ip_client_data->http_request,  0, buffer_size);		
					memset(ip_client_data->chunk_name,  0, 50);
				    ip_client_data->server_ip=server_name;
				    ip_client_data->server_fd=client_sd;
				    ip_client_data->is_server=1;
				    ip_client_data->average_throughput=0.0;
				    ip_client_data->first_f4m=1;
				    ip_client_data->request_bitrate=0;				
				    client_data_fd* ip_client_data_fd;
				    ip_client_data_fd=(client_data_fd*)malloc(sizeof(struct client_data_fd));					
				    ip_client_data_fd->cli_data=ip_client_data;
				    ip_client_data_fd->fd=ip_sd;
					client_data* client_client_data;
					client_client_data=(client_data*)malloc(sizeof(struct client_data));
					memset(client_client_data->http_request,  0, buffer_size);
					memset(client_client_data->chunk_name,  0, 50);
				    client_client_data->server_fd=ip_sd;
				    client_client_data->is_server=0;
				    client_client_data->server_pointer=ip_client_data;
				    client_client_data->average_throughput=0.0;
				    client_client_data->first_f4m=1;
				    ip_client_data->request_bitrate=0;
				    client_client_data->server_ip=client_client_data->server_pointer->server_ip;
				    client_data_fd* client_client_data_fd;
				    client_client_data_fd=(client_data_fd*)malloc(sizeof(struct client_data_fd));						
				    client_client_data_fd->cli_data=client_client_data;
				    client_client_data_fd->fd=client_sd;
				    ip_client_data->server_pointer=client_client_data;
					fds.push_back(ip_client_data_fd);
					fds.push_back(client_client_data_fd);
				}
			}
			for(int i = 0; i < (int) fds.size(); ++i)
			{
				if(FD_ISSET(fds[i]->fd, &read_set))
				{
					char buffer[buffer_size];
					bzero(buffer,buffer_size);
					int blen=buffer_size;
					char* header;
					char* body; 
					char* content_length_start;
					char* content_length_end;
					int bytesRecvd = recv(fds[i]->fd, &buffer, blen, 0);
					if(bytesRecvd < 0)
					{
						std::cout << "Error recving bytes" << std::endl;
						std::cout << strerror(errno) << std::endl;
						exit(1);
					}
					else if(bytesRecvd == 0)
					{
						std::cout << "Connection closed" << std::endl;
						fds.erase(fds.begin() + i);
						i=i-1;
					}
					//send
					if (!fds[i]->cli_data->is_server) // client
					{
						//check type
						char *f4m;
						char *chunk;
						chunk=strcasestr(buffer,"-Frag");
						f4m=strcasestr(buffer,".f4m");
						if (f4m!=NULL) // f4m type
						{
							fds[i]->cli_data->is_f4m=1;
							fds[i]->cli_data->server_pointer->is_f4m=1;
							fds[i]->cli_data->is_chunk=0;
							fds[i]->cli_data->server_pointer->is_chunk=0;
							memcpy(fds[i]->cli_data->http_request,buffer,bytesRecvd);
						}
						else if (chunk!=NULL) // chunk type
						{
							fds[i]->cli_data->is_chunk=1;
							fds[i]->cli_data->server_pointer->is_chunk=1;
							fds[i]->cli_data->is_f4m=0;
							fds[i]->cli_data->server_pointer->is_f4m=0;
							//select bitrate
							int bitrate_browser;
							if(fds[i]->cli_data->server_pointer->bitrate_vector.size()==0)
							{
								std::cout << "no bitrate in vector" <<std::endl;
							}
							else //vector has element
							{ 
								float bitrate_constraint=(fds[i]->cli_data->server_pointer->average_throughput)/1.5;
								if (fds[i]->cli_data->server_pointer->bitrate_vector[0]>=bitrate_constraint)
								{	
									bitrate_browser=fds[i]->cli_data->server_pointer->bitrate_vector[0];								 		
								}
								else
								{	
									for (int j = 0; j < fds[i]->cli_data->server_pointer->bitrate_vector.size(); ++j)
									{
										if (fds[i]->cli_data->server_pointer->bitrate_vector[j]<bitrate_constraint)
										{
											bitrate_browser=fds[i]->cli_data->server_pointer->bitrate_vector[j];
										}
										else 
										{											
											break;
										}
									}
								}
							}
							fds[i]->cli_data->request_bitrate=bitrate_browser;
							char * replace_bitrate_pointer_left;
							char * replace_bitrate_pointer_right;
							char * chunk_name_pointer_right;
							replace_bitrate_pointer_left=strstr(buffer,"Seg");
							replace_bitrate_pointer_right=strstr(buffer,"Seg");
							while(*replace_bitrate_pointer_left!='/')
							{
								replace_bitrate_pointer_left=replace_bitrate_pointer_left-1;	
							}
							replace_bitrate_pointer_left=replace_bitrate_pointer_left+1;
							chunk_name_pointer_right=strstr(replace_bitrate_pointer_right," ");
							strncpy(fds[i]->cli_data->server_pointer->chunk_name,replace_bitrate_pointer_left,(chunk_name_pointer_right-replace_bitrate_pointer_left));
							*replace_bitrate_pointer_left='\0';
							char buffer_again[buffer_size];
							bzero(buffer_again,buffer_size);
							snprintf(buffer_again,buffer_size,"%s%d%s",buffer,bitrate_browser,replace_bitrate_pointer_right);
							memcpy(buffer,buffer_again,strlen(buffer_again));							
							//timer start
							gettimeofday(&fds[i]->cli_data->start_receive, NULL);
							fds[i]->cli_data->server_pointer->start_receive=fds[i]->cli_data->start_receive;
						}
						else
						{
							fds[i]->cli_data->is_chunk=0;
							fds[i]->cli_data->server_pointer->is_chunk=0;
							fds[i]->cli_data->is_f4m=0;
							fds[i]->cli_data->server_pointer->is_f4m=0;
						}

						if (send(fds[i]->cli_data->server_fd, buffer,bytesRecvd , 0)== -1)
						{
	                    	std::cout << "error in browser send to server" <<std::endl;
	                        return -1;
	                    }
	                }
	                else // server
	                {
	                	if (strstr(buffer,"\r\n\r\n")!=NULL)//has header
	                	{
	                		body=strstr(buffer,"\r\n\r\n");
	                		//parse
		                	content_length_start=strstr(buffer,"Content-Length: ");
		                	content_length_end=strstr(content_length_start,"\r\n");
		                	char content_length[25];
		                	strncpy(content_length,(content_length_start+16),(content_length_end-content_length_start-16));
		                	content_length[(content_length_end-content_length_start-16)]='\0';		   
		                	fds[i]->cli_data->content_length=atoi(content_length);
		                	fds[i]->cli_data->total_received=bytesRecvd;
		                	if (fds[i]->cli_data->is_f4m)//f4m file
		                	{
		                		if (fds[i]->cli_data->first_f4m==1)
		   						{
		   							if (fds[i]->cli_data->total_received>fds[i]->cli_data->content_length)// all are received
			                		{
			                			//parse f4m
			                			char* xml_pointer;
			                		 	char* media_pointer;
			                		 	char* bitrate_pointer;
			                			xml_pointer=strstr(body,"<?xml");
			                			media_pointer=strstr(xml_pointer,"<media");
			                			while(media_pointer!=NULL)
			                			{
			                				bitrate_pointer=strstr(media_pointer,"bitrate=");
			                				if (bitrate_pointer!=NULL)
			                				{
			                					int bitrate;
			                					if (std::sscanf(bitrate_pointer,"bitrate=\"%d\"", &bitrate)>=1)
			                					{
			                						//insert into vector			                														
			                						fds[i]->cli_data->bitrate_vector.push_back(bitrate);
			                					}
			                				}
			                				media_pointer=strstr(media_pointer+1,"<media");			                							                				
			                			}			  
			                			fds[i]->cli_data->first_f4m=0;              			
			                		}
			                		char* replace_f4m_pointer;
		                			replace_f4m_pointer=strstr(fds[i]->cli_data->server_pointer->http_request,".f4m");
		                			int total_len=strlen(fds[i]->cli_data->server_pointer->http_request);
		                			int before_len=replace_f4m_pointer-fds[i]->cli_data->server_pointer->http_request;
		                			int after_len=total_len-before_len;
		                			memmove(replace_f4m_pointer+12,replace_f4m_pointer+5,after_len);
		                			memmove(replace_f4m_pointer,"_nolist.f4m ",strlen("_nolist.f4m "));
		                			if (send(fds[i]->fd, fds[i]->cli_data->server_pointer->http_request,strlen(fds[i]->cli_data->server_pointer->http_request), 0)== -1)						
									{
				                    	std::cout << "error in server send to browser" <<std::endl;
				                        return -1;
				                    }
		                		}
		                		else
		                		{
		                			if (send(fds[i]->cli_data->server_fd, buffer,bytesRecvd , 0)== -1)						
									{
				                    	std::cout << "error in server send to browser" <<std::endl;
				                        return -1;
				                    }		                			
		                		}		                			                		
		                	}
		                	else //other html header response
		                	{
		                		if (send(fds[i]->cli_data->server_fd, buffer,bytesRecvd , 0)== -1)						
								{
			                    	std::cout << "error in server send to browser" <<std::endl;
			                        return -1;
			                    }
		                	}
	                	}
	                	else
	                	{   
	                		if (fds[i]->cli_data->is_chunk==1)
	                		{		               		
		                		//chunk body data
		                		fds[i]->cli_data->total_received+=bytesRecvd;
		                		if (fds[i]->cli_data->total_received>fds[i]->cli_data->content_length)// all are received
		                		{
		                			gettimeofday(&fds[i]->cli_data->end_receive, NULL);
		                			double elapsedTime = (fds[i]->cli_data->end_receive.tv_sec - fds[i]->cli_data->start_receive.tv_sec);
			    					elapsedTime += (fds[i]->cli_data->end_receive.tv_usec - fds[i]->cli_data->start_receive.tv_usec) / 1000000.0;
		                			double new_throughput=(fds[i]->cli_data->content_length)*8/(elapsedTime*1000);//kb/s
		                			fds[i]->cli_data->average_throughput=alpha*new_throughput+(fds[i]->cli_data->average_throughput)*(1-alpha);
		                			//log here
		                			pFile=fopen(log_path,"a");
		                			fprintf(pFile, "%f %f %f %d %s %s \n", elapsedTime,new_throughput,fds[i]->cli_data->average_throughput,fds[i]->cli_data->server_pointer->request_bitrate,fds[i]->cli_data->server_pointer->server_ip, fds[i]->cli_data->chunk_name);
		                			fclose(pFile);
		                		}
		                		if (send(fds[i]->cli_data->server_fd, buffer,bytesRecvd , 0)== -1)						
								{
			                    	std::cout << "error in server send to browser" <<std::endl;
			                        return -1;
			                    }
		                	}
		                	else
		                	{
		                		if (send(fds[i]->cli_data->server_fd, buffer,bytesRecvd , 0)== -1)						
								{
			                    	std::cout << "error in server send to browser" <<std::endl;
			                        return -1;
			                    }
		                	}
	                	}
	                }
				}
			}
		}
    }
  	else
    {
      	//printf("Error: missing arguments\n");
      	std::cout << "Error: missing arguments" << std::endl;
    }
    return 0;
}

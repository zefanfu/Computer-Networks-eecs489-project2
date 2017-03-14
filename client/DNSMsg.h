#include "DNSHeader.h"
#include "DNSQuestion.h"
#include "DNSRecord.h"
#include <string>


void parse_qmsg(char * msg, DNSHeader * header, DNSQuestion * question);
void parse_rmsg(char * msg, DNSHeader * header, DNSRecord * record);

int to_qmsg(char * msg, DNSHeader * header, DNSQuestion * question);
int to_rmsg(char * msg, DNSHeader * header, DNSRecord * record);

void header_to_msg(char * msg, DNSHeader * header, int &i);
void ip_to_data(char * rdata, std::string ip);
void parse_ip(char * rdata, char * ip);

// helper function
ushort msg_to_ushort(char *msg, int &i);
void msg_to_header(char * msg, DNSHeader * header, int &i);
void msg_to_name(char * msg, char * name, int &i);

void ushort_to_msg(char * msg, ushort value, int &i);
void name_to_msg(char * msg, char * name, int &i);
void head_to_msg(char * msg, DNSHeader * header, int &i);

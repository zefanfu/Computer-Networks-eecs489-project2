#include "DNSMsg.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

using namespace std;

ushort msg_to_ushort(char *msg, int &i) {
	return (ushort)(msg[i++] << 8) + (ushort)msg[i++];
}

void msg_to_header(char * msg, DNSHeader * header, int &i) {
	header->ID = msg_to_ushort(msg, i);
	header->QR = msg[i] >> 7;
	header->OPCODE = (msg[2] >> 3) & 0x0f;
	header->AA = (msg[i] >> 2) & 0x01;
	header->TC = (msg[i] >> 1) & 0x01;
	header->RD = msg[i] & 0x01;
	i++;
	header->RA = msg[i] >> 7;
	header->Z = (msg[i] >> 4) & 0x07;
	header->RCODE = msg[i] & 0x0f;
	i++;
	header->QDCOUNT = msg_to_ushort(msg, i);
	header->ANCOUNT = msg_to_ushort(msg, i);
	header->NSCOUNT = msg_to_ushort(msg, i);
	header->ARCOUNT = msg_to_ushort(msg, i);
}

void msg_to_name(char * msg, char * name, int &i) {
	int len = msg[i++];
	int name_idx = 0;
	while (len > 0) {
		strncpy(name + name_idx, msg + i, len);
		i += len;
		name_idx += len;
		name[name_idx++] = '.';
		len = msg[i++];
	}
	name[name_idx - 1] = '\0';
}

void parse_qmsg(char * msg, DNSHeader * header, DNSQuestion * question) {
	int i = 0;
	msg_to_header(msg, header, i);

	// parse QNAME
	msg_to_name(msg, question->QNAME, i);

	question->QTYPE = msg_to_ushort(msg, i);
	question->QCLASS = msg_to_ushort(msg, i);
}

void parse_rmsg(char * msg, DNSHeader * header, DNSRecord * record) {
	int i = 0;
	msg_to_header(msg, header, i);
	if (header->RCODE != 0) {
		return;
	}
	msg_to_name(msg, record->NAME, i);
	record->TYPE = msg_to_ushort(msg, i);
	record->CLASS = msg_to_ushort(msg, i);
	record->TTL = msg_to_ushort(msg, i);
	record->RDLENGTH = msg_to_ushort(msg, i);
	// strncpy(record->RDATA, msg + i, record->RDLENGTH);
	for (int j = 0; j < record->RDLENGTH; j++) {
		record->RDATA[j] = *(msg + i + j);
	}
}

void parse_ip(char * rdata, char * ip) {
	char buffer[8];
	memset(buffer, '\0', 4);
	unsigned char d = rdata[0];
	sprintf(buffer,"%d", d);
	strcpy(ip, buffer);
	for (int i = 1; i < 4; i++) {
		strcat(ip, ".");
		memset(buffer, '\0', 4);
		d = rdata[i];
		sprintf(buffer,"%d", (int)d);
		strcat(ip, buffer);
	}
}






void ip_to_data(char * rdata, string ip) {
	int found = 0;
	int prev;
	for (int i = 0; i < 3; i++) {
		prev = found;
		found = ip.find('.', prev) + 1;
		rdata[i] = stoi(ip.substr(prev, found - prev));
	}
	rdata[3] = stoi(ip.substr(found));
}

void ushort_to_msg(char * msg, ushort value, int &i) {
	msg[i++] = value >> 8;
	msg[i++] = value & 0xff;
}
void name_to_msg(char * msg, char * name, int &i) {
	int label_len = 0; // len of one label
	int name_idx = 0; //
	while (name[name_idx] != '\0') {
		while (name[name_idx + label_len] != '.' && name[name_idx + label_len] != '\0') {
			label_len++;
		}
		msg[i++] = label_len;
		strncpy(msg + i, name + name_idx, label_len);
		i += label_len;
		name_idx += label_len + 1; 
		label_len = 0;
	}
	msg[i++] = 0; // LEN = 0
}

void header_to_msg(char * msg, DNSHeader * header, int &i) {
	ushort_to_msg(msg, header->ID, i);
	msg[2] = 0;
	msg[2] += header->QR << 7;
	msg[2] += header->OPCODE << 3;
	msg[2] += header->AA << 2;
	msg[2] += header->TC << 1;
	msg[2] += header->RD;
	msg[3] = 0;
	msg[3] += header->RA << 7;
	msg[3] += header->Z << 4;
	msg[3] += header->RCODE;
	i = 4;
	ushort_to_msg(msg, header->QDCOUNT, i);
	ushort_to_msg(msg, header->ANCOUNT, i);
	ushort_to_msg(msg, header->NSCOUNT, i);
	ushort_to_msg(msg, header->ARCOUNT, i);
}

// convert DNSHeader and DNSQuestion to msg string
// return the length of msg
int to_qmsg(char * msg, DNSHeader * header, DNSQuestion * question) {
	int i = 0;
	header_to_msg(msg, header, i);
	name_to_msg(msg, question->QNAME, i);
	ushort_to_msg(msg, question->QTYPE, i);
	ushort_to_msg(msg, question->QCLASS, i);
	return i;
 }

// convert DNSHeader and DNSRecord to msg string
// return the length of msg
int to_rmsg(char * msg, DNSHeader * header, DNSRecord * record) {
	int i = 0;
	header_to_msg(msg, header, i);
	name_to_msg(msg, record->NAME, i);
	ushort_to_msg(msg, record->TYPE, i);
	ushort_to_msg(msg, record->CLASS, i);
	ushort_to_msg(msg, record->TTL, i);
	ushort_to_msg(msg, record->RDLENGTH, i);
	cout << "before cpy" << endl;
	cout << (int)record->RDATA[0] << endl;
	cout << (int)record->RDATA[1] << endl;
	cout << (int)record->RDATA[2] << endl;
	cout << (int)record->RDATA[3] << endl;
	for (int j = 0; j < record->RDLENGTH; j++) {
		*(msg + i + j) = record->RDATA[j];
	}
	// strncpy(msg + i, record->RDATA, record->RDLENGTH);
	cout << "server cpy data" << endl;
	cout << (int)*(msg + i + 0) << endl;
	cout << (int)*(msg + i + 1) << endl;
	cout << (int)*(msg + i + 2) << endl;
	cout << (int)*(msg + i + 3) << endl;
	return  i + record->RDLENGTH;
}




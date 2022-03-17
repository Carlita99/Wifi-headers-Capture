#ifndef _PCAP_HANDLER_H_
#define _PCAP_HANDLER_H_

#include "defines.h"
#include <pcap.h>

std::string mac2string(unsigned char mac[6]);
void process_pkts(u_char* user, const struct pcap_pkthdr *pkt, const u_char *bytes);

#endif //_PCAP_HANDLER_H_
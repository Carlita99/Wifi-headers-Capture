#include "defines.h"
#include "http.h"
#include "pcap_handler.h"
#include <iostream>
#include <pcap.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <csignal>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <cstring>
#include "config.h"

using namespace std;

string server_host{"127.0.0.1"};
string server_port{"3000"};

/*
 * \function get_mac_address returns network interface MAC address
 * \param ifname the name of the interface as visible with ip addr command
 * \return a string with the ASCII representation of the MAC address
 * */
string get_mac_address(string ifname) {
  struct ifreq s;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  char c_mac_addr[18] = "";
  unsigned char *buf = nullptr;

  strcpy(s.ifr_name, ifname.c_str());
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    buf = (unsigned char *) s.ifr_addr.sa_data;
    sprintf(c_mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
  } else {
    strcpy(c_mac_addr, "00:00:00:00:00:00");
  }
  close(fd);
  return string{c_mac_addr};
}

// The PCAP handler (global for handling by sigint_handler)
pcap_t *pcap_hdl = nullptr;

/*
 * \function sigint_handler quits cleanly the program when issuing
 * Ctrl-C (SIGINT) to the program
 * */
void sigint_handler(int sig) {
  if (sig == SIGINT) {
    if (pcap_hdl)
      pcap_breakloop(pcap_hdl);
  }
}

/*
 * \function main
 * \TODO: implement the pcap handler in pcap_handler.cpp
 * \TODO: implement the send_samples function in http.cpp
 * \TODO: implement new options and use config singleton to store cfg values.
 * */
int main(int argc, char *argv[]) {
  string iface_name{""};
  int opt;
  opt = getopt(argc, argv, "i:");
  if (opt == 'i') {
    iface_name = optarg;
  }
  else if(opt == 'h') {
    server_host = optarg;
  } 
  else if(opt == 'p') {
    server_port = optarg;
  }
  std::cout << "Will connect to server at ";
  std::cout << server_host << ":";
  std::cout << server_port << std::endl;

  signal(SIGINT, sigint_handler);

  char errbuf[PCAP_ERRBUF_SIZE];

  pcap_hdl = pcap_open_live(iface_name.c_str(), BUFSIZ, 1, 100, errbuf);
  if (pcap_hdl == nullptr) {
    cout << "Could not open PCAP on interface" << iface_name << endl;
    return -1;
  }

  pcap_handler_user_data pcap_user_data;
  pcap_user_data.ap_mac_addr = get_mac_address(iface_name);
  cout << "Starting capture on interface " << iface_name
       << " (MAC: " << pcap_user_data.ap_mac_addr << ")" << endl;
  pcap_loop(pcap_hdl, -1, process_pkts, (u_char *) &pcap_user_data);

  return 0;
}

#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <vector>
#include <string>

struct _RSSISample {
  std::string mac_address; ///< Source MAC address
  int rssi;           ///< RSSI value (dBm)
  struct timeval ts;  ///< Frame timestamp
  int antenna;        ///< Antenna index (-1 if not specified)
};

using RSSISample = struct _RSSISample;
using RSSILog = std::vector<RSSISample>;

struct _pcap_handler_user_data {
  std::string ap_mac_addr;
  RSSILog samples;
};

using pcap_handler_user_data = struct _pcap_handler_user_data;

struct ieee80211_header
{
  u_short frame_control;
  u_short frame_duration;
  u_char address1[6];
  u_char address2[6];
  u_char address3[6];
  u_short sequence_control;
  u_char address4[6];
};

#endif // _DEFINES_H_

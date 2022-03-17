#include "pcap_handler.h"
#include "http.h"
#include <radiotap_iter.h>
#include <string>
#include <sys/time.h>
#include <ctime>


using namespace std;

RSSILog logsStorage;

string mac2string(unsigned char mac[6]) {
  char mac_c_str[18];
  sprintf(mac_c_str, "%02X:%02X:%02X:%02X:%02X:%02X\0", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return string{mac_c_str};
}

/*
 * \function process_pkts PCAP callback function
 * \param user a pointer (to be cast) to a RSSILog variable
 * \param pkt a pointer to a rtap header
 * \param bytes a pointer to the captured packet, starting by the radiotap header
 */
void process_pkts(u_char* user, const struct pcap_pkthdr *pkt, const u_char *bytes) {
  /*
   * TODO: for each packet, extract the source address, the RSSI value(s),
   * the antenna index when present, and get system time. Each RSSI goes
   * to one element in the user->samples vector.
   * After dealing with the packet, check the first vector element
   * timestamp against current time. If it is older than 1 second, send
   * the samples (call send_samples from http.h)
   * */
  // TODO: your code here

  //First, we have to convert the captured packet into an instance of the radio tap header structure (defined in radiotap.h file).
  auto radiotapHeader = (struct ieee80211_radiotap_header *) bytes;

  //Then, we have to get the version and check if it's radiotap (It should be equal to 0)
  if(radiotapHeader->it_version == 0){
    printf("It is radiotap");
    //We define now the radiotap iterator that will walk through the radiotap args (defined in radiotap_iter.h)
    struct ieee80211_radiotap_iterator* radiotapIterator;
    //We should now create and initialize this iterator (pcap_pkthdr is a struct inside pcap.h included in pcap.cpp)
    int failure = ieee80211_radiotap_iterator_init(radiotapIterator, radiotapHeader, pkt->caplen, nullptr);
    //ieee80211_radiotap_iterator_init returns an integer to know if the initialisation was succesffull.
    if(!failure){
      //Now, we have to define a variable for the wifi header using the ieee80211_header (defined in defines.h file)
      //The wifi header is in the bytes packet after the radioTap Header. That's why we add the length of it to bytes.
      auto wifiHeader = (struct ieee80211_header *) (bytes + radiotapIterator->_max_length);
      //It should be a Beacon frame : IEEE80211_STYPE_BEACON 0x0080
      if(wifiHeader->frame_control == 0x0080){
        //We can get the source address which is the second 6 bytes address in the wifi header.
        string sourceAddress = mac2string(wifiHeader->address2);
        int rssi;
        int antennaIndex;
        //Now, we iterate through all the args. 0 for success.
        while(ieee80211_radiotap_iterator_next(radiotapIterator) == 0){
          //The RSSI Value is the Antenna signal, it's in dbm and it is defines as IEEE80211_RADIOTAP_DB_ANTSIGNAL = 12 in radiotap.h
          if(radiotapIterator->this_arg_index == IEEE80211_RADIOTAP_DB_ANTSIGNAL){
            rssi = ((int) *(radiotapIterator->this_arg));
          } 
          //The Antenna index is the Antenna, it is defines as IEEE80211_RADIOTAP_ANTENNA = 11 in radiotap.h
          if(radiotapIterator->this_arg_index == IEEE80211_RADIOTAP_DB_ANTSIGNAL){
            antennaIndex = ((int) *(radiotapIterator->this_arg));
          } 
        } 
        //timeval is a structure defined in sys/time.h, and gettimeofday is a function that sets the current time
        struct timeval now;
        gettimeofday(&now,NULL);

        //Now we have to create an instance of the RSSISample in order to stor it in our logsStorage.
        RSSISample currentSample;
        currentSample.mac_address = sourceAddress;
        currentSample.rssi = rssi;
        currentSample.ts = now;
        currentSample.antenna = antennaIndex;

        logsStorage.push_back(currentSample);

        //Now we just have to send the samples and clean them from our logsStorage if the timestamp difference is more than 1 second
        // Only if there is values in the log (size > 0)
        time_t currentTime = time(0);
        if(currentTime - logsStorage[0].ts.tv_sec > 1 && logsStorage.size() > 0){
          send_samples(logsStorage, sourceAddress);
          logsStorage.clear();
        } 
      } 
    } 
  } 
}

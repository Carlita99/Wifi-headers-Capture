#include "http.h"
#include <Poco/Net/Net.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTMLForm.h>
#include <map>
#include <set>
#include <cmath>
#include <string> 

using namespace std;
using namespace Poco::Net;

void send_samples(RSSILog samples, string ap_mac_addr) {
  /*
   * TODO: Implement this function
   * It takes two parameters:
   * 	- samples with RSSI samples ordered by reception time
   * 	- ap_mac_addr the Raspberry Pi MAC address
   * 
   * It must send its MAC address as variable name ap
   * and a list of pairs DeviceMAC=RSSI where each DeviceMAC is unique
   * and RSSI are average RSSI values when multiple values exist for a
   * given DeviceMAC
   * 
   * The packet must be sent to http://localhost:8080/rssi
   * 
   * HTTP requests handling use Poco::Net API
   * */
  // TODO: your code here

  //First, we have to define the url of our request
  string url = "http://localhost:8080/rssi/ap=" + ap_mac_addr;

  //Now we have to add the parameters to this URL in the following format: ap=valueofAP&macAdd=AverageRSSI&...
  for(int i = 0; i < samples.size(); i++){
    int rssiSum = pow(10, samples[i].rssi/10);
    int count = 0;
    for(int j = i + 1; j < samples.size(); j++){
      if(samples[i].mac_address == samples[j].mac_address){
        rssiSum += pow(10, samples[j].rssi/10);
        count++;
        samples.erase(samples.begin() + j); 
      } 
    } 
    url += "&" + samples[i].mac_address + "=" + std::to_string(10*log10(rssiSum / count));
  } 

  //Now that we have our URL complete, we just have to handle the get request using Poco:Net API
   HTTPClientSession session("localhost", 8080);
   HTTPRequest request(HTTPRequest::HTTP_GET, url, HTTPMessage::HTTP_1_1);
   session.sendRequest(request);
}

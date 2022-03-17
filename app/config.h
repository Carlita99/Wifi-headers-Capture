#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <iostream>
#include <string>
//#include <gdal/ogr_geometry.h> // For later use

class Configuration {
  public:
    static Configuration *getInstance() { return instance; }
    const std::string &getServerHost()const { return server_host; }
    unsigned short getServerPort()const { return server_port; }
//    const OGRPoint &getMyLocation()const { return my_location; }
    void setServerHost(const std::string &srv) { server_host = srv; }
    void setServerPort(unsigned short p) { server_port = p; }
    void setMyLocationFromString(const std::string &loc);

    friend std::ostream &operator<<(std::ostream &os, const Configuration &me);

  private:
    std::string server_host{"127.0.0.1"};
    unsigned short server_port{5000};
//    OGRPoint my_location;
    static Configuration *instance;

    Configuration() {}
    Configuration(const Configuration &other) = delete;
};

#endif //_CONFIG_H_

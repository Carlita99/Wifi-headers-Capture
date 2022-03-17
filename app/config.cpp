#include "config.h"

using namespace std;

ostream &operator<<(ostream &os, const Configuration &me) {
  os << "Current configuration:" << endl;
//  os << "\tMy coordinates: (" << me.my_location.getX() << "," << me.my_location.getY() << 
//     "," << me.my_location.getZ() << ")" << endl;
  os << "\tServer host and port: " << me.server_host << ":" << me.server_port << endl;
  return os;
}

void Configuration::setMyLocationFromString(const string &loc) {
  // Split string according to , or ; separators
  string sep{","};
  cout << loc << endl;
  auto xy_sep = loc.find(sep);
  if (xy_sep == string::npos) {
    sep = ";";
    xy_sep = loc.find(sep);
    if (xy_sep == string::npos) return; // Not a valid coordinate
  }
  auto yz_sep = loc.find(sep, xy_sep+1);
  double x, y, z = 0.0;
  if (yz_sep == string::npos) { // No 2nd separator: only x and y coordinate
    x = stod(loc.substr(0, xy_sep));
    y = stod(loc.substr(xy_sep+1));
  } else { // 2 separators: x, y, z coordinate
    x = stod(loc.substr(0, xy_sep));
    y = stod(loc.substr(xy_sep+1, yz_sep));
    z = stod(loc.substr(yz_sep+1));
  }
//  my_location.setX(x);
//  my_location.setY(y);
//  my_location.setZ(z);
}

Configuration *Configuration::instance = new Configuration();

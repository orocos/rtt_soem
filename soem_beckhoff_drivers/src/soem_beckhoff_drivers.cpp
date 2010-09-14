#include <rtt/plugin/Plugin.hpp>

extern "C" {
bool loadRTTPlugin(RTT::TaskContext* c){
  return true;
}
}

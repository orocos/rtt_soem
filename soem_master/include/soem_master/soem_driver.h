#ifndef SOEM_DRIVER_H
#define SOEM_DRIVER_H

extern "C"{
#include <ethercattype.h>
#include <ethercatmain.h>
}


#include <rtt/Service.hpp>
#include <rtt/TaskContext.hpp>

#include <sstream>

template <class T>
inline std::string to_string (const T& t,std::ios_base & (*f)(std::ios_base&)){
  std::stringstream ss;
  ss << f << t;
  return ss.str();
};

namespace soem_master{

  
  class SoemDriver
  {
  public:
    ~SoemDriver(){
      service_->clear();
    };
    
    const std::string& getName() const{
      return name_;
    }
    
    RTT::Service::shared_ptr provides(){
      return RTT::Service::shared_ptr(service_);
    };
 
    virtual void addPortsToTaskContext(RTT::TaskContext* tc)=0;
    
    virtual void updatePorts()=0;
   
  protected:
  SoemDriver(ec_slavet* mem_loc) :
    datap_(mem_loc),
      name_("Slave_"+to_string(datap_->configadr,std::hex)),
      service_(new RTT::Service(name_))
	{
	};
    ec_slavet* datap_;
    std::string name_;
    RTT::Service::shared_ptr service_;
    
  };
}
#endif

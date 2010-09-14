#include "soem_el4004.h"
#include <soem_master/soem_driver_factory.h>

namespace soem_beckhoff_drivers{
  
  SoemEL4004::SoemEL4004(ec_slavet* mem_loc):
    soem_master::SoemDriver(mem_loc),
    size_(4),
    raw_range_(32768),
    lowest_(0.0),
    highest_(10.0),
    values_(size_),
    raw_values_(size_),
    values_port_(this->getName()+"_values"),
    raw_values_port_(this->getName()+"_raw_values")
  {
    
    service_->doc(std::string("Services for Beckhoff ")+std::string(datap_->name)+std::string(" module"));
    service_->addOperation("rawWrite",&SoemEL4004::rawWrite,this).doc("Write raw value to channel i").arg("i","channel nr").arg("value","raw value");
    service_->addOperation("rawRead",&SoemEL4004::rawRead,this).doc("Read raw value of channel i").arg("i","channel nr");
    service_->addOperation("write",&SoemEL4004::write,this).doc("Write value to channel i").arg("i","channel nr").arg("value","value");
    service_->addOperation("read",&SoemEL4004::read,this).doc("Read value to channel i").arg("i","channel nr");

    resolution_=((highest_-lowest_)/(double)raw_range_);

    service_->addConstant("raw_range",raw_range_);
    service_->addConstant("resolution",resolution_);
    service_->addConstant("lowest",lowest_);
    service_->addConstant("highest",highest_);
    

    msg_.values.resize(size_);
    raw_msg_.values.resize(size_);

  }

  void SoemEL4004::addPortsToTaskContext(RTT::TaskContext* tc){
    tc->ports()->addPort(values_port_).doc("AnalogMsg containing the desired values of _all_ channels");
    tc->ports()->addPort(raw_values_port_).doc("AnalogMsg containing the desired values of _all_ channels");
  }

  void SoemEL4004::updatePorts(){
    if(raw_values_port_.connected()){
      if(raw_values_port_.read(raw_msg_)==RTT::NewData)
	if(raw_msg_.values.size()<size_)
	  for(unsigned int i=0;i<size_;i++)
	    ((out_el4004t*)(datap_->outputs))->values[i]=raw_msg_.values[i];
    }
    if(values_port_.connected()){
      if(values_port_.read(msg_)==RTT::NewData)
	if(msg_.values.size()<size_)
	  for(unsigned int i=0;i<size_;i++)
	    ((out_el4004t*)(datap_->outputs))->values[i]=msg_.values[i]/resolution_;
    }
  }

  bool SoemEL4004::rawWrite( unsigned int chan, int value ){
    if(chan<size_){
      ((out_el4004t*)(datap_->outputs))->values[chan]=value;
      return true;
    }
    return false;
  }
  int SoemEL4004::rawRead( unsigned int chan ){
    if(chan<size_)
      return ((out_el4004t*)(datap_->outputs))->values[chan];
      return -1;
  }
  
  bool SoemEL4004::write( unsigned int chan, double value ){
    if(chan<size_){
      ((out_el4004t*)(datap_->outputs))->values[chan]=(int)(value/resolution_);
      return true;
    }
    return false;
  }
  
  double SoemEL4004::read( unsigned int chan){
    if(chan<size_)
      return ((out_el4004t*)(datap_->outputs))->values[chan]*resolution_;
    return -1;
  }
  
  namespace {
    soem_master::SoemDriver* createSoemEL4004(ec_slavet* mem_loc){
      return new SoemEL4004(mem_loc);
    }
    const bool registered0 = soem_master::SoemDriverFactory::Instance().registerDriver("EL4004",createSoemEL4004);
    
  }
    
    
}//namespace


    

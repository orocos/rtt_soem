#include "soem_el4004.h"
#include <soem_master/soem_driver_factory.h>

namespace soem_beckhoff_drivers{
  
  SoemEL4004::SoemEL4004(ec_slavet* mem_loc):
    soem_master::SoemDriver(mem_loc),
    m_size(4),
    m_raw_range(32768),
    m_lowest(0.0),
    m_highest(10.0),
    m_values(m_size),
    m_raw_values(m_size),
    m_values_port("values"),
    m_raw_values_port("raw_values")
  {
    
    m_service->doc(std::string("Services for Beckhoff ")+std::string(m_datap->name)+std::string(" module"));
    m_service->addOperation("rawWrite",&SoemEL4004::rawWrite,this,RTT::OwnThread).doc("Write raw value to channel i").arg("i","channel nr").arg("value","raw value");
    m_service->addOperation("rawRead",&SoemEL4004::rawRead,this,RTT::OwnThread).doc("Read raw value of channel i").arg("i","channel nr");
    m_service->addOperation("write",&SoemEL4004::write,this,RTT::OwnThread).doc("Write value to channel i").arg("i","channel nr").arg("value","value");
    m_service->addOperation("read",&SoemEL4004::read,this,RTT::OwnThread).doc("Read value to channel i").arg("i","channel nr");

    m_resolution=((m_highest-m_lowest)/(double)m_raw_range);

    m_service->addConstant("raw_range",m_raw_range);
    m_service->addConstant("resolution",m_resolution);
    m_service->addConstant("lowest",m_lowest);
    m_service->addConstant("highest",m_highest);
    

    m_msg.values.resize(m_size);
    m_raw_msg.values.resize(m_size);

    m_service->addPort(m_values_port).doc("AnalogMsg containing the desired values of _all_ channels");
    m_service->addPort(m_raw_values_port).doc("AnalogMsg containing the desired values of _all_ channels");
  }

  void SoemEL4004::update(){
    if(m_raw_values_port.connected()){
      if(m_raw_values_port.read(m_raw_msg)==RTT::NewData)
    	  if(m_raw_msg.values.size()<m_size)
    		  for(unsigned int i=0;i<m_size;i++)
    			  ((out_el4004t*)(m_datap->outputs))->values[i]=m_raw_msg.values[i];
    }
    if(m_values_port.connected()){
    	if(m_values_port.read(m_msg)==RTT::NewData)
    		if(m_msg.values.size()<m_size)
    			for(unsigned int i=0;i<m_size;i++)
    				((out_el4004t*)(m_datap->outputs))->values[i]=m_msg.values[i]/m_resolution;
    }
  }

  bool SoemEL4004::rawWrite( unsigned int chan, int value ){
	  if(chan<m_size){
		  ((out_el4004t*)(m_datap->outputs))->values[chan]=value;
		  return true;
	  }
	  return false;
  }
  int SoemEL4004::rawRead( unsigned int chan ){
	  if(chan<m_size)
		  return ((out_el4004t*)(m_datap->outputs))->values[chan];
	  return -1;
  }

  bool SoemEL4004::write( unsigned int chan, double value ){
	  if(chan<m_size){
		  ((out_el4004t*)(m_datap->outputs))->values[chan]=(int)(value/m_resolution);
		  return true;
	  }
	  return false;
  }

  double SoemEL4004::read( unsigned int chan){
	  if(chan<m_size)
		  return ((out_el4004t*)(m_datap->outputs))->values[chan]*m_resolution;
	  return -1;
  }

  namespace {
  soem_master::SoemDriver* createSoemEL4004(ec_slavet* mem_loc){
	  return new SoemEL4004(mem_loc);
  }
  const bool registered0 = soem_master::SoemDriverFactory::Instance().registerDriver("EL4004",createSoemEL4004);

  }


}//namespace




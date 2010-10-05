#include "soem_el4032.h"
#include <soem_master/soem_driver_factory.h>

namespace soem_beckhoff_drivers{
  

  SoemEL4032::SoemEL4032(ec_slavet* mem_loc):
    soem_master::SoemDriver(mem_loc),
    m_size(2),
    m_raw_range(65536), //32768
    m_lowest(-10.0),
    m_highest(10.0),
    m_values(m_size),
    m_raw_values(m_size),
    m_values_port("values"),
    m_raw_values_port("raw_values")
  {
    
    m_service->doc(std::string("Services for Beckhoff ")+std::string(m_datap->name)+std::string(" module"));
    m_service->addOperation("rawWrite",&SoemEL4032::rawWrite,this,RTT::OwnThread).doc("Write raw value to channel i").arg("i","channel nr").arg("value","raw value");
    m_service->addOperation("rawRead",&SoemEL4032::rawRead,this,RTT::OwnThread).doc("Read raw value of channel i").arg("i","channel nr");
    m_service->addOperation("write",&SoemEL4032::write,this,RTT::OwnThread).doc("Write value to channel i").arg("i","channel nr").arg("value","value");
    m_service->addOperation("read",&SoemEL4032::read,this,RTT::OwnThread).doc("Read value to channel i").arg("i","channel nr");

    m_resolution=((m_highest-m_lowest)/(double)m_raw_range);

    m_service->addConstant("raw_range",m_raw_range);
    m_service->addConstant("resolution",m_resolution);
    m_service->addConstant("lowest",m_lowest);
    m_service->addConstant("highest",m_highest);
    

    m_msg.values.resize(m_size);
    m_raw_msg.values.resize(m_size);

    for(unsigned int i=0;i<m_size;i++)
    {
        ((out_el4032t*)(m_datap->outputs))->values[i]=0;
    }

    m_service->addPort(m_values_port).doc("AnalogMsg containing the desired values of _all_ channels");
    m_service->addPort(m_raw_values_port).doc("AnalogMsg containing the desired values of _all_ channels");


  }

  void SoemEL4032::update(){

    if(m_raw_values_port.connected()){
      if(m_raw_values_port.read(m_raw_msg)==RTT::NewData)
      {
          if(m_raw_msg.values.size()<=m_size)
          {
              for(unsigned int i=0;i<m_size;i++)
              {
                  ((out_el4032t*)(m_datap->outputs))->values[i]=m_raw_msg.values[i];
              }
          }
      }
    }
    if(m_values_port.connected()){
      if(m_values_port.read(m_msg)==RTT::NewData)
      {
          if(m_msg.values.size()<=m_size)
          {
              for(unsigned int i=0;i<m_size;i++)
              {
                  ((out_el4032t*)(m_datap->outputs))->values[i]=m_msg.values[i]/m_resolution;
              }
          }
      }
    }
  }


  bool SoemEL4032::rawWrite( unsigned int chan, unsigned int value ){
    if(chan<m_size){
		//Prevent overflow
    	if(value < m_raw_range){
    		((out_el4032t*)(m_datap->outputs))->values[chan]=value;
    		return true;
    	}
    }
    return false;
  }

  int SoemEL4032::rawRead( unsigned int chan ){
    if(chan<m_size)
      return ((out_el4032t*)(m_datap->outputs))->values[chan];
      return -1;
  }
  
  bool SoemEL4032::write( unsigned int chan, double value ){
    if(chan<m_size){
    	if(value < m_highest && value >= m_lowest){
    		((out_el4032t*)(m_datap->outputs))->values[chan]=(int)(value/m_resolution);
    		return true;
    	}
    }
    return false;
  }
  
  double SoemEL4032::read( unsigned int chan){
    if(chan<m_size)
      return ((out_el4032t*)(m_datap->outputs))->values[chan]*m_resolution;
    return -1;
  }
  
  namespace {
    soem_master::SoemDriver* createSoemEL4032(ec_slavet* mem_loc){
      return new SoemEL4032(mem_loc);
    }

    const bool registered0 = soem_master::SoemDriverFactory::Instance().registerDriver("EL4032",createSoemEL4032);
   
  }
    
    
}//namespace


    

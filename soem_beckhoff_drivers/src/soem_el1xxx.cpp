/*!
* @author: Ruben Smits and Koen Buys
* @description: Driver for Beckhoff EL1124 module, 4x5V digital inputs
* @version: 9 April 2010 - 1.0
* @license: LGPL (see Berlios - SOEM)
*/
#include "soem_el1xxx.h"
#include <soem_master/soem_driver_factory.h>

namespace soem_beckhoff_drivers{
  
  SoemEL1xxx::SoemEL1xxx(ec_slavet* mem_loc): 
    soem_master::SoemDriver(mem_loc),
    m_port("bits")
  {
    m_size=mem_loc->Ibits;
    m_service->doc(std::string("Services for Beckhoff ")+std::string(m_datap->name)+std::string(" Dig. Input module"));
    m_service->addOperation("isOn",&SoemEL1xxx::isOn,this,RTT::OwnThread).doc("Check if bit i is on").arg("i","bit nr");
    m_service->addOperation("isOff",&SoemEL1xxx::isOff,this,RTT::OwnThread).doc("Check if bit i is off").arg("i","bit nr");
    m_service->addOperation("readBit",&SoemEL1xxx::readBit,this,RTT::OwnThread).doc("Read value of bit i").arg("i","bit nr");
    m_service->addConstant("size",m_size);
    m_msg.values.resize(m_size);
    m_port.setDataSample(m_msg);
    m_service->addPort(m_port).doc("Data port to communicate full bitsets");
  }

  bool SoemEL1xxx::isOn( unsigned int bit) const{
    return readBit(bit);
  }
  
  bool SoemEL1xxx::isOff( unsigned int bit) const{
    return !readBit(bit);
  }
  
  bool SoemEL1xxx::readBit( unsigned int bit) const{
	  if(bit<m_size)
		  return ((((out_el1xxxt*)(m_datap->inputs))->outbits)&(1<<(m_datap->Istartbit+bit)));

	  else{
		  //TODO: We should raise somekind of error here
		  return false;
	  }
  }
  
  void SoemEL1xxx::update(){
    for(unsigned int i=0;i<m_size;i++)
      m_msg.values[i]=readBit(i);
    m_port.write(m_msg);
  }

  /*    
  unsigned int SoemEL1xxx::readSequence(unsigned int start_bit, unsigned int stop_bit) const{
    if(start_bit<size_&&stop_bit<size_){
      m_bits=((out_el1xxxt*)(datap_->inputs))->outbits;
      std::bitset<8> out_bits;
      unsigned int j=0;
      for(unsigned int i=start_bit;i<=stop_bit;i++)
	out_bits.set(j,m_bits[datap_->Istartbit+i]);
      return out_bits.to_ulong();
    }
  }
  */

  namespace {
    soem_master::SoemDriver* createSoemEL1xxx(ec_slavet* mem_loc){
      return new SoemEL1xxx(mem_loc);
    }
    const bool registered1 = soem_master::SoemDriverFactory::Instance().registerDriver("EL1124",createSoemEL1xxx);
    const bool registered2 = soem_master::SoemDriverFactory::Instance().registerDriver("EL1144",createSoemEL1xxx);
    const bool registered3 = soem_master::SoemDriverFactory::Instance().registerDriver("EL1008",createSoemEL1xxx);
  }
}

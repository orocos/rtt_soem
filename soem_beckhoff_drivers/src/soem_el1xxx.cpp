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
    port_(this->getName()+"_bits")
  {
    size_=mem_loc->Ibits;
    service_->doc(std::string("Services for Beckhoff ")+std::string(datap_->name)+std::string(" Dig. Input module"));
    service_->addOperation("isOn",&SoemEL1xxx::isOn,this).doc("Check if bit i is on").arg("i","bit nr");
    service_->addOperation("isOff",&SoemEL1xxx::isOff,this).doc("Check if bit i is off").arg("i","bit nr");
    service_->addOperation("readBit",&SoemEL1xxx::readBit,this).doc("Read value of bit i").arg("i","bit nr");
    service_->addConstant("size",size_);
    msg_.values.resize(size_);
    //port_.doc("values of the bits");
    port_.setDataSample(msg_);
    
    //    port_.createPortObject("bits");
  }

  void SoemEL1xxx::addPortsToTaskContext(RTT::TaskContext* tc){
    tc->ports()->addPort(port_);
  }
  
  bool SoemEL1xxx::isOn( unsigned int bit) const{
    return readBit(bit);
  }
  
  bool SoemEL1xxx::isOff( unsigned int bit) const{
    return !readBit(bit);
  }
  
  bool SoemEL1xxx::readBit( unsigned int bit) const{
    if(bit<size_){
      bits_= ((out_el1xxxt*)(datap_->inputs))->outbits;
      return bits_[bit+datap_->Istartbit];
    }
    else{
        //TODO: We should raise somekind of error here
        return false;
    }
  }
  
  void SoemEL1xxx::updatePorts(){
    bits_= ((out_el1xxxt*)(datap_->inputs))->outbits;
    for(unsigned int i=0;i<size_;i++)
      msg_.values[i]=bits_[i];
    port_.write(msg_);
  }

  /*    
  unsigned int SoemEL1xxx::readSequence(unsigned int start_bit, unsigned int stop_bit) const{
    if(start_bit<size_&&stop_bit<size_){
      bits_=((out_el1xxxt*)(datap_->inputs))->outbits;
      std::bitset<8> out_bits;
      unsigned int j=0;
      for(unsigned int i=start_bit;i<=stop_bit;i++)
	out_bits.set(j,bits_[datap_->Istartbit+i]);
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

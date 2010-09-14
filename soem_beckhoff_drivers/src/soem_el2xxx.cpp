#include "soem_el2xxx.h"
#include <soem_master/soem_driver_factory.h>

namespace soem_beckhoff_drivers{
  
  SoemEL2xxx::SoemEL2xxx(ec_slavet* mem_loc): 
    soem_master::SoemDriver(mem_loc),
    port_(this->getName()+"_bits")
  {
    size_=mem_loc->Obits;
    service_->doc(std::string("Services for Beckhoff ")+std::string(datap_->name)+std::string(" Dig. Output module"));
    
    service_->addOperation("switchOn",&SoemEL2xxx::switchOn,this).doc("Switch bit i on").arg("i","bit nr");
    service_->addOperation("switchOff",&SoemEL2xxx::switchOff,this).doc("Switch bit i off").arg("i","bit nr");
    service_->addOperation("setBit",&SoemEL2xxx::setBit,this).doc("Set value of bit i to val").arg("i","bit nr").arg("val","new value for bit");
    service_->addOperation("checkBit",&SoemEL2xxx::checkBit,this).doc("Check value of bit i").arg("i","bit nr");
    service_->addConstant("size",size_);

    msg_.values.resize(size_);

    for(unsigned int i=0;i<size_;i++)
            bits_.set(i+datap_->Ostartbit,0);
          ((out_el2xxxt*)(datap_->outputs))->outbits=bits_.to_ulong();
  }

  void SoemEL2xxx::addPortsToTaskContext(RTT::TaskContext* tc){
    tc->ports()->addPort(port_).doc("DigitalMsg containting the desired values of _all_ bits");
  }

  void SoemEL2xxx::updatePorts(){
    if(port_.connected()){
      if(port_.read(msg_)==RTT::NewData){
	if(msg_.values.size()<size_){
	  bits_=((out_el2xxxt*)(datap_->outputs))->outbits;
	  for(unsigned int i=0;i<size_;i++)
	    bits_.set(i+datap_->Ostartbit,msg_.values[i]);
	  ((out_el2xxxt*)(datap_->outputs))->outbits=bits_.to_ulong();
	}
      }
    }
  }
  
  void SoemEL2xxx::setBit(unsigned int bit,bool value){
    if(bit<size_){
      bits_=((out_el2xxxt*)(datap_->outputs))->outbits;
      bits_.set(bit+datap_->Ostartbit,value);
      ((out_el2xxxt*)(datap_->outputs))->outbits=bits_.to_ulong();
    }
  }
  
  void SoemEL2xxx::switchOn(unsigned int n){
    this->setBit(n,true);
  }
  
  void SoemEL2xxx::switchOff(unsigned int n){
    this->setBit(n,false);
  }
  /*
    void SoemEL2xxx::setSequence(unsigned int start_bit,unsigned int stop_bit,unsigned int value){
    if(start_bit<size_&&stop_bit<size_){
    bits_=((out_el2xxxt*)(datap_->outputs))->outbits;
    std::bitset<8> in_bits(value);
    for(unsigned int i=start_bit;i<=stop_bit;i++)
    bits_.set(i+datap_->Ostartbit,in_bits[i]);
    ((out_el2xxxt*)(datap_->outputs))->outbits=bits_.to_ulong();
    }
    }
  */
  
  bool SoemEL2xxx::checkBit(unsigned int bit)const{
    if(bit<size_){
      bits_=((out_el2xxxt*)(datap_->outputs))->outbits;
      return bits_.test(bit+datap_->Ostartbit);
    }
    return false;
  }
  /*
  unsigned int SoemEL2xxx::checkSequence(unsigned int start_bit,unsigned int stop_bit)const{
    if(start_bit<size_&&stop_bit<size_){
      bits_=((out_el2xxxt*)(datap_->outputs))->outbits;
      std::bitset<8> out_bits;
      unsigned int j=0;
      for(unsigned int i=start_bit;i<=stop_bit;i++)
	out_bits.set(j,bits_[datap_->Ostartbit+i]);
      return out_bits.to_ulong();
    }
  }
  */
  
  namespace {
    soem_master::SoemDriver* createSoemEL2xxx(ec_slavet* mem_loc){
      return new SoemEL2xxx(mem_loc);
    }
    const bool registered0 = soem_master::SoemDriverFactory::Instance().registerDriver("EL2002",createSoemEL2xxx);
    const bool registered1 = soem_master::SoemDriverFactory::Instance().registerDriver("EL2004",createSoemEL2xxx);
    const bool registered2 = soem_master::SoemDriverFactory::Instance().registerDriver("EL2008",createSoemEL2xxx);
    const bool registered3 = soem_master::SoemDriverFactory::Instance().registerDriver("EL2124",createSoemEL2xxx);
  }
}

#ifndef SOEM_EL1xxx_H
#define SOEM_EL1xxx_H

#include <soem_master/soem_driver.h>
#include <soem_beckhoff_drivers/DigitalMsg.h>
#include <rtt/Port.hpp>
#include <bitset>

namespace soem_beckhoff_drivers{

  class SoemEL1xxx : public soem_master::SoemDriver
  {
    
    typedef struct PACKED
    {
      uint8 outbits;
    } out_el1xxxt;
    
  public:
    SoemEL1xxx(ec_slavet* mem_loc);
    ~SoemEL1xxx(){};
   
    bool isOn( unsigned int bit = 0) const;
    bool isOff( unsigned int bit = 0) const;
    bool readBit( unsigned int bit = 0) const;
    
    void addPortsToTaskContext(RTT::TaskContext* tc);
    void updatePorts();


  private:
    unsigned int size_;
    DigitalMsg msg_;
    mutable std::bitset<8> bits_;
    RTT::OutputPort<DigitalMsg> port_;
};
 
}
#endif

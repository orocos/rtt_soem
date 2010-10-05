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
    
    void update();
    bool configure(){return true;}


  private:
    unsigned int m_size;
    DigitalMsg m_msg;
    mutable std::bitset<8> m_bits;
    RTT::OutputPort<DigitalMsg> m_port;
};
 
}
#endif

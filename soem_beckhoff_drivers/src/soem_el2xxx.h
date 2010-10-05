#ifndef SOEM_EL2XXX_H
#define SOEM_EL2XXX_H

#include <soem_master/soem_driver.h>
#include <soem_beckhoff_drivers/DigitalMsg.h>
#include <rtt/Port.hpp>
#include <bitset>

namespace soem_beckhoff_drivers{

  class SoemEL2xxx : public soem_master::SoemDriver
  {
    
    typedef struct PACKED
    {
      uint8 outbits;
    } out_el2xxxt;
    
  public:
    SoemEL2xxx(ec_slavet* mem_loc);
    ~SoemEL2xxx(){};

    void switchOn( unsigned int n );
    void switchOff( unsigned int n );
    void setBit( unsigned int bit, bool value );
    bool checkBit(unsigned int n) const;

    void update();
    
  private:
    unsigned int m_size;
    DigitalMsg m_msg;
    RTT::InputPort<DigitalMsg> m_port;
  };
 
}
#endif

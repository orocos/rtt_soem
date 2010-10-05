#ifndef SOEM_EL4004_H
#define SOEM_EL4004_H

#include <soem_master/soem_driver.h>
#include <soem_beckhoff_drivers/AnalogMsg.h>
#include <rtt/Port.hpp>
#include <bitset>

namespace soem_beckhoff_drivers{

  class SoemEL4004 : public soem_master::SoemDriver
  {
    
    typedef struct PACKED
    {
      uint16 values[4];
    } out_el4004t;
    
  public:
    SoemEL4004(ec_slavet* mem_loc);
    ~SoemEL4004(){};

    bool rawWrite( unsigned int chan, int value );
    int rawRead( unsigned int chan );
    
    bool write( unsigned int chan, double value );
    double read( unsigned int chan);
    

    void update();
    
  private:
    const unsigned int m_size;
    const unsigned int m_raw_range;
    const double m_lowest;
    const double m_highest;
    double m_resolution;

    AnalogMsg m_msg;
    AnalogMsg m_raw_msg;
    std::vector<double> m_values;
    std::vector<double> m_raw_values;
    RTT::InputPort<AnalogMsg> m_values_port;
    RTT::InputPort<AnalogMsg> m_raw_values_port;
  };
 
}
#endif

#ifndef SOEM_EL4032_H
#define SOEM_EL4032_H

#include <soem_master/soem_driver.h>
#include <soem_beckhoff_drivers/AnalogMsg.h>
#include <rtt/Port.hpp>
#include <bitset>

namespace soem_beckhoff_drivers{

  class SoemEL4032 : public soem_master::SoemDriver
  {
    
    typedef struct PACKED
    {
      uint16 values[2];
    } out_el4032t;
    
  public:
    SoemEL4032(ec_slavet* mem_loc);
    ~SoemEL4032(){};

    bool rawWrite( unsigned int chan, unsigned int value );
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

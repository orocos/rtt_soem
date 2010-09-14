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
    

    void addPortsToTaskContext(RTT::TaskContext* tc);
    void updatePorts();
    
  private:
    const unsigned int size_;
    const unsigned int raw_range_;
    const double lowest_;
    const double highest_;
    double resolution_;

    AnalogMsg msg_;
    AnalogMsg raw_msg_;
    std::vector<double> values_; 
    std::vector<double> raw_values_; 
    RTT::InputPort<AnalogMsg> values_port_;
    RTT::InputPort<AnalogMsg> raw_values_port_;
  };
 
}
#endif

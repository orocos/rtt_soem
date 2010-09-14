#ifndef SOEM_EL3062_H
#define SOEM_EL3062_H

#include <soem_master/soem_driver.h>
#include <soem_beckhoff_drivers/AnalogMsg.h>
#include <rtt/Port.hpp>
#include <bitset>
#include <rtt/os/main.h>
#include <rtt/Property.hpp>
#include "COE_config.h"


namespace soem_beckhoff_drivers{

  class SoemEL3062 : public soem_master::SoemDriver
  {
    
    typedef struct PACKED
    {
      int16 param_ch1;
      int16 val_ch1;
       
      int16 param_ch2;
      int16 val_ch2;
    } out_el3062t;
	


  public:
    SoemEL3062(ec_slavet* mem_loc);
    ~SoemEL3062(){};
    
    int rawRead( unsigned int chan );
    double read( unsigned int chan);
    bool isOverrange( unsigned int chan = 0) ;
    bool isUnderrange( unsigned int chan = 0);
    int CompareV_to_Lim( unsigned int chan=0 , unsigned int Lim_num=0);
    int read_param( unsigned int chan );
    bool is_error( unsigned int chan );
    bool Tx_PDO( unsigned int chan); 
    bool Tx_PDO_Toggle( unsigned int chan);
    
    void addPortsToTaskContext(RTT::TaskContext* tc);
    void updatePorts();
    
  private:
   

    // Property //////
    RTT::Property<int> temp_val;
  
    const unsigned int size_;
    const unsigned int raw_range_;
    const double lowest_;
    const double highest_;
    double resolution_;
    mutable std::bitset<16> ch_par;
    AnalogMsg msg_;
    AnalogMsg raw_msg_;
    std::vector<double> values_; 
    std::vector<double> raw_values_; 
  
   //Ports///////////
    RTT::OutputPort<AnalogMsg> values_port_;
  
    RTT::OutputPort<AnalogMsg> raw_values_port_;

    std::vector<parameter> params; 
  };
 
}
#endif

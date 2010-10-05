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
    
    void update();
    bool configure();
    
  private:
   

    RTT::Property<parameter> temp_val;
  
    const unsigned int m_size;
    const unsigned int m_raw_range;
    const double m_lowest;
    const double m_highest;
    double m_resolution;
    mutable std::bitset<16> ch_par;
    AnalogMsg m_msg;
    AnalogMsg m_raw_msg;
    std::vector<double> m_values;
    std::vector<double> m_raw_values;
  
   //Ports///////////
    RTT::OutputPort<AnalogMsg> m_values_port;
  
    RTT::OutputPort<AnalogMsg> m_raw_values_port;

    std::vector<parameter> params; 
  };
 
}
#endif

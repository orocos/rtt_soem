#ifndef SOEM_EL5101_H
#define SOEM_EL5101_H

#include <soem_master/soem_driver.h>
#include <soem_beckhoff_drivers/EncoderMsg.h>
#include <rtt/Port.hpp>
#include <rtt/Property.hpp>
#include <bitset>
#include <vector>


namespace soem_beckhoff_drivers{

  class SoemEL5101 : public soem_master::SoemDriver
  {
    
	  typedef struct PACKED
	  {
	  	uint8	   control;
	  	uint16	   outvalue;
	  } out_el5101t;

	  typedef struct PACKED
	  {
	  	uint8	   status;
	  	uint16	   invalue;
	  	uint16	   latch;
        uint32     frequency;
        uint16     period;
        uint16     window;
	  } in_el5101t;

	  typedef struct{
	      uint16   index;
	      uint8    subindex;
	      uint8    size;
	      int      param;
	      string   name;
	      string   description;
	  } parameter;
    
  public:
    SoemEL5101(ec_slavet* mem_loc);
    ~SoemEL5101(){};
    
    //bool write( unsigned int chan, double value );
    double read(void);
    /*double read_out(void);
    int write_out(uint);
    unsigned int control(void);
    unsigned int status(void);*/
    
    void addPortsToTaskContext(RTT::TaskContext* tc);
    void updatePorts();
    
  private:

    EncoderMsg msg_;
    std::vector<double> values_in_;
    RTT::OutputPort<EncoderMsg > values_port_;
    RTT::Property<std::string> propriete;
    std::vector<parameter> params;
  };
 
}
#endif

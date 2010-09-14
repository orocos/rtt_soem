#include "soem_el3062.h"
#include <soem_master/soem_driver_factory.h>
#include <rtt/Property.hpp>
#include <iostream>



namespace soem_beckhoff_drivers{
  
  SoemEL3062::SoemEL3062(ec_slavet* mem_loc):
    soem_master::SoemDriver(mem_loc),
    size_(2), 
    raw_range_(32768),
    lowest_(0.0),
    highest_(10.0),
    values_(size_),
    raw_values_(size_),
    values_port_(this->getName()+"_values"),
    raw_values_port_(this->getName()+"_raw_values")
    
  {
    service_->doc(std::string("Services for Beckhoff ")+std::string(datap_->name)+std::string(" module"));
    service_->addOperation("rawRead",&SoemEL3062::rawRead,this).doc("Read raw value of channel i").arg("i","channel nr");
    service_->addOperation("read",&SoemEL3062::read,this).doc("Read value to channel i").arg("i","channel nr");
    service_->addOperation("Over_Range",&SoemEL3062::isOverrange,this).doc("For the chanel i : 1 = overrange ; 0 = no overrange ").arg("i","channel nr");
    service_->addOperation("Under_Range",&SoemEL3062::isUnderrange,this).doc("For the chanel i : 1 = Underrange ; 0 = no Underrange ").arg("i","channel nr");
    service_->addOperation("Comp_val_to_lim",&SoemEL3062::CompareV_to_Lim,this).doc("Limit 1/2 value monitoring of channel i :  0= not active, 1= Value is higher than    limit 1/2 value, 2= Value is lower than limit 1/2 value, 3: Value equals limit 1/2 value").arg("i","channel nr").arg("x","Limit nr");
    service_->addOperation("Error",&SoemEL3062::is_error,this).doc("For the chanel i : 1 = error (Overrange or Underrange ; 0 = no error ").arg("i","channel nr");
    service_->addOperation("TxPDO",&SoemEL3062::Tx_PDO,this).doc("For the chanel i :Valid of data of assigned TxPDO (0=valid, 1=invalid) ").arg("i","channel nr");
    service_->addOperation("Toggle",&SoemEL3062::Tx_PDO_Toggle,this).doc("The TxPDO Toggle is toggled by the slave, if the data of the corresponding PDO was updated").arg("i","channel nr");
    
    resolution_=((highest_-lowest_)/(double)raw_range_);

    service_->addConstant("raw_range",raw_range_);
    service_->addConstant("resolution",resolution_);
    service_->addConstant("lowest",lowest_);
    service_->addConstant("highest",highest_);
    
    
    msg_.values.resize(size_);
    raw_msg_.values.resize(size_);
    
  }
  
  
  void SoemEL3062::addPortsToTaskContext(RTT::TaskContext* tc){
    
    tc->ports()->addPort(values_port_).doc("AnalogMsg contain the read values of _all_ channels");
    tc->ports()->addPort(raw_values_port_).doc("AnalogMsg containing the read values of _all_ channels");

    // New properties : Component Configuration :  Can be completed
    // Need to be complete for a specific usage : See Beckof : http://www.beckhoff.com/EL3062/  
    //see  documentation (.xchm) / Commissionning / Object description and parameterization

    parameter temp;

    /// For parametrisation : uncomment the following code and complete it with using the document above
    /*   temp.description="Enable user scale";
	 temp.index=0x8010;
	 temp.subindex=0x01;
	 temp.name="E_user_scl";
	 temp.size=1;
	 temp.param=1;
	 params.push_back(temp);
	 
	 temp.description="Limite1ch1";
	 temp.index=0x8010;
	 temp.subindex=0x13;
	 temp.name="Limite1ch1";
	 temp.size=2;
	 temp.param=9174;
	 params.push_back(temp);
	 
	 temp.description="Limite2ch1";
	 temp.index=0x8010;
	 temp.subindex=0x14;
	 temp.name="Limite2ch1";
	 temp.size=2;
	 temp.param=24247;
	 params.push_back(temp);*/
    
    
    // Adding proprietes to the Soem_master_Component.
    for(unsigned int i=0;i<params.size();i++){  
      tc->addProperty(this->getName()+params[i].name,params[i].param).doc(params[i].description);
    }
    
    
    // Configuration of the slave
    int val;
    
    for(unsigned int i=0;i<params.size();i++){							
      
      while(EcatError){ec_elist2string();}
      
      temp_val=tc->properties()->getProperty(this->getName()+params[i].name);						
      val=temp_val.get();
      
      //assigning parameters
      
      ec_SDOwrite (((datap_->configadr)&0x0F),params[i].index,params[i].subindex,FALSE,params[i].size,&val,EC_TIMEOUTRXM);
    }
    
  }
  
  
  void SoemEL3062::updatePorts(){
    if(raw_values_port_.connected()){
      
      raw_msg_.values[0]=((out_el3062t*)(datap_->inputs))->val_ch1;
      raw_msg_.values[1]=((out_el3062t*)(datap_->inputs))->val_ch2;
    }
    
    if(values_port_.connected()){
      
      msg_.values[0]=(((out_el3062t*)(datap_->inputs))->val_ch1*resolution_);
      msg_.values[1]=(((out_el3062t*)(datap_->inputs))->val_ch2*resolution_);
    }
  }
  
  
  
  //rawRead : read the raw value of the input ///////////////////////////////////////////////////// 
  int SoemEL3062::rawRead( unsigned int chan ){
    if(chan<size_)
      {
	if (chan==0) return ((out_el3062t*)(datap_->inputs))->val_ch1;
	if (chan==1) return ((out_el3062t*)(datap_->inputs))->val_ch2;
      }
    return -1;
  }
  
  int SoemEL3062::read_param( unsigned int chan ){
    if(chan<size_)
      {
	if (chan==0) return ((out_el3062t*)(datap_->inputs))->param_ch1;
	if (chan==1) return ((out_el3062t*)(datap_->inputs))->param_ch2;
      }
    return -1;
  }
  
  
  //read: read the value of one of the 2 channels in Volts ///////////////////////////
  double SoemEL3062::read( unsigned int chan){
    
    if(chan<size_)
      {
	if (chan==0) return (((out_el3062t*)(datap_->inputs))->val_ch1*resolution_);
	if (chan==1) return (((out_el3062t*)(datap_->inputs))->val_ch2*resolution_);
      }
    return -1;	
    
  }
  
  
  //Checking overrange////////////////////////////////////////////////////////////////
  
  bool SoemEL3062::isOverrange( unsigned int chan) 
  {
    ch_par=read_param(chan);
    return ch_par[1];
  }
  
  //Checking Underrange////////////////////////////////////////////////////////////////
  
  bool SoemEL3062::isUnderrange( unsigned int chan) 
  {
    ch_par=read_param(chan);
    return ch_par[0];
  }
  
  // Comparing the Value of Channel chan with its own limits (1/2)
  int SoemEL3062::CompareV_to_Lim( unsigned int chan, unsigned int Lim_num){
    
    if ((Lim_num<3) & (chan<size_)){
      switch(Lim_num){
      case 1 :
	return ((read_param(chan))&&(0x0c))>>2;
	break;
      case 2 :
	return ((read_param(chan))&&(0x30))>>4;
	break;
      default : return-1;
	break;
      }
    }
    return -1;
  }
  
  // Checking for errors 
  bool SoemEL3062:: is_error( unsigned int chan ){
    ch_par=read_param(chan);
    return ch_par[6];
  }
  
  //TxPDO
  bool SoemEL3062:: Tx_PDO( unsigned int chan ){
    ch_par=read_param(chan);
    return ch_par[14];
  }
  
  // TxPDO Toggle
  bool SoemEL3062:: Tx_PDO_Toggle( unsigned int chan ){
    ch_par=read_param(chan);
    return ch_par[15];
  }
  
  
  
  namespace {
    soem_master::SoemDriver* createSoemEL3062(ec_slavet* mem_loc){
      return new SoemEL3062(mem_loc);
    }
    const bool registered0 = soem_master::SoemDriverFactory::Instance().registerDriver("EL3062",createSoemEL3062);
    
  }
  
    
}//namespace


    

// Copyright  (C)  2010  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// Author: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
// Maintainer: Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

extern "C"
{
#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatbase.h"
#include "ethercatmain.h"
#include "ethercatconfig.h"
#include "ethercatdc.h"
#include "ethercatcoe.h"
#include "ethercatprint.h"
}

#include <cstdio>

#include "soem_driver_factory.h"
#include "soem_master_component.h"

#include "rtt/Component.hpp"

ORO_CREATE_COMPONENT( soem_master::SoemMasterComponent )

namespace soem_master
{

using namespace RTT;

SoemMasterComponent::SoemMasterComponent(const std::string& name) :
    TaskContext(name, PreOperational)
{
    this->addProperty("ifname", prop_ifname1="eth0").doc(
            "interface to which the ethercat device is connected");
    this->addProperty("ifname2", prop_ifname2="eth1").doc(
            "Second (redundant) interface to which the ethercat device is connected");
    this->addProperty("redundant", prop_redundant=false).doc(
            "Whether to use a redundant nic");
    this->addProperty("slavesCoeParameters",parameters).doc(
    	    "Vector of parameters to be sent to the slaves using CoE SDO");

    SoemDriverFactory& driver_factory = SoemDriverFactory::Instance();
    this->addOperation("displayAvailableDrivers",
            &SoemDriverFactory::displayAvailableDrivers, &driver_factory).doc(
            "display all available drivers for the soem master");
    this->addOperation("writeCoeSdo", &SoemMasterComponent::writeCoeSdo,this).doc(
	    "send a CoE SDO write (blocking: not to be done while slaves are in OP)");
    this->addOperation("readCoeSdo", &SoemMasterComponent::readCoeSdo,this).doc(
	    "send a CoE SDO read (blocking: not to be done while slaves are in OP)");

    //RTT::types::Types()->addType(new types::EnumTypeInfo<ec_state>("ec_state"));

    RTT::types::GlobalsRepository::shared_ptr globals = RTT::types::GlobalsRepository::Instance();

    globals->setValue( new Constant<ec_state>("EC_STATE_INIT",EC_STATE_INIT) );
    globals->setValue( new Constant<ec_state>("EC_STATE_PRE_OP",EC_STATE_PRE_OP) );
    globals->setValue( new Constant<ec_state>("EC_STATE_SAFE_OP",EC_STATE_SAFE_OP) );
    globals->setValue( new Constant<ec_state>("EC_STATE_OPERATIONAL",EC_STATE_OPERATIONAL) );
    globals->setValue( new Constant<ec_state>("EC_STATE_BOOT",EC_STATE_BOOT) );

 //AssignableDataSource<ec_state>::shared_ptr init ;
 //AssignableDataSource<ec_state>::shared_ptr preop ;
 //AssignableDataSource<ec_state>::shared_ptr boot ;
 //AssignableDataSource<ec_state>::shared_ptr safeop ;
 //AssignableDataSource<ec_state>::shared_ptr op ;

 //       init = new ValueDataSource<ec_state>( EC_STATE_INIT );
 //       preop = new ValueDataSource<ec_state>( EC_STATE_PRE_OP );
 //       boot = new ValueDataSource<ec_state>( EC_STATE_SAFE_OP );
 //       safeop = new ValueDataSource<ec_state>( EC_STATE_OPERATIONAL );
 //       op = new ValueDataSource<ec_state>( EC_STATE_BOOT );

    RTT::types::Types()->addType(new ec_stateTypeInfo());
    RTT::types::Types()->addType(new parameterTypeInfo());
    RTT::types::Types()->addType(new types::SequenceTypeInfo< std::vector<rtt_soem::Parameter> >("std.vector<Parameter>"));

    //this->addOperation("start",&TaskContext::start,this,RTT::OwnThread);
}

SoemMasterComponent::~SoemMasterComponent()
{
}

bool SoemMasterComponent::configureHook()
{
    Logger::In in(this->getName());
    
    int ret;
    if(prop_redundant && !prop_ifname1.empty() && !prop_ifname2.empty())
        ret = ec_init_redundant((char*)prop_ifname1.c_str(),(char*)prop_ifname2.c_str());
    else
        ret = ec_init((char*)prop_ifname1.c_str());
    // initialise SOEM, bind socket to ifname
    if ( ret > 0)
    {
        log(Info) << "ec_init on " << prop_ifname1 << (prop_redundant ? std::string("and ") + prop_ifname2 : "")<<" succeeded." << endlog();

        //Initialise default configuration, using the default config table (see ethercatconfiglist.h)
        if (ec_config_init(FALSE) > 0)
        {
            while (EcatError)
                {
                    log(Error) << ec_elist2string() << endlog();
                }
            
            log(Info) << ec_slavecount << " slaves found and configured."
                    << endlog();
            log(Info) << "Request pre-operational state for all slaves"
                    << endlog();
            ec_slave[0].state = EC_STATE_PRE_OP;
            ec_writestate(0);
            //Wait for all slaves to reach PRE_OP state
            if(!checkNetworkState(EC_STATE_PRE_OP, EC_TIMEOUTSTATE))
              return false;

            //The parameters to be sent to the slaves are loaded from the soem.cpf:
            //parameters could be changed without modifying the code
            for (unsigned int i=0; i < parameters.size(); i++)
            {
               int wkc;
               AddressInfo tmp;
               tmp.slave_position = parameters[i].slave_position;
               tmp.index = parameters[i].index;
               tmp.sub_index = parameters[i].sub_index;

               if(ec_slave[tmp.slave_position].mbx_proto & ECT_MBXPROT_COE)
               {
                 wkc = writeCoeSdo(tmp,parameters[i].complete_access,parameters[i].size,&parameters[i].param);

                 if(wkc == 0)
                 {
                    log(Error) << "Slave_" << ec_slave[tmp.slave_position].configadr <<" SDOwrite{index["<< tmp.index
                               << "] sub_index["<< (int)tmp.sub_index <<"] size "<< parameters[i].size
                               << " value "<< parameters[i].param << "} wkc "<< wkc << endlog();
                 }
               }
               else
               {
                 log(Error) << "Slave_" << ec_slave[tmp.slave_position].configadr <<" does not support CoE"
                       << " but in soem.cpf there is a CoE parameter for this slave." << endlog();
               }

            }

            for (int i = 1; i <= ec_slavecount; i++)
            {
                SoemDriver
                        * driver = SoemDriverFactory::Instance().createDriver(
                                &ec_slave[i]);
                if (driver)
                {
                    m_drivers.push_back(driver);
                    log(Info) << "Created driver for " << ec_slave[i].name
                            << ", with address " << ec_slave[i].configadr
                            << endlog();
                    //Adding driver's services to master component
                    this->provides()->addService(driver->provides());
                    log(Info) << "Put configured parameters in the slaves."
                            << endlog();
                    if (!driver->configure())
                        return false;
                }
                else
                {
                    log(Warning) << "Could not create driver for "
                            << ec_slave[i].name << endlog();
                }
            }

            ec_config_map(&m_IOmap);
            while (EcatError)
                {
                    log(Error) << ec_elist2string() << endlog();
                }

            for (unsigned int i = 0; i < m_drivers.size(); i++)
                if (!m_drivers[i]->start()){
                    log(Error)<<"Could not start driver for "<<m_drivers[i]<<getName()<<endlog();
                    return false;
                }
            

            //Configure distributed clock
            //ec_configdc();
            //Read the state of all slaves

            //ec_readstate();

        }
        else
        {
            log(Error) << "Configuration of slaves failed!!!" << endlog();
            log(Error) << "The NIC currently used for EtherCAT is "<<  prop_ifname1.c_str() << " . Another could be chosen by editing soem.cpf." << endlog();
            return false;
        }
        return true;
    }
    else
    {
        log(Error) << "Could not initialize master on " << prop_ifname1 << (prop_redundant ? std::string("and ") + prop_ifname2 : "") << endlog();
        return false;
    }

}

bool SoemMasterComponent::startHook()
{
            bool state_reached;

            log(Info) << "Request safe-operational state for all slaves" << endlog();
            ec_slave[0].state = EC_STATE_SAFE_OP;
            ec_writestate(0);
            // wait for all slaves to reach SAFE_OP state
            checkNetworkState(EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);

            log(Info) << "Request operational state for all slaves" << endlog();
            ec_slave[0].state = EC_STATE_OPERATIONAL;

			// send one valid process data to make outputs in slaves happy
			ec_send_processdata();

            ec_writestate(0);
            while (EcatError)
                {
                    log(Error) << ec_elist2string() << endlog();
                }

            // wait for all slaves to reach OP state
            if(!checkNetworkState(EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE))
              return false;

            return true;
}

void SoemMasterComponent::updateHook()
{
    bool success = true;
    Logger::In in(this->getName());
    if (ec_receive_processdata(EC_TIMEOUTRET) == 0)
    {
        success = false;
        log(Warning) << "receiving data failed" << endlog();
    }

    if (success)
        for (unsigned int i = 0; i < m_drivers.size(); i++)
            m_drivers[i]->update();

    if (ec_send_processdata() == 0)
    {
        success = false;
        log(Warning) << "sending process data failed" << endlog();
    }
    while (EcatError)
    {
        log(Error) << ec_elist2string() << endlog();
    }


}

void SoemMasterComponent::cleanupHook()
{
  for (unsigned int i = 0; i < m_drivers.size(); i++){
    this->provides()->removeService(m_drivers[i]->provides()->getName());
    delete m_drivers[i];
  }

    //stop SOEM, close socket
    ec_close();
}

int SoemMasterComponent::writeCoeSdo(const AddressInfo& address, bool complete_access, int size, void* data)
{
  return ec_SDOwrite(address.slave_position,address.index,address.sub_index,complete_access,size,data,EC_TIMEOUTRXM);
}

int SoemMasterComponent::readCoeSdo(const AddressInfo& address, bool complete_access, int* size, void* data)
{
  return ec_SDOread(address.slave_position,address.index,address.sub_index,complete_access,size,data,EC_TIMEOUTRXM);
}

bool  SoemMasterComponent::checkNetworkState(ec_state desired_state, int timeout)
{
  bool state_is_reached = true;
  bool error_detected = false;

  uint16 network_state = ec_statecheck(0, desired_state, timeout);

  if((network_state & 0xf0) == 0)
  {
    // No slave has toggled the error flag so the AlStatusCode (even if different from 0) should be ignored
    for(int i = 0; i < ec_slavecount; i++)
    {
      ec_slave[i].ALstatuscode = 0x0000;
    }
  }
  else
  {
    error_detected = true;
  }

  switch(network_state)
  {
    case EC_STATE_INIT:
    case EC_STATE_PRE_OP:
    case EC_STATE_BOOT:
    case EC_STATE_SAFE_OP:
    case EC_STATE_OPERATIONAL:
      if(!error_detected)
      {
        //All the slaves have reached the same state so we can update the state of every slave
        for(int i = 0; i < ec_slavecount; i++)
        {
          ec_slave[i].state = network_state;
        }
      }
      else
      {
        ec_readstate();
      }

      break;

    default:
      //The state should be verified for every single slave
      //since not all have the same state
      ec_readstate();
      break;
  }

  if (ec_slave[0].state == desired_state)
  {
      log(Info) << (ec_state)ec_slave[0].state <<" state reached for all slaves."
              << endlog();
      while (EcatError)
          {
              log(Error) << ec_elist2string() << endlog();
          }
  }
  else
  {
      log(Error) << "Not all slaves reached safe operational state."
              << endlog();

      //If not all slaves reached target state find out which one
      for (int i = 0; i <= ec_slavecount; i++)
      {
          if (ec_slave[i].state != desired_state)
          {
              state_is_reached = false;

              log(Error) << "Slave " << i << " State= " <<
                      (ec_state)ec_slave[i].state << " StatusCode="
                      << ec_slave[i].ALstatuscode << " : "
                      << ec_ALstatuscode2string(
                              ec_slave[i].ALstatuscode) << endlog();
          }
      }
  }

  return state_is_reached;
}

}//namespace

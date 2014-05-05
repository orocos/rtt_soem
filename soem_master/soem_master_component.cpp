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
    SoemDriverFactory& driver_factory = SoemDriverFactory::Instance();
    this->addOperation("displayAvailableDrivers",
            &SoemDriverFactory::displayAvailableDrivers, &driver_factory).doc(
            "display all available drivers for the soem master");
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
            // wait for all slaves to reach PRE_OP state
            ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE);

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
            log(Info) << "Request safe-operational state for all slaves" << endlog();
            ec_slave[0].state = EC_STATE_SAFE_OP;
            ec_writestate(0);
            // wait for all slaves to reach SAFE_OP state
            ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);

            if (ec_slave[0].state == EC_STATE_SAFE_OP)
            {
                log(Info) << "Safe operational state reached for all slaves."
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
                ec_readstate();
                //If not all slaves operational find out which one
                for (int i = 0; i <= ec_slavecount; i++)
                {
                    if (ec_slave[i].state != EC_STATE_SAFE_OP)
                    {
                        log(Error) << "Slave " << i << " State= " << to_string(
                                ec_slave[i].state, std::hex) << " StatusCode="
                                << ec_slave[i].ALstatuscode << " : "
                                << ec_ALstatuscode2string(
                                        ec_slave[i].ALstatuscode) << endlog();
                    }
                }
                //return false;
            }

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
            ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
            if (ec_slave[0].state == EC_STATE_OPERATIONAL)
            {
                log(Info) << "Operational state reached for all slaves."
                        << endlog();
            }
            else
            {
                log(Error) << "Not all slaves reached operational state."
                        << endlog();
                //If not all slaves operational find out which one
                for (int i = 1; i <= ec_slavecount; i++)
                {
                    if (ec_slave[i].state != EC_STATE_OPERATIONAL)
                    {
                        log(Error) << "Slave " << i << " State= " << to_string(
                                ec_slave[i].state, std::hex) << " StatusCode="
                                << ec_slave[i].ALstatuscode << " : "
                                << ec_ALstatuscode2string(
                                        ec_slave[i].ALstatuscode) << endlog();
                    }
                }
                return false;

            }
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
}//namespace

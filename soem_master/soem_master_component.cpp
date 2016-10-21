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

extern "C" {
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

ORO_CREATE_COMPONENT(soem_master::SoemMasterComponent)

namespace soem_master {

using namespace RTT;

SoemMasterComponent::SoemMasterComponent(const std::string& name) :
    TaskContext(name, PreOperational) {
  this->addProperty("ifname", prop_ifname1 = "eth0").doc(
      "interface to which the ethercat device is connected");
  this->addProperty("ifname2", prop_ifname2 = "eth1").doc(
      "Second (redundant) interface to which the ethercat device is connected");
  this->addProperty("redundant", prop_redundant = false).doc(
      "Whether to use a redundant nic");
  this->addProperty("slavesCoeParameters", parameters).doc(
      "Vector of parameters to be sent to the slaves using CoE SDO");

  SoemDriverFactory& driver_factory = SoemDriverFactory::Instance();
  this->addOperation("displayAvailableDrivers",
      &SoemDriverFactory::displayAvailableDrivers, &driver_factory).doc(
      "display all available drivers for the soem master");
  this->addOperation("writeCoeSdo", &SoemMasterComponent::writeCoeSdo, this).doc(
      "send a CoE SDO write "
          "(blocking: not to be done while slaves are in OP)");
  this->addOperation("readCoeSdo", &SoemMasterComponent::readCoeSdo, this).doc(
      "send a CoE SDO read "
          "(blocking: not to be done while slaves are in OP)");

  // To teach RTT the ec_state constants
  RTT::types::GlobalsRepository::shared_ptr globals =
      RTT::types::GlobalsRepository::Instance();
  globals->setValue(new Constant<ec_state>("EC_STATE_INIT", EC_STATE_INIT));
  globals->setValue(new Constant<ec_state>("EC_STATE_PRE_OP", EC_STATE_PRE_OP));
  globals->setValue(
      new Constant<ec_state>("EC_STATE_SAFE_OP", EC_STATE_SAFE_OP));
  globals->setValue(
      new Constant<ec_state>("EC_STATE_OPERATIONAL", EC_STATE_OPERATIONAL));
  globals->setValue(new Constant<ec_state>("EC_STATE_BOOT", EC_STATE_BOOT));

  RTT::types::Types()->addType(new ec_stateTypeInfo());

  // To teach RTT SOEM fixed length types
  RTT::types::Types()->addType(new types::StdTypeInfo<uint8_t>("uint8"));
  RTT::types::Types()->addType(new types::StdTypeInfo<uint16_t>("uint16"));
  RTT::types::Types()->addType(new types::StdTypeInfo<uint32_t>("uint32"));

  // To teach RTT how to manage the CoE parameters
  // that can be load from soem.cpf
  RTT::types::Types()->addType(new parameterTypeInfo());
  RTT::types::Types()->addType(
      new types::SequenceTypeInfo<std::vector<rtt_soem::Parameter> >(
          "std.vector<Parameter>"));

  //this->addOperation("start",&TaskContext::start,this,RTT::OwnThread);
}

SoemMasterComponent::~SoemMasterComponent() {
}

bool SoemMasterComponent::configureHook() {
  Logger::In in(this->getName());

  int ret;
  if (prop_redundant && !prop_ifname1.empty() && !prop_ifname2.empty())
    ret = ec_init_redundant((char*) prop_ifname1.c_str(),
        (char*) prop_ifname2.c_str());
  else
    ret = ec_init((char*) prop_ifname1.c_str());
  // Initialise SOEM, bind socket to ifname
  if (ret > 0) {
    log(Info) << "ec_init on " << prop_ifname1
        << (prop_redundant ? std::string("and ") + prop_ifname2 : "")
        << " succeeded." << endlog();

    // Initialise default configuration, using the default config table
    // (see ethercatconfiglist.h)
    if (ec_config_init(FALSE) > 0) {
      if (EcatError)
        notifySoemErrors();

      log(Info) << ec_slavecount << " slaves found and configured." << endlog();

      setSlavesTargetState(EC_STATE_PRE_OP);
      // Wait for all slaves to reach PRE_OP state
      if (!checkSlavesStateReachedWaiting(EC_STATE_PRE_OP, EC_TIMEOUTSTATE))
        return false;

      // The parameters to be sent to the slaves are loaded from the soem.cpf:
      // parameters could be changed without modifying the code
      for (unsigned int i = 0; i < parameters.size(); i++) {
        bool sdoWriteDone;
        AddressInfo tmpAddressInfo;
        tmpAddressInfo.slave_position = (uint16) parameters[i].slave_position;
        tmpAddressInfo.index = (uint16) parameters[i].index;
        tmpAddressInfo.sub_index = (uint8) parameters[i].sub_index;

        sdoWriteDone = writeCoeSdo(tmpAddressInfo,
            parameters[i].complete_access, parameters[i].size,
            &parameters[i].param);

        if (!sdoWriteDone) {
          log(Error) << "SDO write requested from soem.cpf failed." << endlog();
          return false;
        }
      }

      for (int i = 1; i <= ec_slavecount; i++) {
        SoemDriver * driver = SoemDriverFactory::Instance().createDriver(
            &ec_slave[i]);
        if (driver) {
          m_drivers.push_back(driver);
          log(Info) << "Created driver for " << ec_slave[i].name
              << ", with address " << ec_slave[i].configadr << endlog();
          // Adding driver's services to master component
          this->provides()->addService(driver->provides());
          log(Info) << "Put configured parameters in the slaves." << endlog();
          if (!driver->configure())
            return false;
        } else {
          log(Warning) << "Could not create driver for " << ec_slave[i].name
              << endlog();
        }
      }

      ec_config_map(&m_IOmap);
      if (EcatError)
        notifySoemErrors();

      for (unsigned int i = 0; i < m_drivers.size(); i++)
        if (!m_drivers[i]->start()) {
          log(Error) << "Could not start driver for " << m_drivers[i]
              << getName() << endlog();
          return false;
        }

      // Configure distributed clock
      // ec_configdc();
      // Read the state of all slaves

      // ec_readstate();

    } else {
      log(Error) << "Configuration of slaves failed!!! \n"
          << "The NIC currently used for EtherCAT is " << prop_ifname1.c_str()
          << ". Another could be chosen by editing soem.cpf." << endlog();
      return false;
    }
    return true;
  } else {
    log(Error) << "Could not initialize master on " << prop_ifname1
        << (prop_redundant ? std::string("and ") + prop_ifname2 : "")
        << endlog();
    return false;
  }

}

bool SoemMasterComponent::startHook() {
  bool state_reached;

  setSlavesTargetState(EC_STATE_SAFE_OP);
  // Wait for all slaves to reach SAFE_OP state
  state_reached = checkSlavesStateReachedWaiting(EC_STATE_SAFE_OP,
  EC_TIMEOUTSTATE);

  if (EcatError)
    notifySoemErrors();

  if (!state_reached)
    return false;

  // Send one valid process data to make outputs in slaves happy
  ec_send_processdata();

  setSlavesTargetState(EC_STATE_OPERATIONAL);

  if (EcatError)
    notifySoemErrors();

  // Wait for all slaves to reach OP state
  state_reached = checkSlavesStateReachedWaiting(EC_STATE_OPERATIONAL,
  EC_TIMEOUTSTATE);

  if (EcatError)
    notifySoemErrors();

  if (!state_reached)
    return false;

  return true;
}

void SoemMasterComponent::updateHook() {
  bool success = true;
  Logger::In in(this->getName());
  if (ec_receive_processdata(EC_TIMEOUTRET) == 0) {
    success = false;
    log(Warning) << "receiving data failed" << endlog();
  }

  if (success)
    for (unsigned int i = 0; i < m_drivers.size(); i++)
      m_drivers[i]->update();

  if (ec_send_processdata() == 0) {
    success = false;
    log(Warning) << "sending process data failed" << endlog();
  }
  if (EcatError)
    notifySoemErrors();
}

void SoemMasterComponent::cleanupHook() {
  for (unsigned int i = 0; i < m_drivers.size(); i++) {
    this->provides()->removeService(m_drivers[i]->provides()->getName());
    delete m_drivers[i];
  }

  // stop SOEM, close socket
  ec_close();
}

bool SoemMasterComponent::writeCoeSdo(const AddressInfo& address,
    bool complete_access, int size, void* data) {
  if (ec_slave[address.slave_position].mbx_proto & ECT_MBXPROT_COE) {
    // Working counter of the EtherCAT datagram that ends the CoE
    // download procedure.
    // This value is written into the frame by the slave and is used
    // to confirm that the command has been executed.
    // In this case the expected working counter is 1.
    int working_counter = ec_SDOwrite(address.slave_position, address.index,
        address.sub_index, complete_access, size, data, EC_TIMEOUTRXM);

    if (working_counter == 1)
      return true;
    else {
      log(Warning) << "Slave_" << ec_slave[address.slave_position].configadr
          << " CoE SDO write failed for {index[" << address.index
          << "] sub_index[" << (int) address.sub_index << "] size " << size
          << "} wkc " << working_counter << endlog();

      return false;
    }
  } else {
    log(Error) << "Slave_" << ec_slave[address.slave_position].configadr
        << " does not support CoE" << " but a CoE SDO write has been requested"
            " for this slave." << endlog();

    return false;
  }
}

bool SoemMasterComponent::readCoeSdo(const AddressInfo& address,
    bool complete_access, int* size, void* data) {
  if (ec_slave[address.slave_position].mbx_proto & ECT_MBXPROT_COE) {
    // Working counter of the EtherCAT datagram that ends the CoE
    // download procedure.
    // This value is written into the frame by the slave and is used
    // to confirm that the command has been executed.
    // In this case the expected working counter is 1.
    int working_counter = ec_SDOread(address.slave_position, address.index,
        address.sub_index, complete_access, size, data, EC_TIMEOUTRXM);

    if (working_counter == 1)
      return true;
    else {
      log(Warning) << "Slave_" << ec_slave[address.slave_position].configadr
          << " CoE SDO read failed for {index[" << address.index
          << "] sub_index[" << (int) address.sub_index << "] size " << size
          << "} wkc " << working_counter << endlog();

      return false;
    }
  } else {
    log(Error) << "Slave_" << ec_slave[address.slave_position].configadr
        << " does not support CoE" << " but a CoE SDO read has been requested"
            " for this slave." << endlog();

    return false;
  }
}

void SoemMasterComponent::setSlavesTargetState(ec_state target_state) {
  log(Info) << "Request" << target_state << " state for all slaves" << endlog();
  ec_slave[0].state = (uint16) target_state;
  ec_writestate(0);
}

bool SoemMasterComponent::checkSlavesStateReachedWaiting(ec_state desired_state,
    int timeout) {
  bool state_is_reached = true;
  bool error_detected = false;

  uint16 network_state = ec_statecheck(0, desired_state, timeout);

  if ((network_state & EC_STATE_ERROR) == 0) {
    // No slave has toggled the error flag so the AlStatusCode
    // (even if different from 0) should be ignored
    for (int i = 0; i < ec_slavecount; i++) {
      ec_slave[i].ALstatuscode = 0x0000;
    }
  } else {
    error_detected = true;
  }

  switch (network_state) {
  case EC_STATE_INIT:
  case EC_STATE_PRE_OP:
  case EC_STATE_BOOT:
  case EC_STATE_SAFE_OP:
  case EC_STATE_OPERATIONAL:
    if (!error_detected) {
      // All the slaves have reached the same state so we can update
      // the state of every slave
      for (int i = 1; i <= ec_slavecount; i++) {
        ec_slave[i].state = network_state;
      }
    } else {
      // The state should be verified for every single slave
      // since at least one of them is in error
      ec_readstate();
    }
    break;

  default:
    // The state should be verified for every single slave
    // since not all have the same state
    ec_readstate();
    break;
  }

  if (ec_slave[0].state == desired_state) {
    log(Info) << (ec_state) ec_slave[0].state
        << " state reached for all slaves." << endlog();
  } else {
    log(Error) << "Not all slaves reached" << desired_state << endlog();

    // If not all slaves reached target state find out which one
    // It may happens that since more time is passing all the slaves reach the
    // target state before reading their state one by one.
    // In that case the timeout should be increased
    for (int i = 1; i <= ec_slavecount; i++) {
      if (ec_slave[i].state != desired_state) {
        state_is_reached = false;

        log(Error) << "Slave " << i << " State= "
            << (ec_state) ec_slave[i].state << " StatusCode="
            << ec_slave[i].ALstatuscode << " : "
            << ec_ALstatuscode2string(ec_slave[i].ALstatuscode) << endlog();
      }
    }
  }

  return state_is_reached;
}

bool SoemMasterComponent::notifySoemErrors() {
  // The EcatError flag is updated by SOEM library and set to 0 if
  // in the SOEM error list there are no more errors
  bool errorDetected = EcatError;

  // All the errors are retrieved from the list and notified
  while (EcatError) {
    log(Error) << ec_elist2string() << endlog();
  }

  return errorDetected;
}

}  // namespace

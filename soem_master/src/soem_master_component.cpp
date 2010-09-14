// Copyright  (C)  2008  Ruben Smits <ruben dot smits at mech dot kuleuven dot be>

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

#include "soem_master_component.h"

#include "ocl/Component.hpp"
ORO_CREATE_COMPONENT( soem_master::SoemMasterComponent )

namespace soem_master{

  using namespace RTT;
  
  SoemMasterComponent::SoemMasterComponent(const std::string& name) : 
    TaskContext(name,PreOperational),
    ifname_("eth0")
  {
    this->addProperty("ifname",ifname_).doc("interface to which the ethercat device is connected");
  }

  SoemMasterComponent::~SoemMasterComponent(){
  }

  bool SoemMasterComponent::configureHook(){
    if (!soem_master_.init(ifname_.c_str()))
      return false;
    
    const std::vector<SoemDriver*>& drivers = soem_master_.getDrivers();
    
    for(unsigned int i=0;i<drivers.size();i++){
      this->provides()->addService(drivers[i]->provides());
      drivers[i]->addPortsToTaskContext(this);
    }
    return true;
  }
    
  void SoemMasterComponent::updateHook(){
    const std::vector<SoemDriver*>& drivers = soem_master_.getDrivers();
    soem_master_.update();
    for(unsigned int i=0;i<drivers.size();i++)
      drivers[i]->updatePorts();
  }

  void SoemMasterComponent::cleanupHook(){
    soem_master_.close();
  }

}//namespace

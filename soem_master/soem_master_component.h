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
#ifndef SOEM_MASTER_COMPONENT_H
#define SOEM_MASTER_COMPONENT_H

#include <rtt/TaskContext.hpp>
#include <rtt/Property.hpp>

#include <vector>

#include "soem_driver.h"
#include "soem_master_types.hpp"

namespace soem_master
{

/** CoE addressing info */
struct AddressInfo
{
  /** Position of the slave in the EtherCAT network starting from 1 */
  uint16 slave_position;
  /** Index of the CoE object */
  uint16 index;
  /** Subindex of the CoE object*/
  uint8  sub_index;
};

class SoemMasterComponent: public RTT::TaskContext
{
public:
    SoemMasterComponent(const std::string& name);
    ~SoemMasterComponent();

protected:
    virtual bool configureHook();
    virtual bool startHook();
    virtual void updateHook();
    virtual void stopHook()
    {
    }
    ;
    virtual void cleanupHook();

private:
    std::string prop_ifname1, prop_ifname2;
    bool prop_redundant;
    char m_IOmap[4096];
    std::vector<SoemDriver*> m_drivers;
    std::vector<rtt_soem::Parameter> parameters;

    /**
     * Perform a CoE SDO write (blocking)
     * @param[in] address = structure that identifies the slave and the object
     * @param[in] complete_access = FALSE = single subindex.
     *                          TRUE = Complete Access, all subindexes written.
     * @param[in] size = Size in bytes of the object to be written
     * @param[in] data = Pointer to the data to be written
     * @return true if it succeeds false if it fails
     */
    bool writeCoeSdo(const AddressInfo& address, bool complete_access, int size, void* data);

    /**
     * Perform a CoE SDO read (blocking)
     * @param[in] address = structure that identifies the slave and the object
     * @param[in] complete_access = FALSE = single subindex.
     *                          TRUE = Complete Access, all subindexes written.
     * @param[in,out] size = Size in bytes of the read object
     * @param[out] data = Pointer to the read data
     * @return true if it succeeds false if it fails
     */
    bool readCoeSdo(const AddressInfo& address, bool complete_access, int* size, void* data);

    /**
     * Set the EtherCAT target state to be reached by all the slaves
     * @param[in] target_state = target EtherCAT state
     */
    void setSlavesTargetState(ec_state target_state);

    /**
     * Keep refreshing and checking the network state till all slaves reach the
     * desired state or a certain amount of time has elapsed (blocking)
     * @param[in] desired_state = target EtherCAT state
     * @param[in] timeout = timeout in ms
     * @return true if the state has been reached within the set timeout
     */
    bool checkSlavesStateReachedWaiting(ec_state desired_state, int timeout);

    /**
     * Notify if SOEM library detected an error. (non blocking)
     * This function is not blocking because it simply retrieve if an error has
     * already been detected.
     * Possible errors are: packet errors, CoE Emergency messages,
     *                      CoE SDO errors, FoE errors and SoE errors
     * Note: Only packets error are possible in EC_STATE_OPERATIONAL if the User
     *       does not explicitly require a mail box service (CoE, FoE, SoE)
     * @return true if at least one error has been detected
     */
    bool notifySoemErrors();

};//class

}//namespace

#endif

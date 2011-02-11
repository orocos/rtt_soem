/***************************************************************************
 tag: Ruben Smits, Koen Buys  Tue Nov 16 09:27:30 CET 2010  soem_master.h

 soem_master.h -  description
 -------------------
 begin                : Tue November 16 2010
 copyright            : (C) 2010 Ruben Smits, Koen Buys
 email                : first.last@mech.kuleuven.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

/*
 * This is the Ethercat master class based on the SOEM package
 *
 * @author: Koen Buys, Ruben Smits
 *
 *
 *
 */
#ifndef SOEM_MASTER_H
#define SOEM_MASTER_H

extern "C"
{
#include <ethercattype.h>
#include <ethercatmain.h>
}

#include <vector>

#include <soem_master/soem_driver.h>

namespace soem_master
{

class SoemMaster
{
    int nr_slaves_; //contains the number of found slave on the EC bus
    char IOmap_[4096];

public:

    SoemMaster();
    ~SoemMaster();
    /** 
     * Initialise the master and bind the socket to ifname
     * 
     * @param ifname ethernet interface used to connect to ethercat slaves
     * 
     * @return error code
     */
    bool init(const char *ifname);
    /** 
     * send updated data from all drivers to the slaves
     * 
     */
    bool send();
    /** 
     * receive data from slaves and update drivers data
     * 
     */
    bool receive();
    void update();
    void close();
    //void printstate();
    //should be protected, only public for developping

    const std::vector<SoemDriver*>& getDrivers();

protected:
    void createDrivers();
    std::vector<SoemDriver*> drivers_;

};
}

#endif

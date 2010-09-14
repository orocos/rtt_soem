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

extern "C"{
#include <ethercattype.h>
#include <ethercatmain.h>
}

#include <vector>

#include <soem_master/soem_driver.h>

namespace soem_master{

  
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

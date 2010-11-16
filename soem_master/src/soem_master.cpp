/***************************************************************************
  tag: Ruben Smits, Koen Buys  Tue Nov 16 09:27:00 CET 2010  soem_master.cpp

                        soem_master.cpp -  description
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

extern "C"{
#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatbase.h"
#include "ethercatmain.h"
#include "ethercatconfig.h"
#include "ethercatcoe.h"
#include "ethercatdc.h"
#include "ethercatprint.h"
}

#include <cstdio>
#include "soem_master.h"
#include <soem_master/soem_driver_factory.h>
#include <stdexcept>

namespace soem_master{
  
  SoemMaster::SoemMaster(){}
  
  SoemMaster::~SoemMaster(){
    //this->close();
  }
  
  bool SoemMaster::init(const char *ifname){
    
    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname)){	
      printf("ec_init on %s succeeded.\n",ifname);
      /* find and auto-config slaves */
      if ( ec_config(TRUE, &IOmap_) > 0 )
	{
	  //copy nr of slaves to class variable
	  nr_slaves_=ec_slavecount;
	  printf("%d slaves found and configured.\n",ec_slavecount);
	  /* wait for all slaves to reach SAFE_OP state */
	  ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);
	  if (ec_slave[0].state != EC_STATE_SAFE_OP )
	    {
	      printf("Not all slaves reached safe operational state.\n");
	      ec_readstate();
	      //If not all slaves operational find out which one
	      for(int i = 1; i<=ec_slavecount ; i++)
		{
		  if(ec_slave[i].state != EC_STATE_SAFE_OP)
		    {
		      printf("Slave %d State=%2x StatusCode=%4x : %s\n",
			     i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
		    }
		}
	    }
	  
	  //Configure distributed clock
	  ec_configdc();
	  //Read the state of all slaves
	  ec_readstate();
	  
	  printf("Request operational state for all slaves\n");
	  ec_slave[0].state = EC_STATE_OPERATIONAL;
	  /* request OP state for all slaves */
	  ec_writestate(0);
	  /* wait for all slaves to reach OP state */
	  ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);
	  if (ec_slave[0].state == EC_STATE_OPERATIONAL )
	    {
	      printf("Operational state reached for all slaves.\n");
	    }
	  else{
	    printf("Not all slaves reached operational state. \n");
	    return false;
	  }
	  
	  this->createDrivers();
	}	else
	{
	  printf("No slaves found!\n");
	  return false;
	}
      return true;
    }
    else
      {
	printf("No socket connection on %s\nExcecute as root\n",ifname);
	return false;
      }	
  }
  
  void SoemMaster::close(){   
    //delete all drivers
    for (unsigned int i=0;i<drivers_.size();i++)
      delete drivers_[i];

    //stop SOEM, close socket
    ec_close();
  }	
  
  
  bool SoemMaster::send(){
    return ec_send_processdata();
  }

  bool SoemMaster::receive(){
    return ec_receive_processdata(EC_TIMEOUTRET);
  }

  
  void SoemMaster::update(){
    if(ec_send_processdata()==0)
      throw std::runtime_error("sending process data failed");
    if(ec_receive_processdata(EC_TIMEOUTRET)==0)
      throw std::runtime_error("receiving data failed");
  }
  
  void SoemMaster::createDrivers()
  {
    for(int i=1;i<=nr_slaves_;i++){
      try{
	drivers_.push_back(SoemDriverFactory::Instance().createDriver(&ec_slave[i]));
	printf("Created driver for %s, with address %x\n",ec_slave[i].name,ec_slave[i].configadr);
      }catch(const std::exception& ex){
	printf("Could not create driver for %s: %s\n",ec_slave[i].name,ex.what());
      }
    }
  }

  const std::vector<SoemDriver*>& SoemMaster::getDrivers(){
    return drivers_;
  }



/*
void SoemMaster::printstate(){
  for(uint cnt = 1 ; cnt <= ec_slavecount ; cnt++)
    {
      printf("\nSlave:%d\n Name:%s\n Output size: %dbits\n Input size: %dbits\n State: %d\n Delay: %d[ns]\n Has DC: %d\n",
	     cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
	     ec_slave[cnt].state, ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);
      if (ec_slave[cnt].hasdc) printf(" DCParentport:%d\n", ec_slave[cnt].parentport);
      printf(" Activeports:%d.%d.%d.%d\n", (ec_slave[cnt].activeports & 0x01) > 0 ,
	     (ec_slave[cnt].activeports & 0x02) > 0 , 
	     (ec_slave[cnt].activeports & 0x04) > 0 , 
	     (ec_slave[cnt].activeports & 0x08) > 0 );
      printf(" Configured address: %4.4x\n", ec_slave[cnt].configadr);
      printf(" Man: %8.8x ID: %8.8x Rev: %8.8x\n", (int)ec_slave[cnt].eep_man, (int)ec_slave[cnt].eep_id, (int)ec_slave[cnt].eep_rev);
      printf(" SM0 A:%4.4x L:%4d F:%8.8x\n", ec_slave[cnt].SM[0].StartAddr, ec_slave[cnt].SM[0].SMlength, (int)ec_slave[cnt].SM[0].SMflags);
      printf(" SM1 A:%4.4x L:%4d F:%8.8x\n", ec_slave[cnt].SM[1].StartAddr, ec_slave[cnt].SM[1].SMlength, (int)ec_slave[cnt].SM[1].SMflags);
      printf(" SM2 A:%4.4x L:%4d F:%8.8x\n", ec_slave[cnt].SM[2].StartAddr, ec_slave[cnt].SM[2].SMlength, (int)ec_slave[cnt].SM[2].SMflags);
      printf(" SM3 A:%4.4x L:%4d F:%8.8x\n", ec_slave[cnt].SM[3].StartAddr, ec_slave[cnt].SM[3].SMlength, (int)ec_slave[cnt].SM[3].SMflags);
      printf(" SM types 0:%d 1:%d 2:%d 3:%d\n", ec_slave[cnt].SMtype[0], ec_slave[cnt].SMtype[1], ec_slave[cnt].SMtype[2], ec_slave[cnt].SMtype[3]);
      printf(" FMMU0 Ls:%8.8x Ll:%4d Lsb:%d Leb:%d Ps:%4.4x Psb:%d Ty:%2.2x Act:%2.2x\n",
	     (int)ec_slave[cnt].FMMU[0].LogStart, ec_slave[cnt].FMMU[0].LogLength, ec_slave[cnt].FMMU[0].LogStartbit,
	     ec_slave[cnt].FMMU[0].LogEndbit, ec_slave[cnt].FMMU[0].PhysStart, ec_slave[cnt].FMMU[0].PhysStartBit,
	     ec_slave[cnt].FMMU[0].FMMUtype, ec_slave[cnt].FMMU[0].FMMUactive);
      printf(" FMMU1 Ls:%8.8x Ll:%4d Lsb:%d Leb:%d Ps:%4.4x Psb:%d Ty:%2.2x Act:%2.2x\n",
	     (int)ec_slave[cnt].FMMU[1].LogStart, ec_slave[cnt].FMMU[1].LogLength, ec_slave[cnt].FMMU[1].LogStartbit,
	     ec_slave[cnt].FMMU[1].LogEndbit, ec_slave[cnt].FMMU[1].PhysStart, ec_slave[cnt].FMMU[1].PhysStartBit,
	     ec_slave[cnt].FMMU[1].FMMUtype, ec_slave[cnt].FMMU[1].FMMUactive);
      printf(" FMMU2 Ls:%8.8x Ll:%4d Lsb:%d Leb:%d Ps:%4.4x Psb:%d Ty:%2.2x Act:%2.2x\n",
	     (int)ec_slave[cnt].FMMU[2].LogStart, ec_slave[cnt].FMMU[2].LogLength, ec_slave[cnt].FMMU[2].LogStartbit,
	     ec_slave[cnt].FMMU[2].LogEndbit, ec_slave[cnt].FMMU[2].PhysStart, ec_slave[cnt].FMMU[2].PhysStartBit,
	     ec_slave[cnt].FMMU[2].FMMUtype, ec_slave[cnt].FMMU[2].FMMUactive);
      printf(" FMMUfunc 0:%d 1:%d 2:%d 3:%d\n",
	     ec_slave[cnt].FMMU0func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU3func);
      printf(" MBX length: %d MBX protocols : %2.2x\n", ec_slave[cnt].mbx_l, ec_slave[cnt].mbx_proto);
      ssigen = ec_siifind(cnt, ECT_SII_GENERAL);
      // SII general section 
      if (ssigen)
	{
	  ec_slave[cnt].CoEdetails = ec_siigetbyte(cnt, ssigen + 0x07);
	  ec_slave[cnt].FoEdetails = ec_siigetbyte(cnt, ssigen + 0x08);
	  ec_slave[cnt].EoEdetails = ec_siigetbyte(cnt, ssigen + 0x09);
	  ec_slave[cnt].SoEdetails = ec_siigetbyte(cnt, ssigen + 0x0a);
	  if((ec_siigetbyte(cnt, ssigen + 0x0d) & 0x02) > 0)
	    {
	      ec_slave[cnt].blockLRW = 1;
	      ec_slave[0].blockLRW++;						
	    }	
	  ec_slave[cnt].Ebuscurrent = ec_siigetbyte(cnt, ssigen + 0x0e);
	  ec_slave[cnt].Ebuscurrent += ec_siigetbyte(cnt, ssigen + 0x0f) << 8;
	  ec_slave[0].Ebuscurrent += ec_slave[cnt].Ebuscurrent;
	}
      printf(" CoE details: %2.2x FoE details: %2.2x EoE details: %2.2x SoE details: %2.2x\n",
	     ec_slave[cnt].CoEdetails, ec_slave[cnt].FoEdetails, ec_slave[cnt].EoEdetails, ec_slave[cnt].SoEdetails);
      printf(" Ebus current: %d[mA]\n only LRD/LWR:%d\n",
	     ec_slave[cnt].Ebuscurrent, ec_slave[cnt].blockLRW);
      if ((ec_slave[cnt].mbx_proto & 0x04) && printSDO)
	{
	  ODlist.Entries = 0;
	  memset(&ODlist, 0, sizeof(ODlist));
	  if( ec_readODlist(cnt, &ODlist))
	    {
	      printf(" CoE Object Description found, %d entries.\n",ODlist.Entries);
	      for( i = 0 ; i < ODlist.Entries ; i++)
		{
		  ec_readODdescription(i, &ODlist); 
		  while(EcatError)
		    {
		      printf("%s", ec_elist2string());
		    }
		  printf(" Index: %4.4x Datatype: %4.4x Objectcode: %2.2x Name: %s\n",
			 ODlist.Index[i], ODlist.DataType[i], ODlist.ObjectCode[i], ODlist.Name[i]);
		  memset(&OElist, 0, sizeof(OElist));
		  ec_readOE(i, &ODlist, &OElist);
		  while(EcatError)
		    {
		      printf("%s", ec_elist2string());
		    }
		  for( j = 0 ; j < ODlist.MaxSub[i]+1 ; j++)
		    {
		      if ((OElist.DataType[j] > 0) && (OElist.BitLength[j] > 0))
			{
			  printf("  Sub: %2.2x Datatype: %4.4x Bitlength: %4.4x Obj.access: %4.4x Name: %s\n",
				 j, OElist.DataType[j], OElist.BitLength[j], OElist.ObjAccess[j], OElist.Name[j]);
			  if ((OElist.ObjAccess[j] & 0x0007))
			    {
			      dtype = OElist.DataType[j];
			      printf("          Value :%s\n", SDO2string(cnt, ODlist.Index[i], j, OElist.DataType[j]));
			    }
			}
		    }	
		}	
	    }
	  else
	    {
	      while(EcatError)
		{
		  printf("%s", ec_elist2string());
		}
	    }
	}	
    }
}
*/

}//namespace soem_master

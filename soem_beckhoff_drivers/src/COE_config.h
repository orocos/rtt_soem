#ifndef COE_CONFIG_H
#define COE_CONFIG_H
   
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


 typedef struct{
	      uint16   index;
	      uint8    subindex;
	      uint8    size;
	      int      param;
	      string   name;
	      string   description;
	  }parameter;


#endif

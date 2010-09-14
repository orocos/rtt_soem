#ifndef SOEM_DRIVER_FACTORY_H
#define SOEM_DRIVER_FACTORY_H

#include <map>
#include <string>
#include "soem_driver.h"

namespace soem_master{

  class SoemDriverFactory{
  public:
    static SoemDriverFactory& Instance(){
      static SoemDriverFactory soem_driver_factory;
      return soem_driver_factory;
    }
  
    typedef SoemDriver* (*CreateDriverCallBack)(ec_slavet*);

    bool registerDriver(std::string name,CreateDriverCallBack createFn);

    SoemDriver* createDriver(ec_slavet* mem_loc);

  private:
    typedef std::map<std::string,CreateDriverCallBack> FactoryMap;
    FactoryMap factory_map_;

    SoemDriverFactory(){};
    SoemDriverFactory(const SoemDriverFactory&);
    SoemDriverFactory& operator=(const SoemDriverFactory&);
    ~SoemDriverFactory(){};
  };
}
#endif

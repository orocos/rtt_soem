#include <soem_master/soem_driver_factory.h>

namespace soem_master{
using namespace RTT;
  bool SoemDriverFactory::registerDriver(std::string name, CreateDriverCallBack create_fn){
	  Logger::In in("SoemDriverFactor");
	  log(Info)<<"Registering driver for "<<name<<endlog();
    return m_factory_map.insert(FactoryMap::value_type(name,create_fn)).second;
  }

  SoemDriver* SoemDriverFactory::createDriver(ec_slavet* mem_loc){
    FactoryMap::const_iterator it = m_factory_map.find(std::string(mem_loc->name));
    if (it == m_factory_map.end()){
      return NULL;
    }
    return (it->second)(mem_loc);
  }

  void SoemDriverFactory::displayAvailableDrivers(){
	  Logger::In in("SoemDriverFactory");
      	for(FactoryMap::const_iterator it=m_factory_map.begin();it!=m_factory_map.end();++it){
      		log(Info)<<it->first<<endlog();
      	}
  }
}

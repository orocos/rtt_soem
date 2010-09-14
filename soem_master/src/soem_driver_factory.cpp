#include <soem_master/soem_driver_factory.h>
#include <stdexcept>
namespace soem_master{

  bool SoemDriverFactory::registerDriver(std::string name, CreateDriverCallBack create_fn){
    return factory_map_.insert(FactoryMap::value_type(name,create_fn)).second;
  }

  SoemDriver* SoemDriverFactory::createDriver(ec_slavet* mem_loc){
    FactoryMap::const_iterator it = factory_map_.find(std::string(mem_loc->name));
    if (it == factory_map_.end()){
      //not found
      throw std::runtime_error("Unknown Driver Name");
    }
    return (it->second)(mem_loc);
  }
}

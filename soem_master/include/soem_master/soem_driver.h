#ifndef SOEM_DRIVER_H
#define SOEM_DRIVER_H

extern "C"{
#include <ethercattype.h>
#include <ethercatmain.h>
}


#include <rtt/Service.hpp>
#include <rtt/TaskContext.hpp>

#include <sstream>

template <class T>
inline std::string to_string (const T& t,std::ios_base & (*f)(std::ios_base&)){
	std::stringstream ss;
	ss << f << t;
	return ss.str();
};

namespace soem_master{

class SoemDriver
{
public:
	~SoemDriver(){
		m_service->clear();
	};

	const std::string& getName() const{
		return m_name;
	}

	RTT::Service::shared_ptr provides(){
		return RTT::Service::shared_ptr(m_service);
	};

	virtual void update()=0;
	virtual bool configure(){return true;};

protected:
	SoemDriver(ec_slavet* mem_loc) :
		m_datap(mem_loc),
		m_name("Slave_"+to_string(m_datap->configadr,std::hex)),
		m_service(new RTT::Service(m_name))
	{
	};
	ec_slavet* m_datap;
	std::string m_name;
	RTT::Service::shared_ptr m_service;

};
}
#endif

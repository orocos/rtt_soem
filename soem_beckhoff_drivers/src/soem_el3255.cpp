/***************************************************************************
 tag: Sergio Portoles  Tue Oct 27 13:36:50 CET 2015  soem_el3255.cpp
 Based on EL3104 of Ruben Smits
 
 soem_el3255.cpp -  description
 -------------------
 begin                : Tue October 27 2015
 copyright            : (C) 2015 Sergio Portoles
 email                : first.last@kuleuven.be

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

#include "soem_el3255.h"
#include <soem_master/soem_driver_factory.h>
#include <rtt/Property.hpp>
#include <iostream>

using namespace RTT;

namespace soem_beckhoff_drivers
{

SoemEL3255::SoemEL3255(ec_slavet* mem_loc) :
    soem_master::SoemDriver(mem_loc), m_size(5), m_raw_range(65535), m_lowest(
            0.0), m_highest(1.0), m_values(m_size), m_raw_values(m_size),m_params(m_size),
            m_values_port("values"), m_raw_values_port("raw_values")

{
    m_service->doc(std::string("Services for Beckhoff ") + std::string(
            m_datap->name) + std::string(" module"));
    m_service->addOperation("rawRead", &SoemEL3255::rawRead, this,
            RTT::OwnThread).doc("Read raw value of channel i").arg("i",
            "channel nr");
    m_service->addOperation("read", &SoemEL3255::read, this, RTT::OwnThread).doc(
            "Read value to channel i").arg("i", "channel nr");
    m_service->addOperation("Over_Range", &SoemEL3255::isOverrange, this,
            RTT::OwnThread).doc(
            "For the channel i : 1 = overrange ; 0 = no overrange ").arg("i",
            "channel nr");
    m_service->addOperation("Under_Range", &SoemEL3255::isUnderrange, this,
            RTT::OwnThread).doc(
            "For the channel i : 1 = Underrange ; 0 = no Underrange ").arg("i",
            "channel nr");
    m_service->addOperation("Comp_val_to_lim", &SoemEL3255::checkLimit,
            this, RTT::OwnThread).doc(
            "Limit 1/2 value monitoring of channel i :  0= not active, 1= Value is higher than    limit 1/2 value, 2= Value is lower than limit 1/2 value, 3: Value equals limit 1/2 value").arg(
            "i", "channel nr").arg("x", "Limit nr");
    m_service->addOperation("Error", &SoemEL3255::is_error, this).doc(
            "For the channel i : 1 = error (Overrange or Underrange ; 0 = no error ").arg(
            "i", "channel nr");

    m_resolution = ((m_highest - m_lowest) / (double) m_raw_range);

    m_service->addConstant("raw_range", m_raw_range);
    m_service->addConstant("resolution", m_resolution);
    m_service->addConstant("lowest", m_lowest);
    m_service->addConstant("highest", m_highest);

    m_service->addPort(m_values_port).doc(
            "AnalogMsg contain the read values of _all_ channels");
    m_service->addPort(m_raw_values_port).doc(
            "AnalogMsg containing the read values of _all_ channels");

    m_msg.values.resize(m_size);
    m_raw_msg.values.resize(m_size);
}

bool SoemEL3255::configure()
{
    return true;
}

void SoemEL3255::update()
{

    m_raw_msg.values[0] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->val_ch1;
    m_raw_msg.values[1] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->val_ch2;
    m_raw_msg.values[2] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->val_ch3;
    m_raw_msg.values[3] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->val_ch4;
    m_raw_msg.values[4] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->val_ch5;
    m_raw_values_port.write(m_raw_msg);

    for(int i=0;i<m_size;i++){
    	m_msg.values[i] = m_raw_msg.values[i] * m_resolution;
    }


    m_values_port.write(m_msg);

    m_params[0] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->param_ch1;
    m_params[1] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->param_ch2;
    m_params[2] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->param_ch3;
    m_params[3] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->param_ch4;
    m_params[4] = ((in_el3225_rtd_standard_t*) (m_datap->inputs))->param_ch5;

}

//rawRead : read the raw value of the input /////////////////////////////////////////////////////
int SoemEL3255::rawRead(unsigned int chan)
{
    if (chan < m_size)
    {
        return m_raw_msg.values[chan];
    }
    else
        log(Error) << "Channel " << chan << " outside of module's range"
                << endlog();
    return false;
}

//read: read the value of one of the 2 channels in Volts ///////////////////////////
double SoemEL3255::read(unsigned int chan)
{

    if (chan < m_size)
    {
        return m_msg.values[chan];
    }
    else
        log(Error) << "Channel " << chan << " is out of the module's range"
                << endlog();
    return 0.0;

}

//Checking overrange////////////////////////////////////////////////////////////////

bool SoemEL3255::isOverrange(unsigned int chan)
{
    if (chan < m_size)
        return m_params[chan].test(OVERRANGE);
    else
        log(Error) << "Channel " << chan << " is out of the module's range"
                << endlog();
    return false;
}

//Checking Underrange////////////////////////////////////////////////////////////////

bool SoemEL3255::isUnderrange(unsigned int chan)
{
    if (chan < m_size)
        return m_params[chan].test(UNDERRANGE);
    else
        log(Error) << "Channel " << chan << " is out of the module's range"
                << endlog();
    return false;
}

// Comparing the Value of Channel chan with its own limits (1/2)
bool SoemEL3255::checkLimit(unsigned int chan, unsigned int lim_num)
{
    if (!chan < m_size)
    {
        log(Error) << "Channel " << chan << " is out of the module's range"
                << endlog();
        return false;
    }
    if (!(lim_num > 0 && lim_num < 3))
    {
        log(Error) << "Limit nr " << lim_num << " should be 1 or 2" << endlog();
        return false;
    }
    if (lim_num == 1)
        return m_params[chan].test(LIMIT1SMALLER);
    else
        return m_params[chan].test(LIMIT2SMALLER);
}

// Checking for errors
bool SoemEL3255::is_error(unsigned int chan)
{
    if (chan < m_size)
        return m_params[chan].test(ERROR);
    else
        log(Error) << "Channel " << chan << " is out of the module's range"
                << endlog();
    return false;
}

namespace
{
soem_master::SoemDriver* createSoemEL3255(ec_slavet* mem_loc)
{
    return new SoemEL3255(mem_loc);
}


REGISTER_SOEM_DRIVER(EL3255, createSoemEL3255)

}

} //namespace soem_beckhoff_drivers



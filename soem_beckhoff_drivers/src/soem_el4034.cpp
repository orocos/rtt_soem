/***************************************************************************
 tag: Ruben Smits  Tue Nov 16 09:31:20 CET 2010  soem_el4034.cpp

 soem_el4034.cpp -  description
 -------------------
 begin                : Tue November 16 2010
 copyright            : (C) 2010 Ruben Smits
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

#include "soem_el4034.h"
#include <soem_master/soem_driver_factory.h>

namespace soem_beckhoff_drivers
{
SoemEL4034::SoemEL4034(ec_slavet* mem_loc) :
    soem_master::SoemDriver(mem_loc), size_(4),
            raw_range_(65536), //32768
            lowest_(-10.0), highest_(10.0), values_(size_), raw_values_(size_),
            values_port_(this->getName() + "_values"), raw_values_port_(
                    this->getName() + "_raw_values")
{

    service_->doc(std::string("Services for Beckhoff ") + std::string(
            datap_->name) + std::string(" module"));
    service_->addOperation("rawWrite", &SoemEL4034::rawWrite, this).doc(
            "Write raw value to channel i").arg("i", "channel nr").arg("value",
            "raw value");
    service_->addOperation("rawRead", &SoemEL4034::rawRead, this).doc(
            "Read raw value of channel i").arg("i", "channel nr");
    service_->addOperation("write", &SoemEL4034::write, this).doc(
            "Write value to channel i").arg("i", "channel nr").arg("value",
            "value");
    service_->addOperation("read", &SoemEL4034::read, this).doc(
            "Read value to channel i").arg("i", "channel nr");

    resolution_ = ((highest_ - lowest_) / (double) raw_range_);

    service_->addConstant("raw_range", raw_range_);
    service_->addConstant("resolution", resolution_);
    service_->addConstant("lowest", lowest_);
    service_->addConstant("highest", highest_);

    msg_.values.resize(size_);
    raw_msg_.values.resize(size_);

    for (unsigned int i = 0; i < size_; i++)
    {
        ((out_el4034t*) (datap_->outputs))->values[i] = 0;
    }

}

void SoemEL4034::addPortsToTaskContext(RTT::TaskContext* tc)
{
    tc->ports()->addPort(values_port_).doc(
            "AnalogMsg containing the desired values of _all_ channels");
    tc->ports()->addPort(raw_values_port_).doc(
            "AnalogMsg containing the desired values of _all_ channels");
}

void SoemEL4034::updatePorts()
{
    if (raw_values_port_.connected())
    {
        if (raw_values_port_.read(raw_msg_) == RTT::NewData)
            if (raw_msg_.values.size() <= size_)
                for (unsigned int i = 0; i < size_; i++)
                    ((out_el4034t*) (datap_->outputs))->values[i]
                            = raw_msg_.values[i];
    }
    if (values_port_.connected())
    {
        if (values_port_.read(msg_) == RTT::NewData)
            if (msg_.values.size() <= size_)
                for (unsigned int i = 0; i < size_; i++)
                    ((out_el4034t*) (datap_->outputs))->values[i]
                            = msg_.values[i] / resolution_;
    }
}

bool SoemEL4034::rawWrite(unsigned int chan, unsigned int value)
{
    if (chan < size_)
    {
        //Prevent overflow
        if (value < raw_range_)
        {
            ((out_el4034t*) (datap_->outputs))->values[chan] = value;
            return true;
        }
    }
    return false;
}

int SoemEL4034::rawRead(unsigned int chan)
{
    if (chan < size_)
        return ((out_el4034t*) (datap_->outputs))->values[chan];
    return -1;
}

bool SoemEL4034::write(unsigned int chan, double value)
{
    if (chan < size_)
    {
        if (value < highest_ && value >= lowest_)
        {
            ((out_el4034t*) (datap_->outputs))->values[chan] = (int) (value
                    / resolution_);
            return true;
        }
    }
    return false;
}

double SoemEL4034::read(unsigned int chan)
{
    if (chan < size_)
        return ((out_el4034t*) (datap_->outputs))->values[chan] * resolution_;
    return -1;
}

namespace
{
soem_master::SoemDriver* createSoemEL4034(ec_slavet* mem_loc)
{
    return new SoemEL4034(mem_loc);
}
const bool registered0 =
        soem_master::SoemDriverFactory::Instance().registerDriver("EL4034",
                createSoemEL4034);
}

}//namespace



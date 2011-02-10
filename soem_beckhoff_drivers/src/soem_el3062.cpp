/***************************************************************************
 tag: Ruben Smits  Tue Nov 16 09:31:20 CET 2010  soem_el3062.cpp

 soem_el3062.cpp -  description
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

#include "soem_el3062.h"
#include <soem_master/soem_driver_factory.h>
#include <rtt/Property.hpp>
#include <iostream>

namespace soem_beckhoff_drivers
{

SoemEL3062::SoemEL3062(ec_slavet* mem_loc) :
    soem_master::SoemDriver(mem_loc), m_size(2), m_raw_range(32768), m_lowest(
            0.0), m_highest(10.0), m_values(m_size), m_raw_values(m_size),
            m_values_port("values"), m_raw_values_port("raw_values")

{
    m_service->doc(std::string("Services for Beckhoff ") + std::string(
            m_datap->name) + std::string(" module"));
    m_service->addOperation("rawRead", &SoemEL3062::rawRead, this,
            RTT::OwnThread).doc("Read raw value of channel i").arg("i",
            "channel nr");
    m_service->addOperation("read", &SoemEL3062::read, this, RTT::OwnThread).doc(
            "Read value to channel i").arg("i", "channel nr");
    m_service->addOperation("Over_Range", &SoemEL3062::isOverrange, this,
            RTT::OwnThread).doc(
            "For the channel i : 1 = overrange ; 0 = no overrange ").arg("i",
            "channel nr");
    m_service->addOperation("Under_Range", &SoemEL3062::isUnderrange, this,
            RTT::OwnThread).doc(
            "For the channel i : 1 = Underrange ; 0 = no Underrange ").arg("i",
            "channel nr");
    m_service->addOperation("Comp_val_to_lim", &SoemEL3062::CompareV_to_Lim,
            this, RTT::OwnThread).doc(
            "Limit 1/2 value monitoring of channel i :  0= not active, 1= Value is higher than    limit 1/2 value, 2= Value is lower than limit 1/2 value, 3: Value equals limit 1/2 value").arg(
            "i", "channel nr").arg("x", "Limit nr");
    m_service->addOperation("Error", &SoemEL3062::is_error, this).doc(
            "For the channel i : 1 = error (Overrange or Underrange ; 0 = no error ").arg(
            "i", "channel nr");
    m_service->addOperation("TxPDO", &SoemEL3062::Tx_PDO, this).doc(
            "For the channel i :Valid of data of assigned TxPDO (0=valid, 1=invalid) ").arg(
            "i", "channel nr");
    m_service->addOperation("Toggle", &SoemEL3062::Tx_PDO_Toggle, this).doc(
            "The TxPDO Toggle is toggled by the slave, if the data of the corresponding PDO was updated").arg(
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

    // New properties : Component Configuration :  Can be completed
    // Need to be complete for a specific usage : See Beckof : http://www.beckhoff.com/EL3062/  
    //see  documentation (.xchm) / Commissionning / Object description and parameterization

    parameter temp;

    /// For parametrisation : uncomment the following code and complete it with using the document above
    temp.description = "Enable user scale";
    temp.index = 0x8010;
    temp.subindex = 0x01;
    temp.name = "E_user_scl";
    temp.size = 1;
    temp.param = 1;
    params.push_back(temp);

    temp.description = "Limite1ch1";
    temp.index = 0x8010;
    temp.subindex = 0x13;
    temp.name = "Limite1ch1";
    temp.size = 2;
    temp.param = 9174;
    params.push_back(temp);

    temp.description = "Limite2ch1";
    temp.index = 0x8010;
    temp.subindex = 0x14;
    temp.name = "Limite2ch1";
    temp.size = 2;
    temp.param = 24247;
    params.push_back(temp);

    // Adding properties to the Soem_master_Component.
    for (unsigned int i = 0; i < params.size(); i++)
    {
        m_service->addProperty(this->getName() + params[i].name,
                params[i].param).doc(params[i].description);
    }

}

bool SoemEL3062::configure()
{

    // Configuration of the slave
    parameter val;

    for (unsigned int i = 0; i < params.size(); i++)
    {

        while (EcatError)
            log(RTT::Error) << ec_elist2string() << RTT::endlog();

        temp_val = m_service->properties()->getProperty(this->getName()
                + params[i].name);
        val = temp_val.get();

        //assigning parameters
        ec_SDOwrite(((m_datap->configadr) & 0x0F), params[i].index,
                params[i].subindex, FALSE, params[i].size, &val, EC_TIMEOUTRXM);
    }
}

void SoemEL3062::update()
{
    if (m_raw_values_port.connected())
    {

        m_raw_msg.values[0] = ((out_el3062t*) (m_datap->inputs))->val_ch1;
        m_raw_msg.values[1] = ((out_el3062t*) (m_datap->inputs))->val_ch2;
    }

    if (m_values_port.connected())
    {

        m_msg.values[0] = (((out_el3062t*) (m_datap->inputs))->val_ch1
                * m_resolution);
        m_msg.values[1] = (((out_el3062t*) (m_datap->inputs))->val_ch2
                * m_resolution);
    }
}

//rawRead : read the raw value of the input /////////////////////////////////////////////////////
int SoemEL3062::rawRead(unsigned int chan)
{
    if (chan < m_size)
    {
        if (chan == 0)
            return ((out_el3062t*) (m_datap->inputs))->val_ch1;
        if (chan == 1)
            return ((out_el3062t*) (m_datap->inputs))->val_ch2;
    }
    return -1;
}

int SoemEL3062::read_param(unsigned int chan)
{
    if (chan < m_size)
    {
        if (chan == 0)
            return ((out_el3062t*) (m_datap->inputs))->param_ch1;
        if (chan == 1)
            return ((out_el3062t*) (m_datap->inputs))->param_ch2;
    }
    return -1;
}

//read: read the value of one of the 2 channels in Volts ///////////////////////////
double SoemEL3062::read(unsigned int chan)
{

    if (chan < m_size)
    {
        if (chan == 0)
            return (((out_el3062t*) (m_datap->inputs))->val_ch1 * m_resolution);
        if (chan == 1)
            return (((out_el3062t*) (m_datap->inputs))->val_ch2 * m_resolution);
    }
    return -1;

}

//Checking overrange////////////////////////////////////////////////////////////////

bool SoemEL3062::isOverrange(unsigned int chan)
{
    ch_par = read_param(chan);
    return ch_par[1];
}

//Checking Underrange////////////////////////////////////////////////////////////////

bool SoemEL3062::isUnderrange(unsigned int chan)
{
    ch_par = read_param(chan);
    return ch_par[0];
}

// Comparing the Value of Channel chan with its own limits (1/2)
int SoemEL3062::CompareV_to_Lim(unsigned int chan, unsigned int Lim_num)
{

    if ((Lim_num < 3) & (chan < m_size))
    {
        switch (Lim_num)
        {
        case 1:
            //Bogus
            return ((read_param(chan)) & (0x0c)) >> 2;//1100
            break;
        case 2:
            //Bogus
            return ((read_param(chan)) & (0x30)) >> 4;//110000
            break;
        default:
            return -1;
            break;
        }
    }
    return -1;
}

// Checking for errors
bool SoemEL3062::is_error(unsigned int chan)
{
    ch_par = read_param(chan);
    return ch_par[6];
}

//TxPDO
bool SoemEL3062::Tx_PDO(unsigned int chan)
{
    ch_par = read_param(chan);
    return ch_par[14];
}

// TxPDO Toggle
bool SoemEL3062::Tx_PDO_Toggle(unsigned int chan)
{
    ch_par = read_param(chan);
    return ch_par[15];
}

namespace
{
soem_master::SoemDriver* createSoemEL3062(ec_slavet* mem_loc)
{
    return new SoemEL3062(mem_loc);
}

const bool registered0 =
        soem_master::SoemDriverFactory::Instance().registerDriver("EL3062",
                createSoemEL3062);

}

}//namespace



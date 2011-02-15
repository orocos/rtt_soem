/***************************************************************************
 soem_ebox.cpp -  description
 -------------------
 begin                : Tue February 15 2011
 copyright            : (C) 2011 Ruben Smits
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
#include "soem_ebox.h"
#include <math.h>
#include <soem_master/soem_driver_factory.h>

namespace soem_ebox
{

SoemEBox::SoemEBox(ec_slavet* mem_loc) :
    soem_master::SoemDriver(mem_loc)
{
    this->m_service->doc("Services for SMF Ketels E/BOX");
    this->m_service->addOperation("readAnalog", &SoemEBox::readAnalog, this,
            RTT::OwnThread).doc("Read analog in value (in Volts)").arg("chan",
            "channel to read (shold be 0 or 1)");
    this->m_service->addOperation("checkBit", &SoemEBox::checkBit, this,
            RTT::OwnThread).doc("check value of Digital In").arg("bit",
            "input to check");
    this->m_service->addOperation("readEncoder", &SoemEBox::readEncoder, this,
            RTT::OwnThread).doc("Read Encoder value (in ticks)").arg("chan",
            "channel to read");
    this->m_service->addOperation("writeAnalog", &SoemEBox::writeAnalog, this,
            RTT::OwnThread).doc(
            "Set the value of the analog output chan to value (in Volts)").arg(
            "chan", "output channel to set").arg("value", "value to set");
    this->m_service->addOperation("setBit", &SoemEBox::setBit, this,
            RTT::OwnThread).doc("Set the digital output bit to value").arg(
            "bit", "digital output to set").arg("value", "value to set");
    this->m_service->addOperation("writePWM", &SoemEBox::writePWM, this,
            RTT::OwnThread).doc("Set the PWM channel to value (0..1)").arg(
            "chan", "PWM channel to set").arg("value", "value to set");

}

bool SoemEBox::configure()
{
    return true;
}

void SoemEBox::update()
{
    input = *((in_eboxt*) (m_datap->inputs));
    *(out_eboxt*) (m_datap->outputs) = output;
}

double SoemEBox::readAnalog(unsigned int chan)
{
    if (checkChannelRange(chan))
        return (double)input.analog[chan] * (double)EBOX_AIN_COUNTSTOVOLTS;
    else
        return 0.0;
}

bool SoemEBox::checkBit(unsigned int bit)
{
    if (checkBitRange(bit))
        return bitset<8> (input.digital).test(bit);
    else
        return false;
}

int SoemEBox::readEncoder(unsigned int chan)
{
    if (checkChannelRange(chan))
        return input.encoder[chan];
    else
        return false;
}

bool SoemEBox::writeAnalog(unsigned int chan, double value)
{
    if (checkChannelRange(chan))
    {
        int sign = (value > 0) - (value < 0);
        output.analog[chan] = sign * ceil(min(abs(value) / (double) EBOX_AOUT_MAX
                * EBOX_AOUT_COUNTS, (double) EBOX_AOUT_COUNTS));
        return true;
    }
    return false;
}

bool SoemEBox::setBit(unsigned int bit, bool value)
{
    if (checkBitRange(bit))
    {
        bitset < 8 > tmp(output.digital);
        tmp.set(bit, value);
        output.digital = tmp.to_ulong();
        return true;
    }
    return false;
}
bool SoemEBox::writePWM(unsigned int chan, double value)
{
    if (checkChannelRange(chan))
    {
        output.pwm[chan] = (int16)(value * EBOX_PWM_MAX);
        return true;
    }
    return false;
}

namespace
{
soem_master::SoemDriver* createSoemEBox(ec_slavet* mem_loc)
{
    return new SoemEBox(mem_loc);
}
const bool registered1 =
        soem_master::SoemDriverFactory::Instance().registerDriver("E/BOX",
                createSoemEBox);
}//namespace
}//namespace

#include <rtt/plugin/Plugin.hpp>

extern "C"
{
bool loadRTTPlugin(RTT::TaskContext* c)
{
    return true;
}
}

/***************************************************************************
 tag: Sergio Portoles  Tue Oct 27 13:36:50 CET 2015  soem_el3255.h
 Based on EL3104 of Ruben Smits

 soem_el3255.h -  description
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

#ifndef SOEM_EL3255_H
#define SOEM_EL3255_H

#include <soem_master/soem_driver.h>
#include <soem_beckhoff_drivers/AnalogMsg.h>
#include <rtt/Port.hpp>
#include <bitset>
#include <rtt/Property.hpp>
#include "COE_config.h"

namespace soem_beckhoff_drivers
{

class SoemEL3255: public soem_master::SoemDriver
{
    enum
    {
        UNDERRANGE = 0,
        OVERRANGE,
        LIMIT1SMALLER,
        LIMIT1HIGHER,
        LIMIT2SMALLER,
        LIMIT2HIGHER,
        ERROR,
	ST_1,
	ST_2,
	ST_3,
	ST_4,
	ST_5,
	ST_6,
	SYNC_ERROR,
	TXPDO_STATE,
	TXPDO_TOGGLE,
    };

    typedef struct
    PACKED {
      int16 param_ch1;
      int16 val_ch1;
      int16 param_ch2;
      int16 val_ch2;
      int16 param_ch3;
      int16 val_ch3;
      int16 param_ch4;
      int16 val_ch4;
      int16 param_ch5;
      int16 val_ch5;
    } in_el3225_rtd_standard_t;
    
    typedef struct
    PACKED {
      int16 value;
    } in_el3225_rtd_compact_t;
    
    public:
        SoemEL3255(ec_slavet* mem_loc);
        ~SoemEL3255()
        {
        }
        ;

        int rawRead(unsigned int chan);
        double read(unsigned int chan);
        bool isOverrange(unsigned int chan = 0);
        bool isUnderrange(unsigned int chan = 0);
        bool checkLimit(unsigned int chan = 0, unsigned int Lim_num = 0);
        bool is_error(unsigned int chan);

        void update();
        bool configure();

    private:

        const unsigned int m_size;
        const unsigned int m_raw_range;
        const double m_lowest;
        const double m_highest;
        double m_resolution;
        std::vector<std::bitset<16> > m_params;
        AnalogMsg m_msg;
        AnalogMsg m_raw_msg;
        std::vector<double> m_values;
        std::vector<double> m_raw_values;

        //Ports///////////
        RTT::OutputPort<AnalogMsg> m_values_port;
        RTT::OutputPort<AnalogMsg> m_raw_values_port;

    }; // End of class SoemEL3255

} // End of namespace soem_beckhoff_drivers

#endif

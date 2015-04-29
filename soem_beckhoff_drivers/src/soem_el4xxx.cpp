/***************************************************************************
 tag: Ruben Smits  Tue Nov 16 09:31:20 CET 2010  soem_EL4xxx.cpp

 soem_EL4xxx.cpp -  description
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

#include "soem_el4xxx.h"
#include <soem_master/soem_driver_factory.h>

namespace soem_beckhoff_drivers
{

namespace
{
soem_master::SoemDriver* createSoemEL4002(ec_slavet* mem_loc)
{
    return new SoemEL4xxx<2> (mem_loc, 32767, 0.0, 10.0);
}
soem_master::SoemDriver* createSoemEL4004(ec_slavet* mem_loc)
{
    return new SoemEL4xxx<4> (mem_loc, 32767, 0.0, 10.0);
}
soem_master::SoemDriver* createSoemEL4008(ec_slavet* mem_loc)
{
    return new SoemEL4xxx<8> (mem_loc, 32767, 0.0, 10.0);
}
soem_master::SoemDriver* createSoemEL4022(ec_slavet* mem_loc)
{
    return new SoemEL4xxx<2> (mem_loc, 32767, 4., 20.);
}
soem_master::SoemDriver* createSoemEL4032(ec_slavet* mem_loc)
{
    return new SoemEL4xxx<2> (mem_loc, 65535, -10.0, 10.0);
}
soem_master::SoemDriver* createSoemEL4034(ec_slavet* mem_loc) //Resolution wrong????
{
    return new SoemEL4xxx<4> (mem_loc, 65535, -10.0, 10.0);
}
soem_master::SoemDriver* createSoemEL4038(ec_slavet* mem_loc)
{
    return new SoemEL4xxx<8> (mem_loc, 65535, -10.0, 10.0);
}
soem_master::SoemDriver* createSoemEL4134(ec_slavet* mem_loc) // Added by Bert
{
    return new SoemEL4xxx<4> (mem_loc, 65535, -10.0, 10.0);
}


REGISTER_SOEM_DRIVER(EL4002, createSoemEL4002)
REGISTER_SOEM_DRIVER(EL4004, createSoemEL4004)
REGISTER_SOEM_DRIVER(EL4008, createSoemEL4008)
REGISTER_SOEM_DRIVER(EL4022, createSoemEL4022)
REGISTER_SOEM_DRIVER(EL4032, createSoemEL4032)
REGISTER_SOEM_DRIVER(EL4034, createSoemEL4034)
REGISTER_SOEM_DRIVER(EL4038, createSoemEL4038)
REGISTER_SOEM_DRIVER(EL4134, createSoemEL4134)

}

}//namespace



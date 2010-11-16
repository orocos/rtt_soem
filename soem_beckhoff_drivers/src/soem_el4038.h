/***************************************************************************
  tag: Ruben Smits  Tue Nov 16 09:30:46 CET 2010  soem_el4038.h

                        soem_el4038.h -  description
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


#ifndef SOEM_EL4038_H
#define SOEM_EL4034_H

#include <soem_master/soem_driver.h>
#include <soem_beckhoff_drivers/AnalogMsg.h>
#include <rtt/Port.hpp>
#include <bitset>

namespace soem_beckhoff_drivers{

  class SoemEL4038 : public soem_master::SoemDriver
  {
    
    typedef struct PACKED
    {
      uint16 values[8];
    } out_el4038t;
    
  public:
    SoemEL4038(ec_slavet* mem_loc);
    ~SoemEL4038(){};

    bool rawWrite( unsigned int chan, unsigned int value );
    int rawRead( unsigned int chan );
    
    bool write( unsigned int chan, double value );
    double read( unsigned int chan);
    

    void addPortsToTaskContext(RTT::TaskContext* tc);
    void updatePorts();
    
  private:
    const unsigned int size_;
    const unsigned int raw_range_;
    const double lowest_;
    const double highest_;
    double resolution_;

    AnalogMsg msg_;
    AnalogMsg raw_msg_;
    std::vector<double> values_; 
    std::vector<double> raw_values_; 
    RTT::InputPort<AnalogMsg> values_port_;
    RTT::InputPort<AnalogMsg> raw_values_port_;
  };
 
}
#endif

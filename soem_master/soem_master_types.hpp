/***************************************************************************
 soem_master_types.hpp -  description
 -------------------
 begin                : Wed October 12 2016
 copyright            : (C) 2016
 email                : luca@intermodalics.eu

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
#include <rtt/types/TemplateTypeInfo.hpp>
#include <rtt/types/StructTypeInfo.hpp>
#include <rtt/types/SequenceTypeInfo.hpp>
#include <rtt/typekit/StdTypeInfo.hpp>

namespace rtt_soem {
/** The structure that contains the information to be sent for each CoE SDO */
struct Parameter
{
  /** slave's index starting from 1 and depending on position */
  int slave_position;
  /** Index of the CoE object */
  int index;
  /** Subindex of the CoE object */
  int sub_index;
  /** Falg to enable complete access */
  bool complete_access;
  /** Size of the CoE object to be written in bytes */
  int size;
  /** The value of the parameter to be written (TODO change to a vector of chars to send parameters of any dimension)*/
  int param;
  std::string name;
  std::string description;

};
}

//################################################################

namespace boost {
namespace serialization {
// The helper function which you write yourself:
template<class Archive>
void serialize( Archive & a, rtt_soem::Parameter & cd, unsigned int) {
using boost::serialization::make_nvp;
a & make_nvp("slavePosition", cd.slave_position);
a & make_nvp("index", cd.index);
a & make_nvp("subIndex", cd.sub_index);
a & make_nvp("completeAccess", cd.complete_access);
a & make_nvp("size", cd.size);
a & make_nvp("param", cd.param);
a & make_nvp("name", cd.name);
a & make_nvp("description", cd.description);

}
}
}

// The RTT helper class which uses the above function behind the scenes:
struct parameterTypeInfo
: public RTT::types::StructTypeInfo<rtt_soem::Parameter>
{
parameterTypeInfo()
: RTT::types::StructTypeInfo<rtt_soem::Parameter>("Parameter")
{}
}; 


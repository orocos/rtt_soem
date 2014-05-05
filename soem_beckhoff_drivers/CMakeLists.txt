cmake_minimum_required(VERSION 2.6.3)

project(soem_beckhoff_drivers)
# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)

 
# Do setup in case of ros package, If ROS_ROOT is set, it is                                                                                                 # recommended to use RTT/OCL through the ros packages.                                                                                                       
#uncomment if you have defined messages
if(NOT ORO_USE_ROSBUILD)
  find_package(catkin REQUIRED message_generation rtt_roscomm)
  add_message_files( FILES
    AnalogMsg.msg
    CommMsgBig.msg
    CommMsg.msg
    DigitalMsg.msg
    EncoderMsg.msg
    EncoderInMsg.msg
    EncoderOutMsg.msg
    PSUMsg.msg
    )
  
  generate_messages()
endif()
# Set the CMAKE_PREFIX_PATH in case you're not using Orocos through
# ROS for helping these find commands find RTT.
find_package(OROCOS-RTT REQUIRED ${RTT_HINTS} )
# Defines the orocos_* cmake macros. See that file for additional
# documentation.
include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)

if(ORO_USE_ROSBUILD)
  rosbuild_genmsg()
  rosbuild_include(rtt_rosnode GenerateRTTtypekit)
endif()

ros_generate_rtt_typekit(soem_beckhoff_drivers)

find_package(soem REQUIRED)
include_directories(${soem_INCLUDE_DIRS})
  
orocos_use_package(soem_master)

orocos_plugin(soem_el1xxx src/soem_beckhoff_drivers.cpp src/soem_el1xxx.cpp)
target_link_libraries(soem_el1xxx ${soem_LIBRARIES})
orocos_plugin(soem_el2xxx src/soem_beckhoff_drivers.cpp src/soem_el2xxx.cpp)
target_link_libraries(soem_el2xxx ${soem_LIBRARIES})
orocos_plugin(soem_el4xxx src/soem_beckhoff_drivers.cpp src/soem_el4xxx.cpp)
target_link_libraries(soem_el4xxx ${soem_LIBRARIES})
orocos_plugin(soem_el3102 src/soem_beckhoff_drivers.cpp src/soem_el3102.cpp)
target_link_libraries(soem_el3102 ${soem_LIBRARIES})
orocos_plugin(soem_el5101 src/soem_beckhoff_drivers.cpp src/soem_el5101.cpp)
target_link_libraries(soem_el5101 ${soem_LIBRARIES})
orocos_plugin(soem_el6022 src/soem_beckhoff_drivers.cpp src/soem_el6022.cpp)
target_link_libraries(soem_el6022 ${soem_LIBRARIES})
orocos_plugin(soem_el30xx src/soem_beckhoff_drivers.cpp src/soem_el30xx.cpp)
target_link_libraries(soem_el30xx ${soem_LIBRARIES})
orocos_plugin(soem_el3104 src/soem_beckhoff_drivers.cpp src/soem_el3104.cpp)
target_link_libraries(soem_el3104 ${soem_LIBRARIES})

if(NOT ORO_USE_ROSBUILD)
  add_dependencies(soem_el1xxx ${PROJECT_NAME}_generate_messages_cpp)
  add_dependencies(soem_el2xxx ${PROJECT_NAME}_generate_messages_cpp)
  add_dependencies(soem_el4xxx ${PROJECT_NAME}_generate_messages_cpp)
  add_dependencies(soem_el3102 ${PROJECT_NAME}_generate_messages_cpp)
  add_dependencies(soem_el5101 ${PROJECT_NAME}_generate_messages_cpp)
  add_dependencies(soem_el6022 ${PROJECT_NAME}_generate_messages_cpp)
  add_dependencies(soem_el30xx ${PROJECT_NAME}_generate_messages_cpp)
  add_dependencies(soem_el3104 ${PROJECT_NAME}_generate_messages_cpp)
endif()
orocos_generate_package()

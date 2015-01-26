#
# The find_package macro for Orocos-RTT works best with
# cmake >= 2.6.3
#
cmake_minimum_required(VERSION 2.6.3)

#
# This creates a standard cmake project. You may extend this file with
# any cmake macro you see fit.
#
project(soem_ebox)

#uncomment if you have defined messages
if(NOT ORO_USE_ROSBUILD)
  find_package(catkin REQUIRED message_generation rtt_roscomm)
  add_message_files( FILES
    EBOXAnalog.msg
    EBOXDigital.msg
    EBOXOut.msg
    EBOXPWM.msg
    )
  
  generate_messages()
endif()


# Set the CMAKE_PREFIX_PATH in case you're not using Orocos through ROS
# for helping these find commands find RTT.
find_package(OROCOS-RTT REQUIRED ${RTT_HINTS})

# Defines the orocos_* cmake macros. See that file for additional
# documentation.
include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)

if(ORO_USE_ROSBUILD)
  rosbuild_genmsg()
  rosbuild_include(rtt_rosnode GenerateRTTtypekit)
endif()

ros_generate_rtt_typekit(soem_ebox)


find_package(soem REQUIRED)
include_directories(${soem_INCLUDE_DIRS})
  
orocos_use_package(soem_master)

#
# Components, types and plugins.
#
orocos_plugin(soem_ebox src/soem_ebox.cpp) # ...only one plugin function per library !
target_link_libraries(soem_ebox ${soem_LIBRARIES})
#

# Generates and installs our orocos-soem_ebox-<target>.pc and manifest.xml file
#
if(NOT ORO_USE_ROSBUILD)
  add_dependencies(soem_ebox ${PROJECT_NAME}_generate_messages_cpp)
endif()

orocos_generate_package()

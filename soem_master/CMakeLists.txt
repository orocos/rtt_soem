cmake_minimum_required(VERSION 2.4.6)

project(soem_master)

# Set the CMAKE_PREFIX_PATH in case you're not using Orocos through
# ROS for helping these find commands find RTT.
find_package(OROCOS-RTT REQUIRED ${RTT_HINTS} )
find_package(catkin QUIET)

# Defines the orocos_* cmake macros. See that file for additional
# documentation.
include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake)

find_package(soem REQUIRED)
include_directories(${soem_INCLUDE_DIRS})

#common commands for building c++ executables and libraries
orocos_library(soem_driver_factory soem_driver_factory.cpp)
orocos_component(soem_master soem_master_component.cpp)
target_link_libraries(soem_master soem_driver_factory ${soem_LIBRARIES})

orocos_plugin(soem_plugin soem_plugin.cpp)

orocos_install_headers( soem_driver.h soem_driver_factory.h)

install(PROGRAMS setcap.sh DESTINATION lib/${PROJECT_NAME})

orocos_generate_package( INCLUDE_DIRS ..)


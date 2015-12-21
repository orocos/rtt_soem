#setup

  - connect ethercat module using ethernet cable (outport of the module block)
  - set on your pc a (random) fixed IP to the port where you connected the cable
  - test your connection with the slave info script with the ethernet port number you use, eg.:
    -  ```sudo ./soem_core/bin/slaveinfo eth1``` or ```sudo /opt/ros/indigo/bin/slaveinfo eth1``` if installed from ros-indigo-soem-master
    - in case the latter fails because the executable does not find libsoem.so
      - ```sudo su```
      - ```export LD_LIBRARY_PATH=/opt/ros/indigo/lib```
      - ```/opt/ros/indigo/bin/slaveinfo eth1```
  - set permissions for the executables that will use soem: ```./setcaps.sh``` or ```./opt/ros/indigo/lib/soem_master/setcap.sh``` if installed from ros-indigo-soem-master
  - deploy a soem master component
  - set the component ifname property to the correct eth number, eg. eth1
  - configure and start the component

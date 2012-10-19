sudo setcap cap_net_raw+ep `which deployer-$OROCOS_TARGET`
sudo setcap cap_net_raw+ep `which rttlua-$OROCOS_TARGET`
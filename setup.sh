#!/bin/bash
sudo apt-get update -y
sudo apt-get dist-upgrade -y
sudo apt-get install rpi-update olsrd isc-dhcp-server libboost-all-dev git
sudo apt-get remove avahi-daemon
#recommended for troubleshooting, but not required
sudo apt-get install tshark screen tmux netemul vim
git clone https://github.com/mitchbertsch/lightnet.git
sudo rpi-update

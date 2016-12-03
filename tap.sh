#!/bin/bash
#tap0 MTU MAC IP
/etc/init.d/dhcpcd stop
ifconfig $1 down
ifconfig $1 mtu $2
ifconfig $1 hw ether $3
ifconfig $1 $4
olsrd

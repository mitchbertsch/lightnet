#!/bin/bash
#tap0 MTU MAC IP
echo $1
echo $2
echo $3
echo $4
ifconfig $1 down
ifconfig $1 mtu $2
ifconfig $1 hw ether $3
ifconfig $1 $4
#!/bin/bash
#tap0 MTU MAC IP
ifconfig $1 down
ifconfig $1 mtu $2
ifconfig $1 hw ether $3
ifconfig $1 $4

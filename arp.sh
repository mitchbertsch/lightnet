#!/bin/bash
#tap0 IP MAC
echo $1
echo $2
echo $3
arp -i $1 -s $2 $3
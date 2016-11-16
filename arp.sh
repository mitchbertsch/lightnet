#!/bin/bash
#tap0 IP MAC
arp -i $1 -s $2 $3

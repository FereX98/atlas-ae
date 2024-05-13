#!/bin/bash

sudo service opensm start

echo "Launching remoteswap server"
cd /home/osdi/remoteswap/server
nohup ./rswap-server 172.16.1.76 9999 64 96 &
sleep 10
#!/bin/bash

MOUNT_DIR=$(cd "$(dirname "$0")"; pwd)
WORK_DIR=/home/pi/work

if [ ! -d "/home/pi/work" ]; then
	mkdir /home/pi/work
fi
	
#if [ ! -d "/home/pi/.config/autostart" ]; then	
#	mkdir /home/pi/.config/autostart
#fi

#cp ir_ipc.desktop    /home/pi/.config/autostart/

cp -a update/*  $WORK_DIR

cp  wtdog  $WORK_DIR

cp rc.local   /etc/

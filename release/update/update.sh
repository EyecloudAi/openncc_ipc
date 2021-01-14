#!/bin/bash

MOUNT_DIR=$(cd "$(dirname "$0")"; pwd)
WORK_DIR=/home/pi/work

if [ -f $MOUNT_DIR/mainApp ];then
	rm $WORK_DIR/mainApp
	cp $MOUNT_DIR/mainApp  $WORK_DIR
fi

if [ -f $MOUNT_DIR/Rtsp ];then
	rm $WORK_DIR/Rtsp 
	cp $MOUNT_DIR/Rtsp     $WORK_DIR
fi

if [ -f $MOUNT_DIR/Onvif ];then	
	rm $WORK_DIR/Onvif 
	cp $MOUNT_DIR/Onvif    $WORK_DIR
fi

if [ -f $MOUNT_DIR/wtdog ];then
	rm $WORK_DIR/wtdog 
	cp $MOUNT_DIR/wtdog    $WORK_DIR
fi

if [ -f $MOUNT_DIR/config.ini ];then
	cp $MOUNT_DIR/config.ini     $WORK_DIR
fi

if [ -d $MOUNT_DIR/fw ];then
	rm -rf $WORK_DIR/fw 
	cp -a $MOUNT_DIR/fw    $WORK_DIR
fi

if [ -d $MOUNT_DIR/blob ];then	
	rm -rf $WORK_DIR/blob 
	cp  -a $MOUNT_DIR/blob  $WORK_DIR 
fi



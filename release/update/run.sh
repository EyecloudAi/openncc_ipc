#!/bin/bash
cd /home/pi/work 
echo "start lot_ipc server..."
sudo ./mainApp &
sudo ./Rtsp &
sudo ./Onvif &
sudo ./wtdog &


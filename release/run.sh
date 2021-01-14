#!/bin/bash
cd /chird/eye/work 
echo "start lot_ipc server..."
sudo ./mainApp
sudo ./Rtsp &
sudo ./Onvif &
sudo ./wtdog &


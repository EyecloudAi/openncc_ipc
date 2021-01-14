For  device source,it build in native method in rasberry platform.
We alse support toolchain compiler too, you can modify CROSS_COMPILE in Makefile to the compiler you used.

All  sources  included file folders: mainApp,onvif,Rtsp,wtdog,public,release in current path.

1:mainApp: core application,includes functions as:
1.1: usb communicated  with OpenNCC mode.
1.2: tcp remote control with client(private protocal)
1.3: handle others command from others application mode.
build:
run "make" in current directory ,it will build a output file(mainApp) in bin directory
but if you havn't install libusb,build will failed,please install it first:
sudo apt-get install libusb-dev
sudo apt-get install libusb-1.0-0-dev

2:Rtsp£º used live555 for playing rtsp streaming
build:
step1:run "./genMakefiles linux" to create makefile for live library.
step2:run "make" in live directory first,then enter current directory .
step3:run "make" too,it will build a output file(Rtsp) in current  directory

3:onvif£ºused opensource gSOAP2.8.17 to support onvif protocal 
build:
run "make" in current directory ,it will build a output file(Onvif) in current  directory

4:wtdog: 
4.1: monitor all applications just like(mainAPP,Rtsp,Onvif),if exit,run it at once
4.2: accept update request,update application automatic
build:
run "make" in current directory ,it will build a output file(wtdog) in current  directory

5: public:library for system resource.
include head files in include dirctory and libPlatform.a file

6: release:build release  package
you will find a update script which named update.sh in release/update .
step1: copy all bin files(mainApp,Onvif,Rtsp,wtdog) which you haved build on above steps 
or resource files(fw,blob,we have prepared them now) to directory release/update.
step2: enter current release path, execute  "tar -czvf update.ec update/*" to got a update package.
step3: used client tool IPCView to update application.
at the same time,used script file install.sh,you can install the all application to rasberry linux os
just execut "sudo ./install.sh"

For client application ,it include files folders:ClientSdk,ipc_viewer
build:
1:ClientSdk,it is a library for client sdk and some test code main.cpp
run "make" to get a simple test program

2:ipc_viewer,it is a application base on qt5.x framework,you can used qtcreator to open the project 
"ipc_viewer/IpcViewer/IpcViewer.pro" and build it.
you can find sdk files in directory "ipc_viewer/depends/cliSdk"

if build success,you will find a output bin file named IpcViewer in "ipc_viewer/build" directory,
you can copy the bin file to another folder(example:qt_release),at the same time you copy the "ipc_viewer/build/Configuration"
to the directory qt_release,and run "sudo linuxdeployqt IpcViewer -appimage" in qt_release path,you will get a qt 
release package.

 


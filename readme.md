### Introduce

   We prepared the openncc IPC viewer to demonstrate, you can use this tool to discover the Openncc IPC in the LAN, and connect with the camera. After the connection is successful, the video stream of the camera and the running results of the AI model can be obtained. At the same time, the camera can be controlled through the viewer to complete the online aI model upgrade and camera firmware upgrade. Of course, we also provide SDK package to facilitate secondary development and integration.  

  The application code of Raspberry Pi on openncc IPC device side is also opened. When the standard program cannot meet the requirements, developers can carry out secondary development on the device side, such as integrating the camera into their own cloud system.  

### Getting start

​		When you get the camera,you could reference 'doc/IPC user manual.docx' to get start.

### Client development

  For client application ,it included under these folders:

- ClientSdk

    It is a library for client sdk and some test codes. Run "make" to build the main.cpp sample code and could get a simple test program. It shows how to use the libclientsdk.a,also ClientSDK_API.pdf has more informations to show how to use the library.

- ipc_viewer

    It is a application based on QT 5.x framework,you can use QtCreator to open the project "ipc_viewer/IpcViewer/IpcViewer.pro" and build it. You can find sdk files in directory "ipc_viewer/depends/cliSdk".    

After built success,you will find a output bin file named IpcViewer in "ipc_viewer/build" directory,
  you can copy the bin file to another folder(example:qt_release),at the same time you need copy the "ipc_viewer/build/Configuration" to the directory qt_release,and run "sudo linuxdeployqt IpcViewer -appimage" in qt_release path,you will get a qt release package.  

### Update the models

  OpenNCC  IPC supports OpenVINO 2020.3.194 models,more information could click [here](http://eyecloud.gitee.io/openncc/Software_Manual.html#_3-openvino-installation-and-getting-start).

###  Raspberry Pi development

  For  device source,it build in native method in rasberry platform.  
  We also support toolchain compiler too, you can modify CROSS_COMPILE in Makefile to which the compiler you used.  All  sources  included under folders: 

- mainApp

  A main application,includes functions as:    

  1.1: usb communicated  with OpenNCC mode.    

  1.2: tcp remote control with client(private protocal)    

  1.3: handle others command from others application mode.    

  - Run "make" in current directory ,it will build a output file(mainApp) in bin directory
    but if you havn't install libusb,build will failed,please install it first:
    sudo apt-get install libusb-dev
    sudo apt-get install libusb-1.0-0-dev

- Onvif

  It is using opensource gSOAP2.8.17 to support onvif protocal 

  - build steps:
    run "make" in current directory ,it will build a output file(Onvif) in current  directory

- Rtsp

  It is using  live555 for playing rtsp streaming

  - build steps:
    - run "./genMakefiles linux" to create makefile for live library.
    - run "make" in live directory first,then enter current directory .
    - run "make" too,it will build a output file(Rtsp) in current  directory

- Wtdog

  Monitor all applications just like(mainAPP,Rtsp,Onvif),if exited,run it again.
  Could accept update request,update application automatic

  - build:
    run "make" in current directory ,it will build a output file(wtdog) in current  directory

- Public

  library for system resource. Included head files in include dirctory and libPlatform.a file

- Release 

  build release  package  
  you will find a update script which named update.sh in release/update .  
  step1: copy all bin files(mainApp,Onvif,Rtsp,wtdog) which you haved build on above steps 
  or resource files(fw,blob,we have prepared them now) to directory release/update.  
  step2: enter current release path, execute  "tar -czvf update.ec update/*" to got a update package.  
  step3: used client tool IPCView to update application.  
  At the same time,used script file install.sh,you can install the all application to rasberry linux os, type "sudo ./install.sh"
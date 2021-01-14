/*Copyright 2020 Eyecloud, Inc

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef DEVICEAGENT_H
#define DEVICEAGENT_H

#include <QString>
#include <QDebug>
#include <vector>
#include <QObject>
#include "cliSdk.h"

// change to another device,    close device and stop rtsp stream.
// restart app ,                close device and stop rtsp stream.
// update model,                close device and stop rtsp stream.
// interface error,             close device but do not stop rtsp stream.

// check opened of device befor interface called


// change main windows from one thread to two thread by QFeature

class DeviceAgent : public QObject
{
    Q_OBJECT
public:
    DeviceAgent(QString ip):ip_(ip){}

    bool checkOpen(){
        if(handle_ <= 0){
            return openDevice();
        }
        return true;
    }

    bool openDevice(){
        handle_ = ConnectToDevice(ip_.toLocal8Bit().toStdString().c_str());
        if(handle_ <= 0){
            return false;
        }
        return true;
    }

    bool closeDevice(){
        if(handle_ > 0){
            CloseDevice(handle_);
            handle_ = 0;
        }
    }

    bool restartMainApp(){
        if(!checkOpen()){
            qCritical()<<"Failed to restart App,can not connect device.";
            return false;
        }

        int ret = RebootByRemote(handle_,0);
        if(ret != 0){
            closeDevice();
            qCritical()<<"Failed to restart App, 'RebootByRemote' cmd return "
                      << ret;
            return false;
        }

        closeDevice();
    }

    bool readMetaData(BYTE* meta,int size){

        if(!checkOpen()){
            qCritical()<<"Failed to read meta data,can not connect device.";
            return false;
        }

        int ret = ReadMetaData(handle_,meta,size);
        if(ret != 0){
            closeDevice();
            qCritical()<<"Failed to read meta data, 'ReadMetaData' cmd return "
                      << ret;
            return false;
        }

        return true;
    }

    std::vector<QString> scanDevice(){

        std::vector<std::string>        rtspAddress;
        std::vector<QString>            qtRtspVec;

        ScanIPCAddr(3,rtspAddress);

        for(int i = 0;i < rtspAddress.size();i++){
            qtRtspVec.push_back(
                        QString::fromStdString(rtspAddress[i])
                        );
        }

        return qtRtspVec;
    }
    bool updateModel(
                     QString blobPath,
                     QString xmlPath,
                     QString modelName){

        if(!checkOpen()){
            qCritical()<<"Failed to update model,can not connect device.";
            return false;
        }

        int ret = UpdateAiModel(handle_,
                                blobPath.toLocal8Bit().toStdString().c_str(),
                                xmlPath.toLocal8Bit().toStdString().c_str(),
                                modelName.toLocal8Bit().toStdString().c_str()
                                );
        if(ret != 0){
            closeDevice();
            qCritical()<<"Failed to update model, 'UpdateAiModel' cmd return "
                      << ret;
            return false;
        }

        return true;

    }

    bool upgradeEc(QString ecPath){

        if(!checkOpen()){
            qCritical()<<"Failed to upgrade ec,can not connect device.";
            return false;
        }

        int ret = StartUpgrade(handle_,ecPath.toLocal8Bit().toStdString().c_str());
        if(ret != 0){
            closeDevice();
            qCritical()<<"Failed to upgrade pkg, 'StartUpgrade' ret="
                        << ret
                        <<  ", pkg path is "
                        << ecPath;
            return false;
        }

        return true;
    }

private:
    CLI_HANDLE                          handle_;
    QString                             ip_;
};

#endif // DEVICEAGENT_H

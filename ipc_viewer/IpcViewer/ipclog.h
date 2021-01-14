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
#ifndef IPCLOG_H
#define IPCLOG_H

#include <QObject>

class IpcLog : public QObject
{
    Q_OBJECT
public:
    explicit IpcLog(QObject *parent = nullptr);

public:
    static IpcLog& IpcLogger(){
        static IpcLog inst;
        return inst;
    }
    void info(QString& log);
    void error(QString& log);
signals:
    void sendLogToText(QString log);
};

static QString IpcLog_PlayThread_ReadyStop("==IpcLog_PlayThread_ReadyStop== ");
static QString IpcLog_PlayThread_FinishedStop("==IpcLog_PlayThread_FinishedStop==");

static QString IpcLog_PlayThread_ReadyPlay("==IpcLog_PlayThread_ReadyPlay==");
static QString IpcLog_PlayThread_FinishedPlay("==IpcLog_PlayThread_FinishedPlay==");

static QString IpcLog_PlayThread_ReadyRecvFrame("==IpcLog_PlayThread_ReadyRecvFrame==");
static QString IpcLog_PlayThread_RecvFirstFrame("==IpcLog_PlayThread_RecvFirstFrame==");

static QString IpcLog_PlayThread_PlayError("==IpcLog_PlayThread_PlayError==");
static QString IpcLog_PlayThread_PlayRtspError("==IpcLog_PlayThread_PlayRtspError==");

static QString IpcLog_PlayThread_OpeningFormat("==IpcLog_PlayThread_OpeningFormat==");

static QString IpcLog_UI_ReadyScanDevice("==IpcLog_UI_ReadyScanDevice==");
static QString IpcLog_UI_FinishedScanDevice("==IpcLog_UI_FinishedScanDevice==");


#endif // IPCLOG_H

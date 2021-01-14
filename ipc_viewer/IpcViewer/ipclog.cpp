// Copyright (C) 2020-2022 Eyecloud Corporation
// SPDX-License-Identifier: Apache-2.0
#include "ipclog.h"

IpcLog::IpcLog(QObject *parent) : QObject(parent)
{

}

void IpcLog::info(QString& log){

    emit sendLogToText("Info: " + log);
}

void IpcLog::error(QString& log){

    emit sendLogToText("Error: " + log);
}

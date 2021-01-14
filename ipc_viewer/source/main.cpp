// Copyright (C) 2020-2022 Eyecloud Corporation
// SPDX-License-Identifier: Apache-2.0
#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>
#include "videodecodethread.h"
#include <iostream>

#ifndef Q_OS_WIN
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#endif

/*TODO LIST:
 * 1. AVFrame to cv::Mat
 * 2. stop thread
 * 3. close handle
 * 4. log
 * */



/*
 * decode at one thread
 *
 * paint mat
 *
 * device opteration  -- change device  -- device connect invalid  --
 * ip, not equal,
 *
 *
 * ui - log
 **/



void setStyleByCss(){
    QFile styleSheet;
       styleSheet.setFileName(":/res/style.css");
       if(styleSheet.open(QFile::ReadOnly)) {
           QString styleString = styleSheet.readAll();
           styleSheet.close();
           static_cast<QApplication*>(QApplication::instance())->setStyleSheet(styleString);
           qInfo()<<"Successed set style!";
       }
}

//void test(){

//    VideoDecodeThread *videoDecodeT  = new VideoDecodeThread;

//    videoDecodeT->playerRtspStream("rtsp://192.168.0.11:8554/liveRGB");

//    getchar();
//}

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    //setStyleByCss();
    MainWindow w;

    w.show();

    return a.exec();
}

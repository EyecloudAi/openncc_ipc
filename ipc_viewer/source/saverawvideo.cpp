#include "saverawvideo.h"
#include <QDateTime>

void SaveRawVideo::startRecord(QString url, int delaySec, QString path){
    url_                =   url;
    delaySecToPlay_     =   delaySec;
    savePath_           =   path;
    start();
}

void SaveRawVideo::stopRecord(){

    requestInterruption();

    wait();
}

SaveRawVideo::PlayError SaveRawVideo::runPlay(QString rtspUrl){

    char                buf[1024]                   =   { 0 };
    int                 ret                         =   0;
    int                 openFormatTryMaxNumber      =   5;              // If failed openformat, try reopen.
    QString             logs;

    logs = QString("Open rtsp ")
            + url_
            + QString(", at play thread...");

    bool successeedOpenFormat = false;
    qInfo()<<"for record, openFormatTryMaxNumber + delaySecToPlay_= "<<openFormatTryMaxNumber + delaySecToPlay_;
    for(int i = 0;i < openFormatTryMaxNumber + delaySecToPlay_;i ++){
        QThread::sleep(3);
        if(!openFormat(url_)){
            closeFormat();
        }else{
            successeedOpenFormat = true;
            break;
        }
        if(isInterruptionRequested()){
            QString logs = "Exit video thread for `quit` cmd";
            return INTERRUPTION;
        }
    }

    if(!successeedOpenFormat){
        // notify wtdog to restart rtsp server.
        return RTSP_SERVER_ERROR;
    }

    lostTimer_.restart();

    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd-hh-mm-ss-zzz");

    QString filePath = savePath_ + QString("/") + current_date + QString(".dat");

    qInfo()<<"record filePath="<<filePath;


    int recvNumber=0;
    while (!isInterruptionRequested()){

        ret = av_read_frame(ic_, pkt_);
        if(0 != ret){
            memset(buf,0,1024);
            av_strerror(ret, buf, sizeof(buf) - 1);
            logs = QString("Failed play rtsp stream, 'av_read_frame' error=")
                    + QString::fromLocal8Bit(buf);
            break;
        }

        if(!binaryWriter_){
            binaryWriter_ = new BinaryWriter;
            bool openRet = binaryWriter_->open(filePath.toStdString());
            if(!openRet){
                qInfo()<<"Fatal, failed open "<< filePath;
                delete binaryWriter_;
                break;
            }
        }

        if(recvNumber++ % 200 == 0){
            qInfo()<<"recording.....";
        }

        binaryWriter_->append(pkt_->data,pkt_->size);
        av_packet_unref(pkt_);
    }

    if(isInterruptionRequested()){
        if(binaryWriter_){
            binaryWriter_->close();
            delete binaryWriter_;
            binaryWriter_ = nullptr;
        }
    }

    closeFormat();

    if(isInterruptionRequested()){
        return INTERRUPTION;
    }else{
        return NETWORK_ERROR;
    }

    return SaveRawVideo::PlayError::INTERRUPTION;
}

// Copyright (C) 2020-2022 Eyecloud Corporation
// SPDX-License-Identifier: Apache-2.0
#include "videodecodethread.h"

#include <QDateTime>
#include <iostream>
#include <thread>
#include <QImage>
#include <QDebug>
#include "ipclog.h"
#include <ctime>


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>

using namespace std;

static double r2d(AVRational r)
{
    return r.den == 0 ? 0:(double)r.num / (double)r.den;
}

VideoDecodeThread::VideoDecodeThread()
{
    vc_                         =   nullptr;
    vcodec_                     =   nullptr;
    swsCtx_                     =   nullptr;
    pkt_                        =   nullptr;
    frame_                      =   nullptr;
    ic_                         =   nullptr;
    videoStream_                =   -1;
    delaySecToPlay_             =   0;
    recvFrameNumber_            =   0;
}

void VideoDecodeThread::closeFormat(){

    if(pkt_){
        av_packet_free(&pkt_);
    }

    if(frame_){
        av_frame_free(&frame_);
    }

    if(vc_){
        avcodec_close(vc_);
    }

    if(ic_){
        avformat_close_input(&ic_);
    }

    if(swsCtx_){
        sws_freeContext(swsCtx_);
    }

    avformat_network_deinit();

    vc_                         =   nullptr;
    vcodec_                     =   nullptr;
    swsCtx_                     =   nullptr;
    pkt_                        =   nullptr;
    frame_                      =   nullptr;
    ic_                         =   nullptr;
    videoStream_                =   -1;
 //   delaySecToPlay_             =   0;
    recvFrameNumber_            =   0;
}

void VideoDecodeThread::playerRtspStream(QString url,int delaySec){
    url_                =   url;
    delaySecToPlay_     =   delaySec;
    start();
}

void VideoDecodeThread::stopRtspStream(){

    IpcLog::IpcLogger().info(IpcLog_PlayThread_ReadyStop);

    requestInterruption();

    wait();

    IpcLog::IpcLogger().info(IpcLog_PlayThread_FinishedStop);
}

bool VideoDecodeThread::openFormat(QString url){

    ic_                                     =   avformat_alloc_context();
    pkt_                                    =   av_packet_alloc();
    frame_                                  =   av_frame_alloc();

    char                buf[1024]           =   { 0 };
    int                 ret                 =   0;
    QString             logs;
    AVDictionary        *opts_              =   NULL;

    //av_register_all();
    avformat_network_init();
    //avcodec_register_all();

    av_dict_set(&opts_, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts_, "max_delay", "5000", 0);
    av_dict_set(&opts_, "stimeout", "5000000", 0);
    av_dict_set(&opts_, "initial_timeout", "15000000", 0);
    av_dict_set(&opts_, "probesize", "10240", 0);
    av_dict_set(&opts_, "infbuf", "1", 0);

    ret = avformat_open_input(
        &ic_,
        url.toStdString().c_str(),
        0,
        &opts_);

    if(ret != 0){

        av_strerror(ret, buf, sizeof(buf) - 1);

        logs = QString("Failed for 'avformat_open_input' ,url=")
                + url
                + QString(", error=")
                + QString::fromLocal8Bit(buf);

        IpcLog::IpcLogger().error(logs);
        return false;
    }

    avformat_find_stream_info(ic_, 0);
    av_dump_format(ic_,0,url.toStdString().c_str(),0);

    for (unsigned int i = 0; i < ic_->nb_streams; i++){

        AVStream *as = ic_->streams[i];
        if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            videoStream_ = i;
            logs = QString("Found one video Stream, codec_id= ")
                    + QString::number(as->codecpar->codec_id)
                    + QString(", format= ")
                    + QString::number(as->codecpar->format)
                    + QString(", width=")
                    + QString::number(as->codecpar->width)
                    + QString(", height=")
                    + QString::number(as->codecpar->height)
                    + QString(", video fps= ")
                    + QString::number(r2d(as->avg_frame_rate));

            IpcLog::IpcLogger().info(logs);
            break;
        }
    }

    if(-1 == videoStream_){

        logs = QString("Failed for can not found video stream at url=")
                + url_;

        IpcLog::IpcLogger().error(logs);
        return false;
    }

    vcodec_ = avcodec_find_decoder(ic_->streams[videoStream_]->codecpar->codec_id);
    if (!vcodec_){
        logs = QString("Failed for 'avcodec_find_decoder', codec id=")
                + QString::number(ic_->streams[videoStream_]->codecpar->codec_id);
        IpcLog::IpcLogger().error(logs);
        return false;
    }

    vc_ = avcodec_alloc_context3(vcodec_);
    avcodec_parameters_to_context(vc_, ic_->streams[videoStream_]->codecpar);
    vc_->thread_count = 8;

    ret = avcodec_open2(vc_, 0, 0);
    if (ret != 0){
        memset(buf,0,1024);
        av_strerror(ret, buf, sizeof(buf) - 1);
        logs = QString("Failed for 'avcodec_open2', error=")
                + QString::fromLocal8Bit(buf);
        IpcLog::IpcLogger().error(logs);
        return false;
    }

    return true;
}

// output times: 1-1-11 , fps: 30
void VideoDecodeThread::statisticMat(cv::Mat& mat){

    char                        fpsInfo[256];
    char                        timesInfo[256];
    long long                   lostTimes           = lostTimer_.elapsed();
    long long                   fps                 = -1;
    cv::Point                   origin;

    memset(fpsInfo,0,256);
    memset(timesInfo,0,256);

    if(lostTimes > 1000){
        fps = recvFrameNumber_ / (lostTimes / (long long)1000);
    }
    if(mat.rows < 240 || mat.cols < 240){
        qInfo()<<"Video resolution is too lower!";
        return;
    }

    long long allSec        =   lostTimes        / (long)1000;
    long long hour          =   allSec      / (long long)3600;
    long long min           =   (allSec     % (long long)3600) / (long long)60;
    long long sec           =   allSec      % 60;

    sprintf(timesInfo, "times: %lld-%lld-%lld ",hour,min,sec);
   // sprintf(fpsInfo, ", fps: %lld", fps);

   // strcat(timesInfo,fpsInfo);

    origin.x = 32 ;
    origin.y = 32;

    cv::putText(mat, timesInfo, origin, cv::FONT_HERSHEY_COMPLEX, 1,  cv::Scalar(255, 255, 128), 1, 8, 0);
}

VideoDecodeThread::PlayError VideoDecodeThread::runPlay(QString rtspUrl){
    char                buf[1024]                   =   { 0 };
    int                 ret                         =   0;
    bool                recv_I_Frame                =   false;          // Frames before the first keyframe are discarded
    bool                isFirstFlag                 =   true;
    int                 openFormatTryMaxNumber      =   5;              // If failed openformat, try reopen.
    QString             logs;

    IpcLog::IpcLogger().info(IpcLog_PlayThread_ReadyPlay);

    logs = QString("Open rtsp ")
            + url_
            + QString(", at play thread...");
    IpcLog::IpcLogger().info(logs);

    bool successeedOpenFormat = false;
    qInfo()<<"openFormatTryMaxNumber + delaySecToPlay_= "<<openFormatTryMaxNumber + delaySecToPlay_;
    for(int i = 0;i < openFormatTryMaxNumber + delaySecToPlay_;i ++){
        QThread::sleep(3);
        if(!openFormat(url_)){
            closeFormat();
        }else{
            successeedOpenFormat = true;
            break;
        }

        IpcLog::IpcLogger().info(IpcLog_PlayThread_OpeningFormat);
        if(isInterruptionRequested()){
            QString logs = "Exit video thread for `quit` cmd";
            IpcLog::IpcLogger().info(logs);
            return INTERRUPTION;
        }
    }

    if(!successeedOpenFormat){
        // notify wtdog to restart rtsp server.
        IpcLog::IpcLogger().info(IpcLog_PlayThread_PlayRtspError);
        return RTSP_SERVER_ERROR;
    }

    lostTimer_.restart();
    IpcLog::IpcLogger().info(IpcLog_PlayThread_ReadyRecvFrame);

    long long allPacketSize = 0;
    long long allPacketNum = 0;

    while (!isInterruptionRequested()){

        ret = av_read_frame(ic_, pkt_);
        if(0 != ret){

            IpcLog::IpcLogger().info(IpcLog_PlayThread_PlayError);

            memset(buf,0,1024);
            av_strerror(ret, buf, sizeof(buf) - 1);
            logs = QString("Failed play rtsp stream, 'av_read_frame' error=")
                    + QString::fromLocal8Bit(buf);
            IpcLog::IpcLogger().error(logs);
           // emit restartRtspPlay(url_);
            break;
        }

        if(pkt_->stream_index != videoStream_){
            av_packet_unref(pkt_);
            continue;
        }

        //qInfo()<<"size="<<pkt_->size;
        if(pkt_->flags & AV_PKT_FLAG_KEY){
            logs = QString("pkt->flags=")
                    + QString::number(pkt_->flags);
            IpcLog::IpcLogger().info(logs);
        }

        allPacketNum++;

        allPacketSize += pkt_->size;

        long long averageSize10 = (allPacketSize * (long long)10 )/ allPacketNum;
        long long myAverage10 = (pkt_->size * (long long)100 ) / averageSize10;
       // qInfo()<<"myAverage10="<<myAverage10;
        if(myAverage10 >= 13){
             pkt_->flags |= AV_PKT_FLAG_KEY;
        }

        if(pkt_->size > 100 * 900){
            pkt_->flags |= AV_PKT_FLAG_KEY;
            logs = QString("force set i frame, recvFrameNumber=")
                + QString::number(recvFrameNumber_);
            IpcLog::IpcLogger().info(logs);
        }

        if(!recv_I_Frame){
            if(!(pkt_->flags & AV_PKT_FLAG_KEY)){
                 av_packet_unref(pkt_);
                continue;
            }
            recv_I_Frame = true;
        }

        ret = avcodec_send_packet(vc_,pkt_);
        if(0 != ret && EAGAIN != ret){

            IpcLog::IpcLogger().info(IpcLog_PlayThread_PlayError);

            memset(buf,0,1024);
            av_strerror(ret, buf, sizeof(buf) - 1);
            logs = QString("Failed play rtsp stream, 'avcodec_send_packet' error=")
                    + QString::fromLocal8Bit(buf);
            IpcLog::IpcLogger().error(logs);
            av_packet_unref(pkt_);
            break;
        }

        av_packet_unref(pkt_);

        while(true){

            ret = avcodec_receive_frame(vc_,frame_);
            if(0 != ret){
                break;
            }
             recvFrameNumber_++;
            if(isFirstFlag){

                IpcLog::IpcLogger().info(IpcLog_PlayThread_RecvFirstFrame);
                swsCtx_ = sws_getContext(frame_->width,
                                         frame_->height,
                                         (enum AVPixelFormat)frame_->format,
                                         frame_->width,
                                         frame_->height,
                                         AV_PIX_FMT_BGR24,
                                         SWS_BICUBIC,
                                         NULL,
                                         NULL,
                                         NULL);

                isFirstFlag = false;
            }

            if(swsCtx_){

                cv::Mat mat;
                mat.create(cv::Size(frame_->width,frame_->height),
                         CV_8UC3);

                AVFrame *bgr24frame = av_frame_alloc();

                av_image_fill_arrays(bgr24frame->data,
                                        bgr24frame->linesize,
                                        mat.data,
                                        AV_PIX_FMT_BGR24,
                                        frame_->width,
                                        frame_->height,
                                        1);

                sws_scale(swsCtx_,
                        (const uint8_t* const*)frame_->data,
                        frame_->linesize,
                        0,
                        frame_->height,
                        bgr24frame->data,
                        bgr24frame->linesize);

                av_free(bgr24frame);

                statisticMat(mat);

                emit updateMat(mat);

            }
        }
    }

    closeFormat();

    IpcLog::IpcLogger().info(IpcLog_PlayThread_FinishedPlay);
    if(isInterruptionRequested()){
        return INTERRUPTION;
    }else{
        return NETWORK_ERROR;
    }
}

void VideoDecodeThread::run(){
    while(!isInterruptionRequested()){
        PlayError ret = runPlay(url_);
        if(RTSP_SERVER_ERROR == ret){
            emit rtspServerError(url_);
            break;
        }else if(INTERRUPTION == ret){
            break;
        }else{
            emit closeDevice();
        }

    }
}

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
#ifndef VIDEODECODETHREAD_H
#define VIDEODECODETHREAD_H

#include <QObject>
#include <QThread>
#include <QPixmap>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>

extern "C"{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
}

#include "aiutil.h"

class VideoDecodeThread : public QThread
{
    Q_OBJECT
public:

    enum PlayError{
        INTERRUPTION,
        RTSP_SERVER_ERROR,
        NETWORK_ERROR,
        FAILED_OPEN
    };

    VideoDecodeThread();

    void run() override;

    void playerRtspStream(QString url, int delaySec);
    void stopRtspStream();

    PlayError virtual runPlay(QString rtspUrl) ;

signals:
    void updateMat(cv::Mat mat);
    void restartRtspPlay(QString rtspUrl);
    void rtspServerError(QString rtspUrl);
    void closeDevice();

protected:
    bool openFormat(QString url);
    void closeFormat();
    void statisticMat(cv::Mat& mat);


protected:
    QString url_;

    AVFormatContext                     *ic_;
    AVCodecContext                      *vc_;
    AVCodec                             *vcodec_;
    struct SwsContext                   *swsCtx_;
    AVPacket                            *pkt_;
    AVFrame                             *frame_;
    int                                 videoStream_;
    int                                 delaySecToPlay_;          // Wait for a while before opening the package
    NccMideaElapsedTimer                lostTimer_;
    long long                           recvFrameNumber_;
};
#endif // VIDEODECODETHREAD_H

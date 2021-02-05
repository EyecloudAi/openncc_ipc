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

#ifndef SAVERAWVIDEO_H
#define SAVERAWVIDEO_H

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
#include "videodecodethread.h"

class SaveRawVideo : public VideoDecodeThread
{
    Q_OBJECT
public:

    SaveRawVideo(){
        binaryWriter_ = nullptr;
    }

    void startRecord(QString url, int delaySec, QString path);

    void stopRecord();

    PlayError runPlay(QString rtspUrl);

protected:
    BinaryWriter*   binaryWriter_;
    QString         savePath_;

};

#endif // SAVERAWVIDEO_H

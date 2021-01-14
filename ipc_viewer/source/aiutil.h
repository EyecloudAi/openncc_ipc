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
#ifndef AIUTIL_H
#define AIUTIL_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <QStringList>
#include <QDebug>
#include <QObject>
#include <chrono>

cv::Mat pintScoreOnMat(cv::Mat rgbImg, char *nnret, float minScore);
cv::Mat cls_show_img_func(cv::Mat rgbImg, char *nnret,float min_score);
cv::Mat fd_show_img_func(cv::Mat rgbImg, char *nnret,float min_score);



class NccMideaElapsedTimer {

public:
    NccMideaElapsedTimer() {
        startMillSec_ = 0;
    }

    inline void start(){
        if (0 == startMillSec_) {
            std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp
                = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
            startMillSec_ = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
        }
    }

    inline void restart(){
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp
            = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        startMillSec_ = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
    }

    inline long long elapsed() const{
        if (0 == startMillSec_) {
            return -1;
        }
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp
            = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
        long long curMillSec = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
        return curMillSec - startMillSec_;
    }
private:
    long long startMillSec_;
};

void millsecToTimes(long long msec,char* buf);


#endif // AIUTIL_H

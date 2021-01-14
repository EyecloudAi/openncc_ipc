// Copyright (C) 2020-2022 Eyecloud Corporation
// SPDX-License-Identifier: Apache-2.0
#include "aiutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <QStringList>

#include <QDebug>
#define printf qDebug

#include "Fp16Convert.h"

//#include<vector>
using namespace std;
using namespace cv;

extern QStringList valid_data_list;

typedef struct{
int id;
char type[100];
int  clolors;
}t_cls_type;

static t_cls_type cls_type[]=
{
        {1,	    "person", 0x0000ff},
        {17,	"cat",    0xffff00},
        {18,	"dog",    0x00ffff},
        {44,	"bottle", 0xff00ff},
        {47,	"cup",    0x800000},
        {48,	"fork",   0x00ff00},
        {49,	"knife",  0x00ff00},
        {50,	"spoon",  0x00ff00},
        {51,	"bowl",   0x00ff00},
        {52,	"banana", 0xffff00},
        {53,	"apple" , 0xff0000},
        {55,	"orange", 0x00ff00},
        {62,	"chair",  0xffff00},
        {64,	"plant", 0xffff00},
        {67,	"table", 0xffff00},
        {77,	"cell phone",0xff0000},
        {78,	"microwave", 0x808000},
        {80,	"toaster", 0x808000},
        {84,	"book", 0x800080},
        {85,	"clock", 0x800080},
        {86,	"vase", 0x800080},
        {87,	"scissors", 0x800080},
        {90,	"toothbrush", 0x000080}
};

static int coordinate_is_valid(float x1, float y1, float x2, float y2)
{
    if((x1<0) || (x1>1))
        return 0;
    if((y1<0) || (y1>1))
        return 0;
    if((x2<0) || (x2>1))
        return 0;
    if((y2<0) || (y2>1))
        return 0;
    if((x1>=x2) || (y1>=y2))
        return 0;

    return 1;
}

cv::Mat fd_show_img_func(cv::Mat rgbImg, char *nnret,float min_score){

    float x0, x1, y0, y1, score;
    uint16_t* cls_ret = (uint16_t*)(nnret);

    int width   = rgbImg.cols;
    int height  = rgbImg.rows;

    for (int i = 0; i < 100; i++){

        int image_id = (int)(f16Tof32(cls_ret[i*7+0]));
        if (image_id < 0) {
            break;
        }

        x0 = f16Tof32(cls_ret[i*7+3]);
        y0 = f16Tof32(cls_ret[i*7+4]);
        x1 = f16Tof32(cls_ret[i*7+5]);
        y1 = f16Tof32(cls_ret[i*7+6]);
        score =(float)f16Tof32(cls_ret[i*7+2]);

        qInfo()<<"fd_show_img_func: score="<<score;
        /* 不显示无效数据或者概率太低的框 */
        if( (coordinate_is_valid(x0, y0, x1, y1) ==0 )|| (score < min_score))
        {
            continue;
        }

        /* 画识别的人脸框 */
        cv::Rect box;
        box.x               = x0 * width;
        box.y               = y0 * height;
        box.width           = (x1 - x0) * width;
        box.height          = (y1 - y0) * height;

        if(box.width >= width - 5){
            box.width = width - 5;
        }

        if(box.height >= height - 5){
            box.height = height - 5;
        }

        cv::rectangle(rgbImg,
                      box,
                      cv::Scalar(255, 128, 128),
                      2,
                      8,
                      0);

        cv::Point origin;
        origin.x = box.x ;
        origin.y = box.y + 32;
        char   result[128];
        memset(result, 0, sizeof(result));
        sprintf(result, "score:%d%%", (int)(100*score));
        cv::putText(rgbImg,
                    result,
                    origin,
                    cv::FONT_HERSHEY_COMPLEX,
                    1,
                    cv::Scalar(255, 255, 128),
                    1,
                    8,
                    0);
    }

    return rgbImg;
}


cv::Mat cls_show_img_func(cv::Mat rgbImg, char *nnret,float min_score){
                            
    float x0, x1, y0, y1, score;
    uint16_t* cls_ret = (uint16_t*)(nnret);

    int width   = rgbImg.cols;
    int height  = rgbImg.rows;

    for (int i = 0; i < 100; i++){
        
        int image_id = (int)(f16Tof32(cls_ret[i*7+0]));
        if (image_id < 0) {
            break;
        }

        x0 = f16Tof32(cls_ret[i*7+3]);
        y0 = f16Tof32(cls_ret[i*7+4]);
        x1 = f16Tof32(cls_ret[i*7+5]);
        y1 = f16Tof32(cls_ret[i*7+6]);

        score =(float)f16Tof32(cls_ret[i*7+2]);
        /* 无效数据，不显示画框 */
        if((coordinate_is_valid(x0, y0, x1, y1) ==0 )|| (score < min_score)){
            continue;
        }

        /* 判断是否需要识别的物体 */
        int id =-1;
        int cls_id = (int)(f16Tof32(cls_ret[i*7+1]));
        for (int index = 0; 
             index < sizeof(cls_type) / sizeof(cls_type[0]);
             index++){
            
            if (cls_type[index].id == cls_id){
                id = index;
                break;
            }
        }
        /* 不是有效识别的物体，不显示画框 */
        if (id == -1){
            continue;
        }

        /* 画识别的物体框 */
        
        cv::Rect box;
        box.x               = x0 * width;
        box.y               = y0 * height;
        box.width           = (x1 - x0) * width;
        box.height          = (y1 - y0) * height;
        
        if(box.width >= width - 5){
            box.width = width - 5;
        }

        if(box.height >= height - 5){
            box.height = height - 5;
        }
        
        cv::rectangle(rgbImg,
                      box, 
                      cv::Scalar(
                                cls_type[id].clolors&0xff,
                                (cls_type[id].clolors&0xff00)>>8,
                                (cls_type[id].clolors&0xff0000)>>16
                                ),
                      2,
                      8,
                      0);
        
        cv::Point origin;
        origin.x = box.x ;
        origin.y = box.y + 32;
        
        char   result[128];
        
        memset(result, 0, sizeof(result));
        
        sprintf(result, 
                "%s:%d%%",
                cls_type[id].type,
                (int)(100*score)
                );
        
        cv::putText(rgbImg, 
                    result,
                    origin,
                    cv::FONT_HERSHEY_COMPLEX,
                    1,
                    cv::Scalar(cls_type[id].clolors&0xff,
                               (cls_type[id].clolors&0xff00)>>8,
                               (cls_type[id].clolors&0xff0000)>>16),
                    1,
                    8,
                    0);
    }

    return rgbImg;
}

cv::Mat pintScoreOnMat(cv::Mat rgbImg, char *nnret, float minScore){
    float x0, x1, y0, y1, score;
    uint16_t* cls_ret = (uint16_t*)nnret;

    int width   = rgbImg.cols;
    int height  = rgbImg.rows;

    for (int i = 0; i < 100; i++){
        int image_id = (int)(f16Tof32(cls_ret[i*7+0]));
        if (image_id < 0) {
            break;
        }

        x0 = f16Tof32(cls_ret[i*7+3]);
        y0 = f16Tof32(cls_ret[i*7+4]);
        x1 = f16Tof32(cls_ret[i*7+5]);
        y1 = f16Tof32(cls_ret[i*7+6]);

        score =(float)f16Tof32(cls_ret[i*7+2]);

        if( (coordinate_is_valid(x0, y0, x1, y1) ==0 )|| (score < minScore)){
            continue;
        }

        qInfo()<<"image id="<<image_id << ",score="<<score
              <<",minscore="<<minScore
              <<",x0="<<x0
             <<",y0="<<y0
            <<",x1="<<x1
           <<",y1="<<y1;

        /* 画识别的人脸框 */
        cv::Rect box;
        box.x               = x0 * width;
        box.y               = y0 * height;
        box.width           = (x1 - x0) * width;
        box.height          = (y1 - y0) * height;
        if(box.width >= width - 5){
            box.width = width - 5;
        }

        if(box.height >= height - 5){
            box.height = height - 5;
        }

        cv::rectangle(rgbImg, box, cv::Scalar(255, 128, 128), 2, 8, 0);

        cv::Point origin;
        origin.x = box.x ;
        origin.y = box.y + 32;

        char   result[128];
        memset(result, 0, sizeof(result));
        sprintf(result, "score:%d%%", (int)(100*score));

        cv::putText(rgbImg,
                    result,
                    origin,
                    cv::FONT_HERSHEY_COMPLEX,
                    1,
                    cv::Scalar(255, 255, 128),
                    1,
                    8,
                    0);
    }
    return rgbImg;
}


// input: 3671000
// output: 1 : 1 : 11
void millsecToTimes(long long msec,char* buf){
//    chrono::milliseconds mills{msec};
//    std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds> tp
//            = std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds>(mills);
//    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
//    std::tm* now = std::gmtime(&tt);
//    sprintf(buf,"%4d-%02d-%02d %02d:%02d:%02d",
//           now->tm_year+1900,
//           now->tm_mon+1,
//           now->tm_mday,
//           now->tm_hour,
//           now->tm_min,
//           now->tm_sec);

    long long allSec        =   msec        / (long)1000;
    long long hour          =   allSec      / (long long)3600;
    long long min           =   (allSec     % (long long)3600) / (long long)60;
    long long sec           =   allSec      % 60;

    sprintf(buf,"%lld :%lld :%lld ",hour,min,sec);
}

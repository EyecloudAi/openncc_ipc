/*
 Copyright (C) 2020-2022 Eyecloud Corporation

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
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <sys/types.h>

#define  VIDEO_RGB_KEY      0x11aa   //gui  mod get ir rgb data from mainApp
#define  VIDEO_SCREEN_KEY   0x11b0   //rtsp mod get screen data from gui 
#define  TEMP_MAIN_KEY      0x11cc   //gui  mod get temp data from mainApp 

#define  NCC_RGB_KEY        0x11dd   //gui  mod get rgb data from mainApp
#define  NCC_METADATA_KEY   0x11ee   //gui  mod get meta data from mainApp
#define  NCC_H264_KEY       0x11ff   //rtsp mod get h264 data from mainApp

#define  BLOCK_NUM      4

typedef struct _SingleFrame
{
    int length;
    char frame[256*1024-4];
}SingleFrame;

typedef unsigned long   SHARE_MEM_HANDLE;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//创建共享内存 如果存在返回key指定内存
//输入 唯一keyid 如 12345,内存大小
//输出 返回内存操作句柄 ,if err return -1
SHARE_MEM_HANDLE shr_open( key_t keyid,int blockSize,int blockNum);

//输入：内存操作句柄，输出：成功 0，失败 -1
int shr_close(SHARE_MEM_HANDLE handle);


//输入：内存操作句柄，数据指针，长度
//输出：成功 0，失败 -1
int shr_write(SHARE_MEM_HANDLE handle,void* buffer,int nBytes);


//输入：内存操作句柄，数据指针，长度
//输出：成功 0，失败 -1
int shr_read(SHARE_MEM_HANDLE handle,void* buffer,int nBytes);

int get_shr_size(SHARE_MEM_HANDLE handle);

//clear all buffer that have not read
int shr_reset_size(SHARE_MEM_HANDLE handle);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //#ifndef _CONFIG_H_


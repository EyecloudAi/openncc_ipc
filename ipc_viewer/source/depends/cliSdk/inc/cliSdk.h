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
#ifndef __CLI_SDK_H__
#define __CLI_SDK_H__

#include <string>
#include <vector> 
using namespace std; 

#define OUTPUT_INDEX_SIZE   64 

typedef unsigned char BYTE ;
typedef unsigned long   CLI_HANDLE;

typedef struct
{
     int      type;
     unsigned int  seqNo;
     unsigned int  size;
     unsigned int  res[13];
}frameSpecOut;

struct video_enc_param
{
	 int enc_type;//264=h264;265=h265
	 int enc_bps;  //unit KB
	 int res[6];  //resverd
};

enum CLI_STATUS
{
	ST_OK = 0,
	NO_READ_DATA = -1,
	NET_CLOSE = -2,
	RECV_ERROE = -3,
};

enum REBOOT_ACTION
{
	USB_MODE=0,
	SYS_RESTART=1,
	RTSP_MODE=2,
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void get_sdk_version( char* version);

//return 0: error,  >0: ok 
CLI_HANDLE ConnectToDevice(const char* device_ip,const char* filepath="./");

//all 0:ok -1:error
int CloseDevice(CLI_HANDLE handle);
int GetDeviceVer(CLI_HANDLE handle,char* version);
int GetAIMode(CLI_HANDLE handle,char* modename);

int DelAIMode(CLI_HANDLE handle);

// 1:restart device system   2:restart rtsp server
int RebootByRemote(CLI_HANDLE handle,int action);

int StartUpgrade(CLI_HANDLE handle,const char* up_file);

int UpdateAiModel(CLI_HANDLE handle,const char* ai_blob,const char* ai_xml,const char* mode_name);
int QueryUpStep(CLI_HANDLE handle,int step);//step from 0~100

//return -1: no data  -2:tcp close
int ReadMetaData(CLI_HANDLE handle,BYTE* pbuf,int& size);

int ScanIPCAddr(int nsecond,vector<string>& rtsp);

int SetEncParam(CLI_HANDLE handle,struct video_enc_param* enc);
int GetEncParam(CLI_HANDLE handle,struct video_enc_param* enc);

int SendCustomData(CLI_HANDLE handle,char* in_outbuf,int& in_outlen);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif



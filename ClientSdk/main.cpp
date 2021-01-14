// Copyright (C) 2020-2022 Eyecloud Corporation
// SPDX-License-Identifier: Apache-2.0
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mutex>

#include "sysinfo.h"
#include "cliSdk.h"
#ifndef _WINDOWS
#include <sys/time.h>
#else
#include <windows.h>	
extern "C" int gettimeofday(struct timeval *tp, void *tzp);
#pragma warning(disable:4996)
#endif

///////////////////////////////////////////////////////////////////////////////////
bool g_run = true;
int main(int argc ,char *argv[])
{
//  pthread_t ctrlThd;
//  pthread_create(&ctrlThd, NULL,ControlThread,NULL);
	CLI_HANDLE hdev;
#if  0
	vector<string> rtsp;
	int count=ScanIPCAddr(10,rtsp);
	for (int i=0;i<count;i++)
	{
		printf("query device rtsp url:%s \n",rtsp[i].c_str());
	}
	return 0;
#endif 		 
 	hdev=ConnectToDevice("192.168.0.5","./rec");
 			
  if (hdev==0)
  {
  		PTRACE("connect to server error!");
  		return -1;
  }
  else
  {
	  char aimode[100];
	  //GetAIMode(hdev,aimode);
	 // DelAIMode(hdev);
	  GetDeviceVer(hdev,aimode);
	  printf("cur mode :%s \n",aimode);  
	  struct video_enc_param enc;
	  GetEncParam(hdev,&enc);
	  printf("enc type:%d,bps:%d\n",enc.enc_type,enc.enc_bps);
	}
	
#if  0
  UpdateAiModel(hdev,"./eyecloud.blob","./eyecloud.xml","face-detection-adas");
  RebootByRemote(hdev,0);	
  return 0;
#endif
  
  BYTE meta[100*1024];
	while(g_run)
	{
		int size=sizeof(meta);
		int ret=ReadMetaData(hdev,meta,size);
		if	(ret==0)
		{
			frameSpecOut* out = (frameSpecOut*)meta;			
			printf("recv media seq=%d len=%d times=%d ms\n",out->seqNo,out->size,out->res[8]);
		}		
		else if (ret==NET_CLOSE)
			break;
		else 
			os_sleep(50);			
	}    		 
	printf("exit test main....\n");
	CloseDevice(hdev);
	return 0;
}

// Copyright (C) 2020-2022 Eyecloud Corporation
// SPDX-License-Identifier: Apache-2.0
//
#include "common.h"
#include "sdk.h"
#include "cameraCtrl.h"
#include "shm.h"
#include "times.hpp"
//#define USE_MJPEG
//#define TEST_GUI  
//#include "ping_pong.h"
#include <wiringPi.h>
#include "inifile.h"

static SHARE_MEM_HANDLE m_h264Mem;  //rtsp h26x data

static int cap_mode=1;   //1:h264  2:yuv420p

//ping_pong * pp=NULL;
static SingleFrame shframe;
#if 0
static void vscRead(void* param,void* data, int len)
{
    frameSpecOut *out = (frameSpecOut *)data;

  //	if (out->seqNo%10==0)
    	PTRACE("Meta:type:%d,size:%d,seqNo:%d addr=%p\n", out->type, out->size, out->seqNo,data);
    switch (out->type)
    {
        case( H26X) :
        {
       // 	CTimeUsed time("handle frame");    					
					frameSpecOut *out = (frameSpecOut *)data;		
				//PTRACE("Meta:type:%d,size:%d,seqNo:%d addr=%p h264_size=%d\n", out->type, out->size, out->seqNo,frame.data,h264_size);		
					uint8_t* pdata=(uint8_t*)out;
					pdata += sizeof(frameSpecOut);
											
					shframe.length=out->size;
					memcpy(shframe.frame,pdata,shframe.length);
					
					int ret=shr_write(m_h264Mem,&shframe,sizeof(SingleFrame));
					if (ret<0)
						PTRACE("write m_h264Mem status->ret: %d\n",ret);																						
       	}      
       	break;
       	case METEDATA:       	//report to remote client
       		meta_relay((char*)data,len);      	
       		break;
       	case  YUV420p:
       	{  		   					
       	}
        break;
        default:
            break;
    }
}
#endif

typedef enum _ncc_status
{
	NCC_INIT=0,
	NCC_OPEN,
	NCC_PLAY,
	NCC_CLOSE
}NCC_STATUS;

static NCC_STATUS ncc_st=NCC_INIT;
 
static int lowd_ncc_fw(bool breset=false)
{
	int ret=0;
	
	int enc_type=read_profile_int("ENC_PARAM","enc_type",265,"config.ini");		
	int bps=read_profile_int("ENC_PARAM","enc_bps",4000*1000,"config.ini");		
	printf("read enc_type=%d bps=%d\n",enc_type,bps);
	/*CameraInfo*/Network1Par cam_info =
	{
		-1,  //imageWidth
		-1,  //imageHeight                  
	    -1,  //startX
		-1,  //startY                 
		-1,   //endX
		-1,   //endY                  
		0,  //inputDimWidth
		0,   //inputDimHeight                   /* <dim>300</dim>  <dim>300</dim> */
		IMG_FORMAT_BGR_PLANAR,      //IMAGE_FORMAT   
		0,                    //meanValue
		0,
		0,
		1,                         //stdValue
	    1,                           /*??YUV420???????*/
	    1,                           /*??H26X??????*/
	    1,                           /*??MJPEG??????*/	  
		ENCODE_H265_MODE,            /* ???H264?????? */
		
		{0},                                    /* for 2 input param with 1 mode */
		0 ,                                  /* cascade with next AI mode*/
		1                                    /*Accelerating */
	};
	
	cam_info.mode=(enc_type==265)?ENCODE_H265_MODE:ENCODE_H264_MODE;

	wiringPiSetup();
	pinMode (0, OUTPUT) ;	
	if (breset)
	{
	  digitalWrite (0, HIGH) ;
	  os_sleep(1000);	
	  digitalWrite (0,  LOW) ;
	  os_sleep(1000);	
	}		 
	 
  //1. load firmware
   ret=load_fw("./moviUsbBoot","./fw/flicRefApp.mvcmd");
   if (ret<0)
   {
   		printf("lowd firmware error! return \n");
		//return NULL;
	//	exit(-1);

			exit(-1);
   }

   char version[100];
   get_sdk_version(version);
   printf("usb sersion:%d sdk version:%s \n",get_usb_version(),version);

 		SensorModesConfig mode[MAX_MODE_SIZE];
		int num=camera_control_get_sensorinfo(mode,MAX_MODE_SIZE);
	  for(int i=0;i<num;i++)
    {
        printf("[%d/%d]camera: %s, %dX%d@%dfps, AFmode:%d, maxEXP:%dus,gain[%d, %d]\n",i,num,
                mode[i].moduleName, mode[i].camWidth, mode[i].camHeight, mode[i].camFps,
                mode[i].AFmode, mode[i].maxEXP, mode[i].minGain, mode[i].maxGain);
    }	    

    int sensorModeId = 0; //1080P??
   // int sensorModeId = 1; //4K??
    camera_select_sensor(sensorModeId);
   // memcpy(&cameraCfg, &list.mode[sensorModeId], sizeof(cameraCfg));//select camera info

  cam_info.imageWidth  = mode[sensorModeId].camWidth;
  cam_info.imageHeight = mode[sensorModeId].camHeight; 
  cam_info.startX      = 0;
  cam_info.startY      = 0;
  cam_info.endX        = mode[sensorModeId].camWidth;
  cam_info.endY        = mode[sensorModeId].camHeight;
	
	char* ai_file=NULL;
	if (access(AI_BLOB_FILE,F_OK)==0)
	{
		ai_file=AI_BLOB_FILE;
	}
	ret=camera_control_set_bps(bps);
	if (ret<0)
		printf("camera_control_set_bps error ret=%d\n",ret);
	
	ret = sdk_init_ex(NULL,NULL, ai_file, &cam_info, sizeof(cam_info));	
//  ret = sdk_init(NULL, NULL, ai_file, &cam_info, sizeof(cam_info));
  printf("sdk_init return:%d \n", ret);
	if (ret<0)
	{
		exit(-1);
	} 	
	
	ncc_st=NCC_OPEN;
	PTRACE("%s  success!",__FUNCTION__);
	return ret;
}

int open_ncc(int capmode)
{
	if (ncc_st==NCC_PLAY) return -1;
	if  (capmode == 1)  
	{
		shr_reset_size(m_h264Mem);
		camera_video_out(H26X,VIDEO_OUT_CONTINUOUS); 
	}
	else if  (capmode == 2) //vga 
	{
	}
	ncc_st=NCC_PLAY;
	cap_mode = capmode;	
	PTRACE("switch NCC_PLAY success!");
	return 0;
}

int close_ncc()
{
		if (ncc_st==NCC_CLOSE) return -1;
		ncc_st=NCC_CLOSE;
		sdk_uninit();
		cap_mode=0;
		PTRACE("%s success!",__FUNCTION__);
				
		lowd_ncc_fw();
		return 0;
}

static char rbuf[300*1024];
void*  ncc_loop( void *arg)
{
	int ret;
	ncc_st=NCC_INIT;
 	m_h264Mem = shr_open(NCC_H264_KEY,sizeof(SingleFrame),BLOCK_NUM);		
 	if (m_h264Mem==0)
 	{
 		PTRACE("open share memory error!");	
 		return NULL;
 	}	 	

  lowd_ncc_fw(true);	
  open_ncc(1);
	while(core_context->running)	
	{	
		if (ncc_st!=NCC_PLAY)
		{
			os_sleep(100);
			continue;
		}
		
		int size=sizeof(rbuf);
		ret=read_26x_data(rbuf,&size,1);
		if (ret==USB_ERROR_NO_DEVICE)
		{
			PTRACE("recv no device !%d",ncc_st);
			if (ncc_st==NCC_PLAY)
			{
				system("killall Rtsp");
				exit(1);
			}
		}		
		if (ret==0)
		{
				frameSpecOut *out = (frameSpecOut *)rbuf;		
				//PTRACE("Meta:type:%d,size:%d,seqNo:%d \n", out->type, out->size, out->seqNo);		
				shframe.length=out->size;
				uint8_t* pdata=(uint8_t*)out;
				pdata += sizeof(frameSpecOut);
												
				memcpy(shframe.frame,pdata,shframe.length);
				
				ret=shr_write(m_h264Mem,&shframe,sizeof(SingleFrame));			
				//if (ret<0)	
				//	PTRACE("shr_write 264 mem error!");
			 // else PTRACE("shr_write 264 mem  success!=%d",out->size);		
		}
			
		size=sizeof(rbuf);
		ret=read_meta_data(rbuf,&size,0);
		if (ret==0)
		{
				meta_relay(rbuf,size);  
		}				
	}    		 
	printf("exit ncc loop....\n");
	
	if (m_h264Mem>0)
	{
	  shr_close(m_h264Mem);
	  m_h264Mem=0;
  }
 
	return NULL;
}

int  get_ncc_status()
{
		return ncc_st;
}


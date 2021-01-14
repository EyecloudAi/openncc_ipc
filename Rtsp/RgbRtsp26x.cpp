#include "RgbRtsp26x.h"
#include "sps_decode.h"

#include<fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <mqueue.h>

//#include "video.h"
//#include "video_codec.h"
#include "message.h"

//#include "v4l2_device.h"  

/*********************for OpenNCC************************/
#include <sys/time.h>
//#include "sdk.h"
//#include "cameraCtrl.h"

//static char metadata[5*1024];

//Table 7-1 h264 NAL unit type codes 
#define NAL_UNIT_TYPE_UNSPECIFIED                    0    // Unspecified
#define NAL_UNIT_TYPE_CODED_SLICE_NON_IDR            1    // Coded slice of a non-IDR picture
#define NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_A   2    // Coded slice data partition A
#define NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_B   3    // Coded slice data partition B
#define NAL_UNIT_TYPE_CODED_SLICE_DATA_PARTITION_C   4    // Coded slice data partition C
#define NAL_UNIT_TYPE_CODED_SLICE_IDR                5    // Coded slice of an IDR picture
#define NAL_UNIT_TYPE_SEI                            6    // Supplemental enhancement information (SEI)
#define NAL_UNIT_TYPE_SPS                            7    // Sequence parameter set
#define NAL_UNIT_TYPE_PPS                            8    // Picture parameter set
#define NAL_UNIT_TYPE_AUD                            9    // Access unit delimiter
#define NAL_UNIT_TYPE_END_OF_SEQUENCE               10    // End of sequence
#define NAL_UNIT_TYPE_END_OF_STREAM                 11    // End of stream
#define NAL_UNIT_TYPE_FILLER                        12    // Filler data
#define NAL_UNIT_TYPE_SPS_EXT                       13    // Sequence parameter set extension
											 // 14..18    // Reserved
#define NAL_UNIT_TYPE_CODED_SLICE_AUX               19    // Coded slice of an auxiliary coded picture without partitioning


//hevc nal unit types
#define CODED_SLICE_0  0
#define CODED_SLICE_1  1
#define NAL_IDR_W_RADL      19
#define NAL_IDR_N_LP        20
#define VIDEO_PARAMETER_SET    32
#define SEQUENCE_PARAMETER_SET 33
#define PICTURE_PARAMETER_SET  34
#define ACCESS_UNIT_DELIMITER  35
#define NAL_UNIT_FILLER_DATA   38

#define NAL_START_CODE_SIZE 4 //00 00 00 01

static uint8_t start_code[4]={0x0,0x0,0x0,0x1};
//static uint32_t cntFrame=0;
const char* frame264_des[]=
{
"TYPE_UNSPECIFIED"                   ,    // 0    // Unspecified
"TYPE_CODED_SLICE_NON_IDR"           ,    //1    // Coded slice of a non-IDR picture
"TYPE_CODED_SLICE_DATA_PARTITION_A" ,     //  2    // Coded slice data partition A
"TYPE_CODED_SLICE_DATA_PARTITION_B"  ,  //3    // Coded slice data partition B
"TYPE_CODED_SLICE_DATA_PARTITION_C"  ,   //4    // Coded slice data partition C
"TYPE_CODED_SLICE_IDR"               ,     //5    // Coded slice of an IDR picture
"TYPE_SEI"                           ,   //6    // Supplemental enhancement information (SEI)
"TYPE_SPS"                           ,   //7    // Sequence parameter set
"TYPE_PPS"                              //8    // Picture parameter set	
};

static void vscRead(void*param,void* data,int size);

static void* threadRecv(void* This)
{
	((RgbRtsp26x *)This)->threadRecvFunc(This);
	return NULL;
}

static void* threadRgb(void* This)
{
	((RgbRtsp26x *)This)->threadRgbFunc(This);
	return NULL;
}

RgbRtsp26x::RgbRtsp26x(bool h265)
{
	m_isInitMeta = true;
	m_running = false;
	m_h265 = false;
	//tick_gap = 1000 / 25;  //default; 
	m_ntick = 0;
	m_h265 = h265;
	
	m_h264Mem = 0;
}

// Close stream on destruction.
RgbRtsp26x::~RgbRtsp26x() 
{

}

//#############################################
bool RgbRtsp26x::Create(MSG_Q_ID msgid,bool self_thrd)
{	
	int ret=-1;
	m_msgid = msgid;

  m_h264Mem=shr_open(NCC_H264_KEY,sizeof(SingleFrame),BLOCK_NUM);	
 	if (m_h264Mem==0)
 	{
 		PTRACE("open share memory error!");	
 		return false;
 	}

  m_running = true;
  
	ret =pthread_create(&vid_rgb, NULL/*&attr*/, threadRgb, this);
	if (ret != 0)
	{
			PTRACE("create rgb thread error!\n");
			Close();
			return false;
	}	
			
	if (self_thrd)
	{
		printf("start create recv rtsp request thread !\n");
		ret =pthread_create(&vid_rth, NULL/*&attr*/, threadRecv, this);
		if (ret != 0)
		{
			PTRACE("create recv thread error!\n");
			Close();
			return false;
		}	
	}
	
	m_isInitMeta = true;
	//printf("create threads success!pipe fd:%d \n",pipe_fd);
	return true;//(pipe_fd>0);
}

bool RgbRtsp26x::check265Send(uint8_t nal_unit_type)
{
    //do not send data if nal type is access unit delimiter, live555 does recognize when AU is terminated
    if((nal_unit_type == ACCESS_UNIT_DELIMITER) ||(NAL_UNIT_FILLER_DATA==nal_unit_type))
      return false;

    //vps,sps,pps sent only once because live server memorize them and it will be streamed in the rtp header when session is opened
    //TODO check when vps/sps/pps changes and send it again
    if( (nal_unit_type == VIDEO_PARAMETER_SET) || 
        (nal_unit_type == SEQUENCE_PARAMETER_SET) ||
        (nal_unit_type == PICTURE_PARAMETER_SET))
      {
      		return true;     		
          if(m_isInitMeta) 
            return true;
          else
            return false;
      }
      else
      {
      	if ((nal_unit_type==NAL_IDR_W_RADL)||(nal_unit_type==NAL_IDR_N_LP))
      		PTRACE("Got I frame!");
        return  true;
      }
}

bool RgbRtsp26x::check264Send(uint8_t nal_unit_type)
{
    //do not send data if nal type is access unit delimiter, live555 does recognize when AU is terminated   
    if(nal_unit_type == NAL_UNIT_TYPE_AUD)
      return false;
      	 
    if( (nal_unit_type == NAL_UNIT_TYPE_SPS) ||
        (nal_unit_type == NAL_UNIT_TYPE_PPS))
    {  
    		return true;    				
        if(m_isInitMeta) 
          return true;
        else
          return false;
    }
    else if (nal_unit_type==NAL_UNIT_TYPE_CODED_SLICE_IDR)
    {  
    	PTRACE("Got I frame!");
      return  true;
    }	
    else if (nal_unit_type==NAL_UNIT_TYPE_CODED_SLICE_NON_IDR)      
    	return true;
		return false;  	
}

int RgbRtsp26x::find_nal_unit(uint8_t* buf, int size, int* nal_start, int* nal_end, uint8_t* nal_type)
{
	int i;
	// find start
	*nal_start = 0;
	*nal_end = 0;

	i = 0;
	while (   //( next_bits( 24 ) != 0x000001 && next_bits( 32 ) != 0x00000001 )
		(buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01) &&
		(buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0 || buf[i + 3] != 0x01)
		)
	{
		i++; // skip leading zero
		if (i + 4 >= size) { return 0; } // did not find nal start
	}

	if (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01) // ( next_bits( 24 ) != 0x000001 )
	{
		i++;
	}

	if (buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01) { /* error, should never happen */ return 0; }
	i += 3;
	*nal_start = i;
	
	if (m_h265)
		*nal_type= (buf[i] & 0x7E)>>1;
	else 
		*nal_type = buf[i] & 0x1f;
	
	while (   //( next_bits( 24 ) != 0x000000 && next_bits( 24 ) != 0x000001 )
		(buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0) &&
		(buf[i] != 0 || buf[i + 1] != 0 || buf[i + 2] != 0x01)
		)
	{
		i++;
		// FIXME the next line fails when reading a nal that ends exactly at the end of the data
		if (i + 3 >= size)
		{
			*nal_end = size;
			//	return -1;
			return (*nal_end - *nal_start);
		} // did not find nal end, stream ended first
	}

	*nal_end = i;
	return (*nal_end - *nal_start);
}

static SingleFrame shframe;
void * RgbRtsp26x::threadRgbFunc(void *)
{	 
	printf("........Enter  uvc  thread ,then read uvc......\n");	
	while(m_running)
	{		
		 if (!m_recvMedia) 
		 {
		 		os_sleep(50);
		 		continue;
		 }
		 			
		if (shr_read(m_h264Mem,&shframe,sizeof(SingleFrame))==0)					
		{
		//	PTRACE("shr_read h26x size=%d success!",shframe.length);
			if (shframe.length<sizeof(shframe.frame))
			{		
#if  1				
					ParseEncoderData((uint8_t*)shframe.frame,shframe.length);				
#else 								
	    	  FrameInfo frame;
	    	  frame.frame_type=0;
	    	  frame.frame_len=shframe.length;
	    	  frame.pframe=new uint8_t[shframe.length]; 
	    	  memcpy(frame.pframe,shframe.frame,shframe.length);
					if (sys_send_message(m_msgid,&frame,sizeof(FrameInfo))==0) 
					{   								
							if (msgQNumMsgs(m_msgid)>=MAX_QUEUES)
							{	
								usleep(500*1000);
								PTRACE("sys queue full!sleep 500ms...");
							}
							else if (msgQNumMsgs(m_msgid)>=(MAX_QUEUES/2))
							{	
								usleep(200*1000);
								PTRACE("sys queue half full!sleep 200ms...");
							}   												  															
					}	
					else 			
						PTRACE("send queue %d error!",msgQNumMsgs(m_msgid));			
						
				  PTRACE("send video len %d!",frame.frame_len);	
#endif						
				  m_ntick++;										
			}		
		}	
		else
		{
			//PTRACE("read h264 mem queue empty!");
			os_sleep(30);	
		}
	}	
	return NULL;
}	

void RgbRtsp26x::ParseEncoderData(uint8_t* p,uint32_t size)
{
	//while (m_running)
//	static FILE* file=fopen("/mnt/udisk/test.264","w");
	if (msgQNumMsgs(m_msgid)>=MAX_QUEUES)
	{	
		//usleep(500*1000);
		PTRACE("sys queue full!sleep 500ms...");
		return ;
	}
	else if (msgQNumMsgs(m_msgid)>=(MAX_QUEUES/2))
	{	
		//usleep(200*1000);
		PTRACE("sys queue half full!need sleep 200ms...");
	}   	   		
	if (m_recvMedia)
	{
		//	printf("read xlink data size =%d \n",packet->length);			 				
			int offset=(m_h265)?7:6;
		 // uint8_t *p = packet->data+offset;
     // uint32_t size=packet->length-offset;
			while (size > 0)
			{
					int nal_start = 0, nal_end = 0, nal_size = 0;
					uint8_t nal_type = 0;
					nal_size = find_nal_unit(p, size, &nal_start, &nal_end, &nal_type);
					if (nal_size == 0)
					{
						printf("no find nal head continue!\n");
						size = 0;
						break;
					}
				//	printf("get nal size=%d  nal_type:%d data:%x-%x\n", nal_size,nal_type,p[nal_start],p[nal_start+1]);
	       	bool bneed=false;
	       	
	        if (m_h265&&check265Send(nal_type))
	        	bneed = true;
	        else if (check264Send(nal_type))
	        	bneed = true;
	 
	        if (bneed)	    
          {                   	  
          	  uint8_t* buffer=new uint8_t[nal_size+NAL_START_CODE_SIZE];
          	  memcpy(buffer,start_code,NAL_START_CODE_SIZE);
          	  memcpy(buffer+NAL_START_CODE_SIZE,&p[nal_start],nal_size);
          	  
          	  FrameInfo frame;
          	  frame.frame_type=nal_type;
          	  frame.frame_len=nal_size+NAL_START_CODE_SIZE;
          	  frame.pframe=buffer; 
							if (sys_send_message(m_msgid,&frame,sizeof(FrameInfo))==0) 
							{  
								if  (!m_h265)
									PTRACE("recv h264 data and send video type=%s size=%u ok!",frame264_des[nal_type],nal_size+4); 
						   else		
								  PTRACE("recv h265 data and send video type=%d size=%u ok!", nal_type,nal_size+4);  
							//  fwrite(frame.pframe,1,frame.frame_len,file);
							//	fflush(file);																				  															
							}    
         	} 
				 	size -= nal_end;
				 	p += nal_end; 			
			}   	
      m_isInitMeta = false;					
		//	int len=writen(pipe_fd,packet->data+7,   packet->length-7);						     
	}//end if	
}

static int send_main_cmd(int command,char* data, int len, int timeout)  //second
{
	KERNEL_MESSAGE msg;
	msg.command= command;
	msg.receiver=CORE_APP_MOD;
	msg.sender= RTSP_APP_MOD;
	memcpy(msg.data,data,len);
	msg.length = len;
	int ret = send_recv_timeout((char*)&msg, sizeof(MESSAG_HEADER) + msg.length,MAIN_CTRL_PORT,
														(char*)&msg,sizeof(msg),timeout);
														
	if (ret<0)
		return 0;
  else 
  	return msg.data[0];														
}

static inline double difftimeval(const struct timeval *start, const struct timeval *end)
{
        double d;
        time_t s;
        suseconds_t u;

        s = start->tv_sec - end->tv_sec;
        u = start->tv_usec - end->tv_usec;
        //if (u < 0)
        //        --s;

        d = s;
        d *= 1000000.0;
        d += u;
				d   /=1000.0; //return ms
        return d;
}

//recv command from rtsp command  
void * RgbRtsp26x::threadRecvFunc(void *)
{
	int ret;
	PTRACE("........Enter  recv cmd thread ,then listen cmd......\n");	
	struct  timeval  tv_s,tv_e;
	while(m_running)
	{
		char cmd[100]={0};
		ret = recv_udp_message(cmd,sizeof(cmd),CMD_PORT_RGB);
		PTRACE("recv request %s ok! codec status:%d",cmd,m_recvMedia);
		if ((strstr(cmd,OK_CMD)!=NULL)&&(!m_recvMedia))
	//	if ((strstr(cmd,OK_CMD)!=NULL))
		{	
			PTRACE("Encoder init success!");	
			cmd[0]=1;
			send_main_cmd(APP_VIDEO_CONTROL,cmd,1,3);					
			m_recvMedia=true;		
			m_ntick=0;
			gettimeofday(&tv_s,NULL); 
		}
		else if ((strstr(cmd,EXIT_CMD)!=NULL)&&(m_recvMedia))
		{
				gettimeofday(&tv_e,NULL);
				PTRACE("exit video");
			//  if (m_ntick<100)  continue;
				int delay= difftimeval(&tv_e,&tv_s);
				if (delay<3*1000) continue;
				m_recvMedia=false;
				cmd[0]=0;
				send_main_cmd(APP_VIDEO_CONTROL,cmd,1,3);		
		}
	}
	return NULL;
}

bool  RgbRtsp26x::Close()
{
		m_running=false;
		if (vid_rth!=NULL)
		{	
			pthread_join(vid_rth,NULL);
			vid_rth=NULL;
		}
		if (vid_rgb!=NULL)
		{
			pthread_join(vid_rgb,NULL);
			vid_rgb=NULL;
		}
	
		if (m_h264Mem>0)
		{
		  shr_close(m_h264Mem);
		  m_h264Mem=0;
	  }
	  
		return true;
}

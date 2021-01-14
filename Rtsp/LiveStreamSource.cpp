#include<fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "LiveStreamSource.h"
#include "GroupsockHelper.hh"
//#include "includes.h"
#include "param.h"
//#define FIFO_NAME "/tmp/fifo"

LiveStreamSource* LiveStreamSource::createNew(UsageEnvironment& env, 
				MSG_Q_ID msgid,std::string streamid,unsigned playTimePerFrame) 
{
	LiveStreamSource* newSource = new LiveStreamSource(env,msgid,streamid, playTimePerFrame);
	
	return newSource;
}

LiveStreamSource::LiveStreamSource(UsageEnvironment& env,
					   MSG_Q_ID msgid,std::string streamid,unsigned playTimePerFrame)
  : FramedSource(env),fHaveStartedReading(False),fNeedIFrame(True)
{
	//pipefd = open(FIFO_NAME,O_RDONLY);	
	m_msgid = msgid;
	m_streamid = streamid;
//	PTRACE("get streamid %s",m_streamid.c_str());
}

LiveStreamSource::~LiveStreamSource() 
{
//	PTRACE("~LiveStreamSource stop video");	
	char cmd[100];			
	int port=0;
	if (m_streamid.compare(IR_STREAM)==0)
		port = CMD_PORT;			
	else  	if (m_streamid.compare(RGB_STREAM)==0)
		port = CMD_PORT_RGB;		
		
	if 	(port>0)
	{
		sprintf(cmd,"%s-%s",EXIT_CMD,m_streamid.c_str());		
		int ret=send_udp_message(cmd,strlen(cmd),port);
		PTRACE("send cmd %s ret=%d",cmd,ret);
		fHaveStartedReading = true;
	}		
}

static int readn(int fd, uint8_t* buffer, int len)
{
	int readleft = len;
	int index = 0;
	int ret=0;
	while (readleft > 0)
	{
		ret = read(fd, buffer+index, readleft);
		if (ret <= 0)
		{
			if (errno == EAGAIN )
				continue;
			else
			{
				index=-1;
				break;
			}
		}
		readleft -= ret;	
		index += ret;	
	}
	return index;
}

//FILE* testfile=NULL;
void LiveStreamSource::doGetNextFrame() 
{
		FrameInfo frame;
		if (!fHaveStartedReading)
		{
			char cmd[100];			
			int port=0;
			if (m_streamid.compare(IR_STREAM)==0)
				port = CMD_PORT;			
			else  	if (m_streamid.compare(RGB_STREAM)==0)
				port = CMD_PORT_RGB;		
				
			if 	(port>0)
			{
				sprintf(cmd,"%s-%s",OK_CMD,m_streamid.c_str());		
				int ret=send_udp_message(cmd,strlen(cmd),port);
				PTRACE("send cmd=%s port=%d ret=%d",cmd,port,ret);
				fHaveStartedReading = true;
			}
	//		testfile=fopen("/tmp/live.265","w");
		}
				
    if (sys_receive_message(m_msgid,&frame,sizeof(FrameInfo))<0)
    {
    	PTRACE("recv live frame error!");
    	return;
    }
    u_int8_t* newFrameDataStart = (u_int8_t*)frame.pframe;
  	unsigned newFrameSize = frame.frame_len; 
	  if(newFrameSize > fMaxSize) 
	  {
	    fFrameSize = fMaxSize;
	    fNumTruncatedBytes = newFrameSize - fMaxSize;
	    PTRACE("new frame size =%u ,but need %u \n",newFrameSize,fFrameSize);
	  }
	  else
	  {
	    fFrameSize = newFrameSize;
	    fNumTruncatedBytes=0;
	  }  	
	  PTRACE("send rtsp  fame type=%d size=%u and consume frame size=%u \n",frame.frame_type,newFrameSize,fFrameSize);
		
	//  fwrite(newFrameDataStart,1,newFrameSize,testfile);
	//  fflush(testfile);
	//	memmove(fTo, newFrameDataStart, fFrameSize); 
		memcpy(fTo, newFrameDataStart, fFrameSize); 
		gettimeofday(&fPresentationTime, NULL); 
		delete frame.pframe;
	//	fDurationInMicroseconds = 1000000/25;
		fDurationInMicroseconds = 0;
 // 	nextTask() = envir().taskScheduler().scheduleDelayedTask(0,(TaskFunc*)FramedSource::afterGetting, this);
	 FramedSource::afterGetting(this);
}

void LiveStreamSource::doStopGettingFrames() 
{
	//fHaveStartedReading = False;
	PTRACE("doStopGettingFrames....");
//	send_udp_message(EXIT_CMD,strlen(EXIT_CMD),CMD_PORT);
}

unsigned LiveStreamSource::maxFrameSize() const {
  return 512*1024;
}

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "H26xVideoLiveSMS.h"
//#include "PlgRtsp26x.h"
#include "sysinfo.h"
#include "param.h"
#include "RgbRtsp26x.h"
#include "inifile.h"

//#define FIFO_FILE "/tmp/fifo" 

UsageEnvironment* env;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = True;//False;

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
			   char const* streamName)
{
  char* url = rtspServer->rtspURL(sms);
  UsageEnvironment& env = rtspServer->envir();

  env << "\n"<<"Play this stream using the URL \"" << url << "\"\n";
  delete[] url;
}

#if 0
void test_recv(MSG_Q_ID msg)
{
		while(getchar()!='t');
		printf("........start test recv 0x%x........\n",msg);
		int ret=send_udp_message(OK_CMD,strlen(OK_CMD),CMD_PORT);
		PTRACE("send cmd %s ret=%d",OK_CMD,ret);
		FILE* testfile=fopen("/tmp/live.265","w");
		while (1)
		{
				FrameInfo frame;					
		    if (sys_receive_message(msg,&frame,sizeof(FrameInfo))<0)
		    {
		    	PTRACE("recv live frame error!");
		    	break ;
		    }
		    u_int8_t* newFrameDataStart = (u_int8_t*)frame.pframe;
		  	unsigned newFrameSize = frame.frame_len; 
	
			  PTRACE("recv  fame type=%d size=%u \n",frame.frame_type,newFrameSize);
				
			  fwrite(newFrameDataStart,1,newFrameSize,testfile);
			  fflush(testfile);
		}	
		fclose(testfile);
}
#endif

int main(int argc, char** argv) 
{
  	int is265=265;
  	is265=read_profile_int("ENC_PARAM","enc_type",265,"config.ini");	
  	PTRACE("get enc type=%d",is265);
    // Begin by setting up our usage environment:
    OutPacketBuffer::maxSize = 600000;
    TaskScheduler* scheduler = BasicTaskScheduler::createNew();
    env = BasicUsageEnvironment::createNew(*scheduler);
    UserAuthenticationDatabase* authDB = NULL;  	
#ifdef ACCESS_CONTROL
    // To implement client access control to the RTSP server, do the following:
    authDB = new UserAuthenticationDatabase;
    authDB->addUserRecord("username1", "password1"); // replace these with real strings
    // Repeat the above with each <username>, <password> that you wish to allow
    // access to the server.
#endif

#if  0
		unlink( FIFO_FILE );
	  if (mkfifo(FIFO_FILE, 0666)<0)
	  {
	  	perror("mkfifo err!\n");
	  	return -1;
	  }
#endif
#if 0
	PlgRtsp26x plg26x((is265==265)?true:false);
	MSG_Q_ID msgIR=msgQCreate(MAX_QUEUES,sizeof(FrameInfo));
	if (msgIR==NULL)
	{
			printf("create mseg error!\n");
			//msgQDelete(msg);
			return -1;
	}
	if (!plg26x.Create(msgIR))
	{
		printf("create  plg26x  error!\n");
		msgQDelete(msgIR);
		return -1;
	}
	else
		printf("open plg26x success!0x%x\n",msgIR);
#endif
			
	RgbRtsp26x rgb26x((is265==265)?true:false);
	MSG_Q_ID msgRgb=msgQCreate(MAX_QUEUES,sizeof(FrameInfo));
	if (msgRgb==NULL)
	{
			printf("create mseg error!\n");
			//msgQDelete(msg);
			return -1;
	}
	if (!rgb26x.Create(msgRgb))
	{
		printf("create  plg26x  error!\n");
		msgQDelete(msgRgb);
		return -1;
	}
	else
		printf("open plg26x success!0x%x\n",msgRgb);		
			
		//while (getchar()!='q');
		
  // Create the RTSP server:
    RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554, authDB);
    if (rtspServer == NULL) 
    {
        *env << "Failed to create svt RTSP server: " << env->getResultMsg() << "\n";
        exit(1);
    }  
  	char const* descriptionString = "Session streamed by \"EyeCloud.INC\"";

  // Set up each of the possible streams that can be served by the
  // RTSP server.  Each such stream is implemented using a
  // "ServerMediaSession" object, plus one or more
  // "ServerMediaSubsession" objects for each audio/video substream.

   //  char const* streamName = "liveIR";

  // A H.264 video elementary stream:
#if  0  //remove IR
    ServerMediaSession* sms = ServerMediaSession::createNew(*env, IR_STREAM, IR_STREAM, descriptionString,0);     
    	      
    sms->addSubsession(H26xVideoLiveServerMediaSubsession::createNew(*env,msgIR,IR_STREAM,reuseFirstSource,is265));//replace From H264VideoFileServerMediaSubsession			     

    rtspServer->addServerMediaSession(sms);
    announceStream(rtspServer, sms, IR_STREAM);
#endif 
		//duke add rgb live 2020.5.26
	//	char const* streamNameRgb = "live";
    ServerMediaSession* smsRgb = ServerMediaSession::createNew(*env, RGB_STREAM, RGB_STREAM, descriptionString,0);     
    	      
    smsRgb->addSubsession(H26xVideoLiveServerMediaSubsession::createNew(*env,msgRgb,RGB_STREAM,reuseFirstSource,is265));//replace From H264VideoFileServerMediaSubsession			     

    rtspServer->addServerMediaSession(smsRgb);
    announceStream(rtspServer, smsRgb, RGB_STREAM);
    
  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    *env << "\n(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling.)\n";
  } else {
    *env << "\n(RTSP-over-HTTP tunneling is not available.)\n";
  }

  env->taskScheduler().doEventLoop(); // does not return
//  msgQDelete(msgIR);
  msgQDelete(msgRgb);
  printf("Exit server.....!\n");

  return 0; // only to prevent compiler warning
}

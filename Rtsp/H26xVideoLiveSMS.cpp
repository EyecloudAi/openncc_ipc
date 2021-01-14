#include "H26xVideoLiveSMS.h"
#include "H264VideoRTPSink.hh"
#include "H265VideoRTPSink.hh"
#include "H265VideoStreamFramer.hh"
#include "H264VideoStreamFramer.hh"
#include "LiveStreamSource.h"
#include "H264or5VideoStreamFramer.hh"
#include "H264or5VideoRTPSink.hh"

H26xVideoLiveServerMediaSubsession*
H26xVideoLiveServerMediaSubsession::createNew(UsageEnvironment& env,
					      MSG_Q_ID streamNum,std::string streamid,
					      Boolean reuseFirstSource,int media_type) 
{
	return new H26xVideoLiveServerMediaSubsession(env, streamNum, streamid,reuseFirstSource,media_type);
}

H26xVideoLiveServerMediaSubsession::H26xVideoLiveServerMediaSubsession(UsageEnvironment& env,
								       MSG_Q_ID streamNum, std::string streamid,Boolean reuseFirstSource,int media_type)
  : OnDemandServerMediaSubsession(env, reuseFirstSource),
    fAuxSDPLine(NULL), fDoneFlag(0), fDummyRTPSink(NULL),fStreamNum(streamNum),fstreamid(streamid),fmedia_type(media_type)
{
	
}

H26xVideoLiveServerMediaSubsession::~H26xVideoLiveServerMediaSubsession() 
{
	delete[] fAuxSDPLine;
}

static void afterPlayingDummy(void* clientData) 
{
	H26xVideoLiveServerMediaSubsession* subsess = (H26xVideoLiveServerMediaSubsession*)clientData;
	subsess->afterPlayingDummy1();
} 

void H26xVideoLiveServerMediaSubsession::afterPlayingDummy1()
{
	// Unschedule any pending 'checking' task:
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	// Signal the event loop that we're done:
	setDoneFlag();
} 

static void checkForAuxSDPLine(void* clientData) 
{
	H26xVideoLiveServerMediaSubsession* subsess = (H26xVideoLiveServerMediaSubsession*)clientData;
	subsess->checkForAuxSDPLine1();
}

void H26xVideoLiveServerMediaSubsession::checkForAuxSDPLine1() 
{
	nextTask() = NULL;

	char const* dasl;
	if (fAuxSDPLine != NULL) 
	{
		// Signal the event loop that we're done:
		setDoneFlag();
	} 
	else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) 
	{
		fAuxSDPLine = strDup(dasl);
		fDummyRTPSink = NULL;

		// Signal the event loop that we're done:
		setDoneFlag();
	} else if (!fDoneFlag) 
	{
		// try again after a brief delay:
		int uSecsToDelay = 100000; // 100 ms
		nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
		(TaskFunc*)checkForAuxSDPLine, this);
	}
}

char const* H26xVideoLiveServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource) 
{
	if (fAuxSDPLine != NULL) 
		return fAuxSDPLine; // it's already been set up (for a previous client)

	if (fDummyRTPSink == NULL) 
	{ 
		// we're not already setting it up for another, concurrent stream
		// Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
		// until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
		// and we need to start reading data from our file until this changes.
		fDummyRTPSink = rtpSink;

		// Start reading the file:
		fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);

		// Check whether the sink's 'auxSDPLine()' is ready:
		checkForAuxSDPLine(this);
	}

	envir().taskScheduler().doEventLoop(&fDoneFlag);

	return fAuxSDPLine;
}

FramedSource* H26xVideoLiveServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) 
{
	estBitrate = 10*1000; // kbps, estimate

	// Create the video source:
	//printf("stream %s \n",fstreamid.c_str());
	LiveStreamSource* liveSource = LiveStreamSource::createNew(envir(),fStreamNum,fstreamid);
	if (liveSource == NULL) return NULL;

	// Create a framer for the Video Elementary Stream:
	if (fmedia_type==264)
		return H264VideoStreamFramer::createNew(envir(), liveSource);
	else		
		return H265VideoStreamFramer::createNew(envir(), liveSource);
}

RTPSink* H26xVideoLiveServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
		   unsigned char rtpPayloadTypeIfDynamic,
		   FramedSource* /*inputSource*/) 
{
	if (fmedia_type==264)
		return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
	else		
		return H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

#ifndef _H264_VIDEO_LIVE_SERVER_MEDIA_SUBSESSION_H
#define _H264_VIDEO_LIVE_SERVER_MEDIA_SUBSESSION_H

#ifndef _SERVER_MEDIA_SESSION_HH
#include "ServerMediaSession.hh"
#endif
#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif
#ifndef _FILE_SERVER_MEDIA_SUBSESSION_HH
#include "FileServerMediaSubsession.hh"
#endif

#include "sysinfo.h"
#include <string>
class H26xVideoLiveServerMediaSubsession: public OnDemandServerMediaSubsession 
{
public:
	static H26xVideoLiveServerMediaSubsession*
	createNew(UsageEnvironment& env, MSG_Q_ID streamNum,std::string streamid, Boolean reuseFirstSource,int media_type=265);

	// Used to implement "getAuxSDPLine()":
	void checkForAuxSDPLine1();
	void afterPlayingDummy1();

protected:
	H26xVideoLiveServerMediaSubsession(UsageEnvironment& env,
	MSG_Q_ID streamNum, std::string streamid,Boolean reuseFirstSource,int media_type=265);
	// called only by createNew();
	virtual ~H26xVideoLiveServerMediaSubsession();

	void setDoneFlag() { fDoneFlag = ~0; }

protected: // redefined virtual functions
	virtual char const* getAuxSDPLine(RTPSink* rtpSink,
	FramedSource* inputSource);
	virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
	unsigned& estBitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
	unsigned char rtpPayloadTypeIfDynamic,
	FramedSource* inputSource);

private:
	char* fAuxSDPLine;
	char fDoneFlag; // used when setting up "fAuxSDPLine"
	RTPSink* fDummyRTPSink; // ditto
	MSG_Q_ID fStreamNum;
	int fmedia_type;
	std::string fstreamid;
};

#endif

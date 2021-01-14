#ifndef _LIVE_STREAM_SOURCE_H
#define _LIVE_STREAM_SOURCE_H

#ifndef _FRAMED_SOURCE_HH
#include "FramedSource.hh"
#endif

#include "sysinfo.h"
#include <string>

//#define ipc_rtsp_log(level, args...) ipc_log_print(level, __FILE__, __FUNCTION__, __LINE__, ##args)

class LiveStreamSource: public FramedSource 
{
public:
  static LiveStreamSource* createNew(UsageEnvironment& env,
					 MSG_Q_ID msgid ,std::string streamid,
					 unsigned playTimePerFrame = 0);
  // "preferredFrameSize" == 0 means 'no preference'
  // "playTimePerFrame" is in microseconds

protected:
  LiveStreamSource(UsageEnvironment& env,
		       MSG_Q_ID msgid,std::string streamid,
		       unsigned playTimePerFrame);
	// called only by createNew()

  virtual ~LiveStreamSource();

private:
  // redefined virtual functions:
  virtual void doGetNextFrame();
  virtual void doStopGettingFrames();
	virtual unsigned maxFrameSize() const;
private:
  Boolean fHaveStartedReading;
  Boolean fNeedIFrame;

 // int pipefd;
  MSG_Q_ID  m_msgid;
  std::string m_streamid;
};

#endif



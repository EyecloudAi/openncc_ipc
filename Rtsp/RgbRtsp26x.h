
#ifndef __RGB_RTSP264_H__
#define __RGB_RTSP264_H__

#ifndef PLGXLINK_MAX_CHAN_NAME
#define PLGXLINK_MAX_CHAN_NAME  (50)
#endif
#include "includes.h"
#include "sysinfo.h"
#include "shm.h"
#include "param.h"

class RgbRtsp26x
{
public:
	RgbRtsp26x(bool h265=false);
	~RgbRtsp26x();
    
	bool   Create(MSG_Q_ID msgid,bool self_thrd=true);
	void  *threadRecvFunc(void * );
	void  *threadRgbFunc(void * );
	bool   Close();
	void ParseEncoderData(uint8_t* pdata,uint32_t size);
	//void HandleRgbData(uint8_t* pdata);
private: 	
	 SHARE_MEM_HANDLE m_h264Mem;
	 MSG_Q_ID m_msgid;
	// uint8_t* h264_data;
	// int frame_size;
	 pthread_t vid_rth;
	 pthread_t vid_rgb;

	 bool m_isInitMeta;
	 bool m_running;
	 bool m_h265;
	 bool m_recvMedia;
	 unsigned int m_ntick;
	// int tick_gap;
  //  uint8_t extractNalUnit(uint8_t* buffer, uint32_t start, uint32_t buffer_len, uint32_t* startNal, uint32_t* endNal);
		bool check265Send(uint8_t nal_unit_type);
		bool check264Send(uint8_t nal_unit_type);
		int find_nal_unit(uint8_t* buf, int size, int* nal_start, int* nal_end, uint8_t* nal_type);
};

#endif

#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ipc_pool.h"
using namespace std;
class CPoolMan
{
public:
	~CPoolMan();
	static CPoolMan & GetInstance(unsigned char u8No);
	bool Init();
	unsigned int GetNewestSpsPos();
	bool WriteData(LPPOOL_HEADER_S pHdr,unsigned char *pData);
	unsigned int ReadData(LPPOOL_HEADER_S pHdr,unsigned char *pData,unsigned int posOffset);

private:
	CPoolMan();
		
	unsigned char *pHdl;
	STREAM_TYPE_E eStreamType;
	unsigned int u32BuffSize;
	unsigned int u32WritePos;
	unsigned int u32FrameNo;
	unsigned int u32NewestSPSPos;
	unsigned int u32NewestIFramePos;
	unsigned int u32NewestFramePos;

	pthread_rwlock_t	rwLock;
};


CPoolMan::CPoolMan()
{
	u32WritePos = 0;
	u32FrameNo = 0;
	u32NewestIFramePos = 0;
	u32BuffSize = 0;
	pthread_rwlock_init(&rwLock, NULL);
}

CPoolMan::~CPoolMan()
{	
	if(pHdl)
	{
		delete[] pHdl;
		pHdl = NULL;
	}
}

CPoolMan & CPoolMan::GetInstance(unsigned char streamType)
{
	if(streamType > POOL_MAX_NUM)
		streamType = POOL_MAX_NUM - 1;
	
	static CPoolMan pool[POOL_MAX_NUM];
	pool[streamType].eStreamType = static_cast<STREAM_TYPE_E>(streamType);

	return pool[streamType];
}


bool CPoolMan::Init()
{
	if(STREAM_TYPE_MAIN == eStreamType)
	{
		pHdl = new(nothrow) unsigned char[POOL_MAIN_STREAM_SIZE];
		if(!pHdl)
		{
			cout << "new main stream pool space failed !" << endl;
			return false;
		}
		u32BuffSize = POOL_MAIN_STREAM_SIZE;

		printf("streamType:%d,hdl:%p\n",eStreamType,pHdl);
	}
	else if(STREAM_TYPE_SUB == eStreamType)
	{
		pHdl = new(nothrow) unsigned char[POOL_SUB_STREAM_SIZE];
		if(!pHdl)
		{
			cout << "new sub stream pool space failed !" << endl;
			return false;
		}
		u32BuffSize = POOL_SUB_STREAM_SIZE;
		
		printf("streamType:%d,hdl:%p\n",eStreamType,pHdl);
	}
	else if(STREAM_TYPE_THIRD== eStreamType)
	{
		pHdl = new(nothrow) unsigned char[POOL_THIRD_STREAM_SIZE];
		if(!pHdl)
		{
			cout << "new third stream pool space failed !" << endl;
			return false;
		}
		u32BuffSize = POOL_THIRD_STREAM_SIZE;
		
		printf("streamType:%d,hdl:%p\n",eStreamType,pHdl);
	}
	else if(STREAM_TYPE_AUDIO == eStreamType)
	{
		pHdl = new(nothrow) unsigned char[POOL_AUDIO_SIZE];
		if(!pHdl)
		{
			cout << "new audio stream pool space failed !" << endl;
			return false;
		}
		u32BuffSize = POOL_AUDIO_SIZE;
		
		printf("streamType:%d,hdl:%p\n",eStreamType,pHdl);
	}

	return true;
}


unsigned int CPoolMan::GetNewestSpsPos()
{
	unsigned int u32Pos = 0;

	pthread_rwlock_rdlock(&rwLock);
	u32Pos = u32NewestSPSPos;	
	pthread_rwlock_unlock(&rwLock);
	return u32Pos;
}

bool CPoolMan::WriteData(LPPOOL_HEADER_S pHdr,unsigned char *pData)
{
	unsigned int u32NeedSize = 0;
	unsigned int leftBytes = 0;
	
	u32NeedSize = (pHdr->u32Len & 0XFFFFFFC0) + ALIGN_BYTES + HDR_SIZE; //64�ֽڶ���
	pthread_rwlock_wrlock(&rwLock);

	leftBytes = u32BuffSize - u32WritePos - 1;
	//ʣ���ֽ��㹻�洢֡ͷ,�����㹻�洢��������
	if((leftBytes >= HDR_SIZE) && (u32WritePos + u32NeedSize > u32BuffSize)) //�ֶο���
	{
		if(1 == pHdr->u8IsSPS)
			u32NewestSPSPos = u32WritePos;

		pHdr->u32FrameNo = u32FrameNo++;
		memmove(pHdl + u32WritePos,pHdr,HDR_SIZE);
		u32WritePos += HDR_SIZE;

		u32NeedSize = (pHdr->u32Len & 0XFFFFFFC0) + ALIGN_BYTES;
		leftBytes = u32BuffSize - u32WritePos - 1;
		memmove(pHdl + u32WritePos,pData,leftBytes);
		memmove(pHdl,pData + leftBytes,u32NeedSize - leftBytes); //����
		
		u32WritePos = u32NeedSize - leftBytes - 1;
	}
	//ʣ���ֽڲ��㹻�洢֡ͷ,����������
	else if(leftBytes < HDR_SIZE)
	{	
		u32WritePos = 0;
		if(1 == pHdr->u8IsSPS)
			u32NewestSPSPos = u32WritePos;
		
		pHdr->u32FrameNo = u32FrameNo++;
		memmove(pHdl,pHdr,HDR_SIZE);
		u32WritePos += HDR_SIZE;

		u32NeedSize = (pHdr->u32Len & 0XFFFFFFC0) + ALIGN_BYTES;
		memmove(pHdl + u32WritePos,pData,pHdr->u32Len);
		u32WritePos += u32NeedSize;
	}
	//ʣ���ֽ��㹻�洢֡ͷ+����
	else
	{
		if(1 == pHdr->u8IsSPS)
			u32NewestSPSPos = u32WritePos;

		u32NewestFramePos = u32WritePos;
		pHdr->u32FrameNo = u32FrameNo++;
		memmove(pHdl + u32WritePos,pHdr,HDR_SIZE);
		
		u32WritePos += HDR_SIZE;
		u32NeedSize = (pHdr->u32Len & 0XFFFFFFC0) + ALIGN_BYTES;
		memmove(pHdl + u32WritePos,pData,pHdr->u32Len);
		u32WritePos += u32NeedSize;
	}
	pthread_rwlock_unlock(&rwLock);
	return true;
}

//posOffset ��ʼֵ = CPoolMan::GetInstance(0).GetNewestIFramePos();
//ÿ��ȡһ֡����, offset ���ƫ��һ֡���ݳ���
unsigned int CPoolMan::ReadData(LPPOOL_HEADER_S pHdr,unsigned char *pData,unsigned int posOffset)
{
	unsigned int u32NeedSize = 0;
	unsigned int leftBytes = 0;
	unsigned int u32NextPos = 0;

	pthread_rwlock_rdlock(&rwLock); 
	
	memmove(pHdr,pHdl + posOffset,HDR_SIZE);
	
	//��д֡�Ŵ���30֡,�����¶�ȡ���¹ؼ�֡,�����д���ʲ�ͬ���µ�׷֡����
	if((abs(pHdr->u32FrameNo - u32FrameNo) > 30) && (1 == pHdr->u8IsSPS))
	{
		memset(pHdr,0,HDR_SIZE);
		u32NextPos = u32NewestSPSPos;
		pthread_rwlock_unlock(&rwLock);
		return u32NextPos;
	}

	//��ȡ����֡���߶�ȡ������֡
	if((pHdr->u32FrameNo == (u32FrameNo - 1)) || (0 == pHdr->u32Len) || 
	(HDR_START_FLAG != pHdr->u32HdrStartFlag) || (HDR_END_FLAG != pHdr->u32HdrEndFlag)) 
	{
		memset(pHdr,0,HDR_SIZE);
		u32NextPos = u32NewestSPSPos;
		pthread_rwlock_unlock(&rwLock);
		return u32NextPos;
	}

	u32NeedSize = (pHdr->u32Len & 0XFFFFFFC0) + ALIGN_BYTES;
	if(posOffset + HDR_SIZE + u32NeedSize > u32BuffSize - 1) //�ֶο���
	{
		leftBytes = u32BuffSize - posOffset - HDR_SIZE - 1;
		memmove(pData,pHdl + posOffset + HDR_SIZE,leftBytes);
		memmove(pData + leftBytes,pHdl,u32NeedSize - leftBytes);
		u32NextPos = u32NeedSize - leftBytes - 1;
	}
	else
	{
		memmove(pData,pHdl + posOffset + HDR_SIZE,u32NeedSize);
		u32NextPos = (posOffset + HDR_SIZE + u32NeedSize);
	}

	if(u32NextPos + HDR_SIZE > u32BuffSize - 1)
		u32NextPos = 0;

	pthread_rwlock_unlock(&rwLock);
	return u32NextPos;
}

extern "C"
{
int ipc_ring_buff_init()
{
	CPoolMan::GetInstance(0).Init();
	return 0;
}

int ipc_write_frame(LPPOOL_HEADER_S pHdr,unsigned char *pData)
{
	return static_cast<int>(CPoolMan::GetInstance(0).WriteData(pHdr,pData));
}

int ipc_read_frame(LPPOOL_HEADER_S pHdr,unsigned char *pData,unsigned int offset)
{
	return CPoolMan::GetInstance(0).ReadData(pHdr,pData,offset);
}

unsigned int ipc_get_newest_sps_offset()
{
	 return CPoolMan::GetInstance(0).GetNewestSpsPos();
}
}

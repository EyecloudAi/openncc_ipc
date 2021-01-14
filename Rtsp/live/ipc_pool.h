#ifndef IPC_POOL_H_
#define IPC_POOL_H_

#include <pthread.h>

#define POOL_MAX_NUM		4	//3 + 1
#define HDR_SIZE			64	
#define ALIGN_BYTES			64	//数据按照64字节对齐

#define POOL_MAIN_STREAM_SIZE	4194304//8388608
#define POOL_SUB_STREAM_SIZE	2097152
#define POOL_THIRD_STREAM_SIZE	1048576
#define	POOL_AUDIO_SIZE			131072

#define	HDR_START_FLAG	0X12345678
#define HDR_END_FLAG	0X87654321


typedef enum STREAM_TYPE_E_
{
	STREAM_TYPE_MAIN = 0,
	STREAM_TYPE_SUB,
	STREAM_TYPE_THIRD,
	STREAM_TYPE_AUDIO,
	STREAM_TYPE_BUTT,
}STREAM_TYPE_E;

typedef struct _POOL_HEADER_S_
{
	unsigned int u32HdrStartFlag;
	STREAM_TYPE_E eStreamType;
	unsigned int u32Width;
	unsigned int u32Height;
	unsigned int u32Pts;
	unsigned int u32Len;
	unsigned char u8IsIFrame;
	unsigned char u8IsSPS;
	unsigned char u8IsPPS;
	unsigned int u32FrameNo;
	unsigned int u32HdrEndFlag;
	unsigned char u8Res[28];
}POOL_HEADER_S,*LPPOOL_HEADER_S;


#ifdef __cplusplus           
extern "C"{
#endif

int ipc_ring_buff_init();

int ipc_write_frame(LPPOOL_HEADER_S pHdr,unsigned char *pData);

int ipc_read_frame(LPPOOL_HEADER_S pHdr,unsigned char *pData,unsigned int offset);

unsigned int ipc_get_newest_sps_offset();



#ifdef __cplusplus
}
#endif
	

#endif


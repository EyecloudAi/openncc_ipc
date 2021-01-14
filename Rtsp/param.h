#ifndef __PLG_PARAM_H__
#define __PLG_PARAM_H__

#define  IR_STREAM   "liveIR" 
#define  RGB_STREAM  "liveRGB" 

#define OK_CMD     "givemevideo"
#define EXIT_CMD   "exitvideo"
#define CMD_PORT       9101
#define CMD_PORT_RGB   9102
#define MAX_QUEUES     20
typedef struct _FrameInfo
{
	int frame_type;
	int frame_len;
	uint8_t* pframe;
} FrameInfo;

#endif


#ifndef __COMMON_H__
#define __COMMON_H__

#include "../public/include/includes.h"
#include "../public/include/sysinfo.h"
#include "../public/include/message.h"

#define VERSION  "maiApp0.0.7"

#define  AI_BLOB_FILE "blob/eyecloud.blob"     //更新算法blob文件
#define  AI_BIN_FILE  "blob/eyecloud.bin"      //更新算法xml文件
#define  AI_XML_FILE  "blob/eyecloud.xml"      //更新算法xml文件

typedef struct _APP_CORE_CONTEXT
{
	int running;
}APP_CORE_CONTEXT;

extern APP_CORE_CONTEXT  * core_context;

void*  sys_ctrl_loop( void *arg);  
void*  remote_ctrl_loop( void *arg);
void*  ncc_loop( void *arg);
int  meta_relay(char* buf,int size);

int open_ncc(int capmode);
int close_ncc();
int get_ncc_status();
#endif


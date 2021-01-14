#ifndef __CLI_SDK_H__
#define __CLI_SDK_H__

#include <string>
#include <vector> 
using namespace std; 

#define OUTPUT_INDEX_SIZE   64 

typedef unsigned char BYTE ;
typedef unsigned long   CLI_HANDLE;

typedef struct
{
     int      type;
     unsigned int  seqNo;
     unsigned int  size;
     unsigned int  res[13];
}frameSpecOut;

enum CLI_STATUS
{
	ST_OK = 0,
	NO_READ_DATA = -1,
	NET_CLOSE = -2,
	RECV_ERROE = -3,
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void get_sdk_version( char* version);

//return 0: error,  >0: ok 
CLI_HANDLE ConnectToDevice(const char* device_ip,const char* filepath="./");

//all 0:ok -1:error
int CloseDevice(CLI_HANDLE handle);
int GetDeviceVer(CLI_HANDLE handle,char* version);
int GetAIMode(CLI_HANDLE handle,char* modename);

//action:0 restartmainApp  1£»restart system
int RebootByRemote(CLI_HANDLE handle,int action);

int StartUpgrade(CLI_HANDLE handle,const char* up_file);

int UpdateAiModel(CLI_HANDLE handle,const char* ai_blob,const char* ai_xml,const char* mode_name);
int QueryUpStep(CLI_HANDLE handle,int step);//step from 0~100

//return -1: no data  -2:tcp close
int ReadMetaData(CLI_HANDLE handle,BYTE* pbuf,int& size);

int ScanIPCAddr(int nsecond,vector<string>& rtsp);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif



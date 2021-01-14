#ifndef __ONVIF_CMD_H__
#define __ONVIF_CMD_H__

#include <stdio.h>  
#include <stdlib.h>  

typedef struct
{
	char username[64];
	char password[32];
}USERINFO, *LPUSERINFO;

typedef struct 
{
	char name[100];
	char token[100];
	char media_rtsp[100];
	int  encoder;
	int   width;
	int   height;	
} media_track;

typedef struct 
{
	USERINFO stUserInfo;
	struct SOAP_ENV__Header header;	
	struct soap *soap ;	
	
	char server_addr[100];
	char media_addr[100];
	
	media_track media_info[2];//main and sub 
}onvif_info;

 //int ONVIF_Initsoap(onvif_info* ov,  const char *was_To, const char *was_Action, int timeout) ;
int ONVIF_ClientDiscovery(onvif_info* ov );
int ONVIF_Capabilities(onvif_info* ov,const char* deviceURI);
int ONVIF_Profiles(onvif_info* ov,const char* mediaURI);
int ONVIF_StreamURL(onvif_info* ov,const char* mediaURI, const char* profileToken, int stream_index);

#endif

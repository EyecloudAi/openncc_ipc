#include "base64.h"
//#include "sha1.h"
#include "onvif_common.h"
#include "nsmap.h"
#include "onvif_cmd.h"

static void ONVIF_GenrateDigest(unsigned char *pwddigest_out, unsigned char *pwd, char *nonc, char *time)
{
	const unsigned char *tdist;
	unsigned char dist[1024] = {0};
	char tmp[1024] = {0};
	unsigned char bout[1024] = {0};
	strcpy(tmp,nonc);
	base64_64_to_bits((char*)bout, tmp);
	sprintf(tmp,"%s%s%s",bout,time,pwd);
	sha1((unsigned char*)tmp,strlen((const char*)tmp),dist);
	tdist = dist;
	memset(bout,0x0,1024);
	base64_bits_to_64(bout,tdist,(int)strlen((const char*)tdist));
	strcpy((char *)pwddigest_out,(const char*)bout);
}

static void  generate_message_id(onvif_info* ov)
{
	unsigned char macaddr[6];
	char _HwId[1024];
	unsigned int Flagrand;

	// 为了保证每次搜索的时候MessageID都是不相同的！因为简单，直接取了随机值  
	srand((int)time(0));
	Flagrand = rand()%9000 + 1000; //保证四位整数  
	macaddr[0] = 0x1; macaddr[1] = 0x2; macaddr[2] = 0x3; macaddr[3] = 0x4; macaddr[4] = 0x5; macaddr[5] = 0x6;
	sprintf(_HwId,"urn:uuid:%ud68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X",Flagrand, macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
	ov->header.wsa__MessageID =(char *)malloc( 100);
	memset(ov->header.wsa__MessageID, 0, 100);
	strncpy(ov->header.wsa__MessageID, _HwId, strlen(_HwId));
}

 static int ONVIF_Initsoap(onvif_info* ov,  const char *was_To, const char *was_Action, int timeout)  
{  
	ov->soap = soap_new();
	if(ov->soap  == NULL)  
	{     
		printf("[%d]soap = NULL\n", __LINE__);
		return -1;
	}     
	soap_set_namespaces( ov->soap , namespaces);
	ov->soap ->version = 2;
	
	//超过5秒钟没有数据就退出  
	if (timeout > 0)  
	{     
		ov->soap ->recv_timeout = timeout;
		ov->soap ->send_timeout = timeout;
		ov->soap ->connect_timeout = timeout;
	}     
	else  
	{     
		//如果外部接口没有设备默认超时时间的话，我这里给了一个默认值10s  
		ov->soap ->recv_timeout    = 5; 
		ov->soap ->send_timeout    = 5; 
		ov->soap ->connect_timeout = 5; 
	}     
	soap_default_SOAP_ENV__Header(ov->soap , &ov->header);

	generate_message_id(ov);

#if 0
	// 这里开始作鉴权处理了，如果有用户信息的话，就会处理鉴权问题
	//如果设备端不需要鉴权的话，在外层调用此接口的时候把User信息填空就可以了
	if((strlen(ov->stUserInfo.username)>0)&&(strlen(ov->stUserInfo.password)>0))
	{
		ov->header.wsse__Security = (struct _wsse__Security *)calloc(1, sizeof(struct _wsse__Security));

		ov->header.wsse__Security->UsernameToken = (struct _wsse__UsernameToken *)calloc(1,sizeof(struct _wsse__UsernameToken));

		ov->header.wsse__Security->UsernameToken->Username = (char *)calloc(1,64);
		ov->header.wsse__Security->UsernameToken->Nonce = (char*)calloc(1, 64);
		ov->header.wsse__Security->UsernameToken->wsu__Created = (char*)calloc(1,64);
		ov->header.wsse__Security->UsernameToken->Password = (struct _wsse__Password *)malloc(sizeof(struct _wsse__Password));
		ov->header.wsse__Security->UsernameToken->Password->Type = (char*)calloc( 1,128);
		ov->header.wsse__Security->UsernameToken->Password->__item = (char*)malloc(128);

		strcpy(ov->header.wsse__Security->UsernameToken->Nonce,"LKqI6G/AikKCQrN0zqZFlg=="); //注意这里
		strcpy(ov->header.wsse__Security->UsernameToken->wsu__Created,"2018-04-01T07:50:45Z");
		strcpy(ov->header.wsse__Security->UsernameToken->Username, ov->stUserInfo.username);
		strcpy(ov->header.wsse__Security->UsernameToken->Password->Type,\
			"http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-username-token-profile-1.0#PasswordDigest");

		ONVIF_GenrateDigest((unsigned char*)ov->header.wsse__Security->UsernameToken->Password->__item,\
			(unsigned char*)ov->stUserInfo.password,ov->header.wsse__Security->UsernameToken->Nonce,ov->header.wsse__Security->UsernameToken->wsu__Created);
	}
#endif
	if (was_Action != NULL)  
	{  
		ov->header.wsa__Action  =(char *)malloc(1024);
		strncpy(ov->header.wsa__Action, was_Action, 1024);//"http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
	}  
	if (was_To != NULL)  
	{  
		ov->header.wsa__To =(char *)malloc(1024);
		strncpy(ov->header.wsa__To,  was_To, 1024);//"urn:schemas-xmlsoap-org:ws:2005:04:discovery";   
	}
	ov->soap->header = &ov->header;
	return 0;
}  

static void ONVIF_UnInitsoap(onvif_info* ov)
{
	soap_destroy(ov->soap);
	soap_end(ov->soap);
	soap_free(ov->soap);	
}
	
int ONVIF_ClientDiscovery(onvif_info* ov )  
{  
	int HasDev = 0;
	int retval = SOAP_OK;

	wsdd__ProbeType req;
	wsdd__ScopesType sScope;

	struct __wsdd__ProbeMatches resp;

	const char *was_To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
	const char *was_Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";     
	const char *soap_endpoint = "soap.udp://239.255.255.250:3702/";
	
	if (ONVIF_Initsoap(ov, was_To, was_Action, 5)<0)
	{
		printf("ONVIF_Initsoap error!\n");
		return -1;
	}
	
	soap_default_wsdd__ScopesType(ov->soap, &sScope);
	sScope.__item = "";

	soap_default_wsdd__ProbeType(ov->soap, &req);
	req.Scopes = &sScope;
	req.Types = "tds:Device"; //"dn:NetworkVideoTransmitter";

	retval = soap_send___wsdd__Probe(ov->soap, soap_endpoint, NULL, &req);
	//发送组播消息成功后，开始循环接收各位设备发送过来的消息  
	while (retval == SOAP_OK)  
	{  
		retval = soap_recv___wsdd__ProbeMatches(ov->soap, &resp);
		if (retval == SOAP_OK)  
		{  
			if (ov->soap->error)  
			{  
				printf("[%d]: recv error:%d,%s,%s\n", __LINE__, ov->soap->error, *soap_faultcode(ov->soap), *soap_faultstring(ov->soap));
				retval = ov->soap->error;
			}  
			else //成功接收某一个设备的消息  
			{  
				HasDev ++;
				if (resp.wsdd__ProbeMatches->ProbeMatch != NULL && resp.wsdd__ProbeMatches->ProbeMatch->XAddrs != NULL)  
				{				
					printf(" ################  recv  %d devices info #### \n", HasDev );
					printf("Target Service Address  : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
					printf("Target EP Address       : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
					printf("Target Type             : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->Types);
					printf("Target Metadata Version : %d\r\n", resp.wsdd__ProbeMatches->ProbeMatch->MetadataVersion);
										
					char* p=strchr(resp.wsdd__ProbeMatches->ProbeMatch->XAddrs,' ');
					if (p==NULL) 
						p=strchr(resp.wsdd__ProbeMatches->ProbeMatch->XAddrs,'\n');
					
					if (p != NULL)
					{
						int size=p -resp.wsdd__ProbeMatches->ProbeMatch->XAddrs;
						if (size >sizeof(ov->server_addr))
							size=sizeof(ov->server_addr);
						memcpy(ov->server_addr,resp.wsdd__ProbeMatches->ProbeMatch->XAddrs,size);
						break;
				  	//strncpy(ov->server_addr,resp.wsdd__ProbeMatches->ProbeMatch->XAddrs,sizeof(ov->server_addr));
					}
					//Sleep(1);				
				}  
			}  
		}  
		else if (ov->soap->error)  
		{  
			if (HasDev == 0)  
			{  
				printf("[%s][%d] Thers Device discovery or soap error: %d, %s, %s \n", __FUNCTION__, __LINE__, ov->soap->error, *soap_faultcode(ov->soap), *soap_faultstring(ov->soap));
				retval = ov->soap->error;
			}  
			else  
			{  
				printf(" [%s]-[%d] Search end! It has Searched %d devices! \n", __FUNCTION__, __LINE__, HasDev);
				retval = 0;
			}  
			break;
		}  
	}  

	ONVIF_UnInitsoap(ov);
	return retval;
} 

int ONVIF_Capabilities(onvif_info* ov,const char* deviceURI)  //获取设备能力接口
{
	int retval = 0;
	int result = 0;

	struct _tds__GetCapabilities capa_req;
	struct _tds__GetCapabilitiesResponse capa_resp;	

	if (ONVIF_Initsoap(ov, NULL, NULL, 5)<0)
	{
		printf("ONVIF_Initsoap error!\n");
		return -1;
	}
	
	//鉴权信息
	result = soap_call___tds__GetCapabilities(ov->soap, deviceURI, NULL, &capa_req, &capa_resp);
	if(result == -1)        
	{ 
		printf("soap error: %d, %s, %s\n", ov->soap->error, *soap_faultcode(ov->soap), *soap_faultstring(ov->soap));
		retval = result;
	}
	else
	{        
		if(capa_resp.Capabilities==NULL)
		{
			printf("GetCapabilities failed! result=%d \n",result);
			if (result == 501)
			{
				printf("Please authorize your access !\n");
			}
			retval = -1;
		}
		else
		{
			printf("GetCapabilities  OK! result=%d \n",result);
			printf("Media XAddr = %s \n", capa_resp.Capabilities->Media->XAddr);
			printf("Imaging XAddr = %s \n", capa_resp.Capabilities->Imaging->XAddr);
			printf("RTPMulticast = %s \n", capa_resp.Capabilities->Media->StreamingCapabilities->RTPMulticast ? "true" : "false");
			
			strncpy(ov->media_addr,capa_resp.Capabilities->Media->XAddr,sizeof(ov->media_addr));
			retval = 0; 
		}
	}
	ONVIF_UnInitsoap(ov);		
	return retval;
}

int ONVIF_Profiles(onvif_info* ov,const char* mediaURI)
{
	int result = 0;

	_trt__GetProfiles request;
	_trt__GetProfilesResponse reponse;	

	if (ONVIF_Initsoap(ov, NULL, NULL, 5)<0)
	{
		printf("ONVIF_Initsoap error!\n");
		return -1;
	}
	
	result = soap_call___trt__GetProfiles(ov->soap, mediaURI, NULL, &request, &reponse);
	if(result == -1)   
	{
		printf("soap error: %d, %s, %s\n", ov->soap->error, *soap_faultcode(ov->soap), *soap_faultstring(ov->soap));
	}
	else
	{        
		std::vector<tt__Profile * >::iterator itr;
		int i = 0;
		for (itr = reponse.Profiles.begin(); itr != reponse.Profiles.end(); itr++, i++)
		{
			
			printf("Profiles[%d]->name %s\n", i, (*itr)->Name.c_str());
			printf("Profiles[%d]->token %s\n", i, (*itr)->token.c_str());
			//printf("Profiles[%d]->fixed %s\n", i, (*itr)->fixed ? "true" : "false");
			printf("Profiles[%d]->VideoEncoderConfiguration->Name %s\n", i, (*itr)->VideoEncoderConfiguration->Name.c_str());
			printf("Profiles[%d]->VideoEncoderConfiguration->token %s\n", i, (*itr)->VideoEncoderConfiguration->token.c_str());
			printf("Profiles[%d]->VideoEncoderConfiguration->Encoding %d\n", i, (*itr)->VideoEncoderConfiguration->Encoding);
			printf("Profiles[%d]->VideoEncoderConfiguration->Resolution->Width %d\n", i, (*itr)->VideoEncoderConfiguration->Resolution->Width);
			printf("Profiles[%d]->VideoEncoderConfiguration->Resolution->Height %d\n", i, (*itr)->VideoEncoderConfiguration->Resolution->Height);
			printf("Profiles[%d]->VideoEncoderConfiguration->RateControl->FrameRateLimit %d\n", i, (*itr)->VideoEncoderConfiguration->RateControl->FrameRateLimit);
			printf("Profiles[%d]->VideoEncoderConfiguration->RateControl->BitrateLimit %d\n", i, (*itr)->VideoEncoderConfiguration->RateControl->BitrateLimit);
			
			if  ((*itr)->Name=="mainStream")
			{
				strncpy(ov->media_info[0].token,(*itr)->token.c_str(),sizeof(ov->media_info[0].token));
				strncpy(ov->media_info[0].name,(*itr)->Name.c_str(),sizeof(ov->media_info[0].name));
				ov->media_info[0].encoder = (*itr)->VideoEncoderConfiguration->Encoding;
				ov->media_info[0].width =  (*itr)->VideoEncoderConfiguration->Resolution->Width;
				ov->media_info[0].height =  (*itr)->VideoEncoderConfiguration->Resolution->Height;
			}
			else  if  ((*itr)->Name=="subStream")
			{
				strncpy(ov->media_info[1].token,(*itr)->token.c_str(),sizeof(ov->media_info[1].token));
				strncpy(ov->media_info[1].name,(*itr)->Name.c_str(),sizeof(ov->media_info[1].name));
				ov->media_info[1].encoder = (*itr)->VideoEncoderConfiguration->Encoding;
				ov->media_info[1].width =  (*itr)->VideoEncoderConfiguration->Resolution->Width;
				ov->media_info[1].height =  (*itr)->VideoEncoderConfiguration->Resolution->Height;				
			}
		}
	}
	ONVIF_UnInitsoap(ov);		
	return result;
}

int ONVIF_StreamURL(onvif_info* ov,const char* mediaURI, const char* profileToken,int stream_index)
{
	int result = 0;

	_trt__GetStreamUri request;
	_trt__GetStreamUriResponse response;	

	request.ProfileToken = profileToken;

	if (ONVIF_Initsoap(ov, NULL, NULL, 5)<0)
	{
		printf("ONVIF_Initsoap error!\n");
		return -1;
	}
	
	//获取RTSP流地址
	result = soap_call___trt__GetStreamUri(ov->soap, mediaURI, NULL, &request, &response);
	if(result == -1)
	{
		printf("soap error: %d, %s, %s\n", ov->soap->error, *soap_faultcode(ov->soap), *soap_faultstring(ov->soap));
	}
	else if(result == SOAP_OK)
	{        
		printf("StreamURI : %s\n", response.MediaUri->Uri.c_str());
		strncpy(ov->media_info[stream_index].media_rtsp,response.MediaUri->Uri.c_str(),sizeof(ov->media_info[stream_index].media_rtsp));
	}
	ONVIF_UnInitsoap(ov);	
	return result;
}


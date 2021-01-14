#include "sysinfo.h"
#include "includes.h"
#include <sys/statfs.h>  
#include <sys/sysinfo.h>
#include  "message.h"
#include "param_struct.h"
#include "md5.h"
#include <net/if.h>
#include <netinet/in.h>  
#include <arpa/inet.h>  

#define  MOUNTFILE    "/media/pi/disk"
//#define  MOUNTFILE    "/mnt"
#define  EXE_PATH  "/tmp"  //var

#define RASP

#ifdef   RASP           
#define  REMOTE_UPGRADE_FILE    "/home/pi/work/update.ec" 
#else
#define  REMOTE_UPGRADE_FILE    "/chird/eye/work/update.ec"    
#endif

#define  VERSION "wtdog1.0.0"

static int bupdated = 0;
static int reboot=0;
static int bchek_disk=0;

static void watch_dog(const char* process,const char* exe_process)
{
	char ps[128];  
	sprintf(ps,"pgrep %s  > /dev/null",process);  
	if (system(ps)>0)
	{
		system(exe_process);
		sprintf(ps,"Run %s \n",process);
		PTRACE(ps);
	}		
}

static int _file_exist(const char* file)
{
	return  (access( file, F_OK ) != -1 );
}

static int get_up_md5(char* md5,int size)
{
	KERNEL_MESSAGE msg;

	msg.command = SYS_GET_UP_MD5;
	msg.receiver=CORE_APP_MOD;
	msg.sender=WT_APP_MOD;
	msg.length=0;
	
	int ret = send_recv_timeout((char*)&msg, sizeof(MESSAG_HEADER) + msg.length,MAIN_CTRL_PORT,
														(char*)&msg,sizeof(msg),2);														
	if (ret<0)
		return -1;
	msg.data[msg.length]=0;	
	PTRACE("get md5 %s ret=%d",msg.data,ret);
	strncpy(md5,msg.data,size);	
	return 0;													
}

static int set_up_md5(const char* md5,int size)
{
	KERNEL_MESSAGE msg;

	msg.command = SYS_SET_UP_MD5;
	msg.receiver=CORE_APP_MOD;
	msg.sender=WT_APP_MOD;
	memcpy(msg.data,md5,size);
	msg.length=size;
	int ret = send_recv_timeout((char*)&msg, sizeof(MESSAG_HEADER) + msg.length,MAIN_CTRL_PORT,
														(char*)&msg,sizeof(msg),2);														
	if (ret<0)
		return -1;
	PTRACE("set md5 %s ret=%d",md5,ret);
	return 	(msg.data[0]=1)?0:-1;											
}

//return 0 err,1 success for cmod echo result
static int send_gui_cmd(const char* hint)
{
	KERNEL_MESSAGE msg;
	
	memcpy(msg.data,hint,strlen(hint));
	msg.length = strlen(hint);
	msg.command=SYS_UI_HINT;
	msg.receiver=GUI_APP_MOD;
	msg.sender=WT_APP_MOD;
	
	int ret= send_recv_timeout((char*)&msg, sizeof(MESSAG_HEADER) + msg.length,GUI_CTRL_PORT,
														(char*)&msg,sizeof(msg),2);
														
	if (ret<0)
		return 0;
  else 
  	return msg.data[0];
}

static int execute_update(char* filename,int blocal)
{
		char md5_str[50];
		char cmd[200];
		
		int ret=-1;
		
		bupdated=1;
		if (_file_exist(filename))  //if file exist,check md5 first
		{				
			PTRACE("check  md5 for file %s !",filename);
			ret=cal_file_md5(filename,md5_str);		
			if (ret==0)
			{
				char xml_md5[50];
				ret=get_up_md5(xml_md5,sizeof(xml_md5));
				if (ret==0)
				{
						if (strcmp(xml_md5,md5_str)==0)
						{
								PTRACE("the same upgrade package ,no need upgrade!");
								ret=-1;
						}
						else 
						{
							  PTRACE("md5 different,accept update!");
								ret = 0;
						}
				}
			}	
		}	
			
		if (ret==0)
		{
		   	PTRACE("find %s and update local:%d .....! ",filename,blocal);	
				sprintf(cmd,"tar xzvf %s -C %s; sync",filename,EXE_PATH);	
				system(cmd);		
				//sleep(1);					
				sprintf(cmd,"%s/update/%s",EXE_PATH,"update.sh"); 
				if (!_file_exist(cmd)) 
				{
					PTRACE("no find %s,exit!",cmd);
					ret = -1;
					return ret;
				}
				if (blocal)
					send_gui_cmd("update by udisk...");
				else
					send_gui_cmd("update by remote...");	
							
				sprintf(cmd,"cd %s/update/; sudo ./update.sh",EXE_PATH);
				system(cmd);	
				send_gui_cmd("update success!");
				if (!blocal) //if remote delete
				{
					sprintf(cmd,"sudo rm %s",filename);
					system(cmd);
				}
				PTRACE("update ok! then save md5");
				set_up_md5(md5_str,strlen(md5_str));
				reboot = 1;
		}		
		bupdated = 0;
		return ret;
}

static  void check_udisk_file(void* files)
{
	char name[200];	
								
	sprintf(name,"%s/%s",MOUNTFILE,INSTALL_FILE);
	if ((bchek_disk==0)&&(_file_exist(name)) ) 
	{		
		int ret=execute_update(name,1);
		if  ((ret==0)&&(reboot==1))
			system("reboot");
	  bchek_disk = 1; 		
	}
	
	if (!_file_exist(name)) 
		bchek_disk=0;
}

// /chird/eye/work/
static void  watch_all_program(void* program)
{
	if (!bupdated)
	{			
#ifdef RASP		
		  watch_dog("mainApp","/home/pi/work/mainApp &");		  
		  watch_dog("Rtsp","/home/pi/work/Rtsp &");
		  watch_dog("Onvif","/home/pi/work/Onvif &");
#else
		  watch_dog("mainApp","/chird/eye/work/mainApp &");		  
		  watch_dog("Rtsp","/chird/eye/work/Rtsp &");
		  watch_dog("Onvif","/chird/eye/work/Onvif &");
#endif
	}	
}

static int get_ip(const char *adapter_name, struct sockaddr *addr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    memset(addr, 0, sizeof(struct sockaddr));

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0){
        *addr = ifr.ifr_addr;
    }

	//char* self_ip=inet_ntoa((*(struct sockaddr_in *)addr).sin_addr);
	//PRINT_DBG("get  ip %s \n",self_ip);
	
    close(skfd);
    return 0;
}

static void udp_ctrl_loop(INT16U port)
{
	KERNEL_MESSAGE msg ;
	int val,ret;
	int addr_len;
	struct sockaddr_in addr;
	struct sockaddr clientAddr;
	static int udp_server;
	
	val = 1;
	addr_len = sizeof(struct sockaddr);
	addr.sin_family      = AF_INET       ;
	addr.sin_addr.s_addr = htonl ( INADDR_ANY ) ;
	addr.sin_port        = htons ( port) ;
	
	udp_server  = socket( AF_INET, SOCK_DGRAM, 0 );
	setsockopt( udp_server , SOL_SOCKET , SO_REUSEADDR , &val , sizeof(int) ) ;
	if( bind  ( udp_server , ( struct sockaddr *)&addr , addr_len ) < 0 )
	{
        PTRACE("start_network: Bind UDP PORT[%d] Error ." , port ) ;
        close(udp_server);
        return ;
  }
  PTRACE("start_network: Bind UDP PORT[%d] OK ." , port  ) ;
  		
	while(1)
	{
		memset(&msg,0,sizeof(msg));
		ret = recvfrom(udp_server, (char*)&msg, sizeof(msg), 0,
			&clientAddr, (socklen_t *)&addr_len);
	
		if ( ret <= 0 && errno != EINTR )
			continue;
		if (ret>0)
		{
			char name[200];
			if ((msg.command==SYS_APP_UPDATE)&&(msg.length<sizeof(name)))//recv update command
			{
				strcpy(name,msg.data);
				name[msg.length]=0;
				PTRACE("recv SYS_APP_UPDATE %s  len:%d",name,msg.length);
				char* p=strstr(name,INSTALL_FILE);
				if (p==NULL) continue;
							 
				ret=execute_update(name,0);		
				
				if (ret<0)
					 msg.data[0]=0;
				else 	
					 msg.data[0]=1;
			 		
				msg.length = 1;	
				sendto( udp_server , (char*)&msg , sizeof(MESSAG_HEADER) + msg.length  , 0 , &clientAddr , addr_len ) ; 	
				if  ((ret==0)&&(reboot==1))
				{
					os_sleep(500); //wait for ack to remote client ok
					system("reboot");
				}
			}	
			else if (msg.command==APP_GET_VERSION)
			{
					int len=sprintf(msg.data,"%s-time:%s",VERSION,__DATE__);
					msg.length=len;
					sendto( udp_server , (char*)&msg , sizeof(MESSAG_HEADER) + msg.length  , 0 , &clientAddr , addr_len ) ; 			
			}
			else if (APP_IPC_ADDR==msg.command)
			{
					 struct sockaddr addr;
					 get_ip("eth0",&addr);
					 char* self_ip = inet_ntoa((*(struct sockaddr_in *)&addr).sin_addr);
					 char rtsp_url[200];
					 int len=sprintf(rtsp_url,"rtsp://%s:8554/liveRGB",self_ip);			
					// msg.length=len;
					// ret=sendto( udp_server , (char*)&msg , sizeof(MESSAG_HEADER) + msg.length  , 0 , &clientAddr , addr_len );	 
					 ret=sendto( udp_server , rtsp_url , len , 0 , &clientAddr , addr_len ) ;
					// PTRACE("recv APP_IPC_ADDR  and return len %d=%d %s err:%s",len,ret,msg.data,strerror(errno)); 				 
			}
			else
			{
					PTRACE("invalid  command 0x%6x length=%d recvied! closed!",msg.command,msg.length);
			}
		}		
	}	
}

int main(int argc ,char *argv[])
{	
	INT16U port = WT_CTRL_PORT;
	if (argc>1)
		port=atoi(argv[1]);

#if 1
	PTRACE("boot first check %s",REMOTE_UPGRADE_FILE);
	if (_file_exist(REMOTE_UPGRADE_FILE)) 
	{
		int ret=execute_update(REMOTE_UPGRADE_FILE,1);
	//	if (ret==0)//file exist ,upgrade ok
		{
			char cmd[100];
			sprintf(cmd,"rm -f  %s ",REMOTE_UPGRADE_FILE);
			system(cmd);
		}
		if  ((ret==0)&&(reboot==1))
			system("reboot");
	}
	//add end
#endif

	core_timer_init();	
	sleep(5);//wait for run all program run ok then check program is running
	core_timer_start(SEC_UNIT*10,TIMER_PERIOD,NULL,check_udisk_file);
	core_timer_start(SEC_UNIT*10,TIMER_PERIOD,NULL,watch_all_program);
	udp_ctrl_loop(port);
	return 0;
}


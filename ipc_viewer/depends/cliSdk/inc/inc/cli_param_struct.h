#ifndef __PARAM_STRUCT_H__
#define __PARAM_STRUCT_H__

#define MAX_POINT  3
#define MAX_AREA   3

#define  _PACKED_			__attribute__((packed)) 

typedef struct
{
	int CalTmode;// 测温计算模式0:内 1：外
	int RegionId;  //区号
	int camlens; //镜头类型
	int BGTemp100; //背景温度  
	int AmbTemp100; //环境温度   UNIT K
	int distance;  //距离 0.01米
	int emi100;   //比辐射率
	int humidity; //湿度
	int reservd[22];
}_PACKED_ mt_param;

typedef struct
{
	int x;
	int y;
}_PACKED_ IR_POINT;

typedef struct
{
	int x;
	int y;
	int w;
	int h;
}_PACKED_ IR_AREA;

typedef struct
{
	BYTE type;    //0: ai  >0 index	
  IR_AREA rgbRect;    
  IR_AREA irRect;    
  IR_POINT point;//max temp point
  int temp100;  //X100
}_PACKED_ AreaTemp;

typedef struct
{
  IR_POINT point;//max temp point
  int temp100;  //X100
}_PACKED_ PointTemp;

typedef struct 
{
	char filename[200];
	int filesize;
}_PACKED_ FaceFile;

#endif

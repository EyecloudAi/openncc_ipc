#CROSS_COMPILE =aarch64-linux-gnu-
#CROSS_COMPILE = arm-linux-gnueabihf-
#CROSS_COMPILE = mipsel-openwrt-linux-
CC = $(CROSS_COMPILE)gcc 
CPP = $(CROSS_COMPILE)g++ -std=c++11
CXX = $(CROSS_COMPILE)g++ -std=c++11
AR = $(CROSS_COMPILE)ar 
LINK=$(CROSS_COMPILE)g++

TARGET =  Onvif
MYLIB  = libOnvif.a

CFLAGS = -D_GNU_SOURCE -Wall -O2 
LFLAGS =-lpthread -lm

ONVIF_OBJS  = src/duration.o
ONVIF_OBJS += src/soapC.o
ONVIF_OBJS += src/soapClient.o
ONVIF_OBJS += src/soapServer.o
ONVIF_OBJS += src/stdsoap2.o
#ONVIF_OBJS += src/onvif_server.o
ONVIF_OBJS += src/onvif_server_interface.o
ONVIF_OBJS += src/onvif_function.o
ONVIF_OBJS += src/onvif_imaging.o
ONVIF_OBJS += src/onvif_media.o
ONVIF_OBJS += src/onvif_net.o
ONVIF_OBJS += src/onvif_devicemgmt.o
ONVIF_OBJS += src/onvif_discovery.o
ONVIF_OBJS += src/onvif_common.o
ONVIF_OBJS += src/inifile.o

default:all
all: src/onvif_server.o $(MYLIB)
	$(CC) -o $(TARGET) $^ $(CFLAGS) $(LFLAGS) 
	@echo "over"

$(MYLIB): $(ONVIF_OBJS)
	$(AR) rcs $(MYLIB) $(ONVIF_OBJS) 
	
cli: src/onvif_client.o src/onvif_cmd.o $(MYLIB)
	$(CC) -o OnvifCli $^ $(CFLAGS) $(LFLAGS) 
	@echo "over"
	
%.o:%.c
	-@echo -e "\033[41;32m  compiling $<... \033[0m"
	$(CC)  -c $< -o $@ $(CFLAGS)

clean:
	rm -f  $(ONVIF_OBJS) src/onvif_server.o src/onvif_client.o  $(TARGET) 
	

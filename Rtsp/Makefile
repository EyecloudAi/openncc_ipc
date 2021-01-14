#CROSS_COMPILE =aarch64-linux-gnu-
#CROSS_COMPILE = arm-linux-gnueabihf-
#CROSS_COMPILE = mipsel-openwrt-linux-
CC = $(CROSS_COMPILE)gcc 
CPP = $(CROSS_COMPILE)g++ -std=c++11
CXX = $(CROSS_COMPILE)g++ -std=c++11
AR = $(CROSS_COMPILE)ar 
LINK=$(CROSS_COMPILE)g++

INCS = -I./inc   -I../public/include/  -I./ncc_inc
INCS += -Ilive/UsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/BasicUsageEnvironment/include
LIVE555_LIBS =  live/liveMedia/libliveMedia.a live/groupsock/libgroupsock.a \
				live/BasicUsageEnvironment/libBasicUsageEnvironment.a live/UsageEnvironment/libUsageEnvironment.a

#OPENCV = `pkg-config opencv --cflags --libs`
PUBLIC_LIB=../public/libPlatform.a
#LIB_NCC=./libOpenNCC.a
#PUBLIC_LIB	+= /usr/local/lib/libyuv.a

CFLAGS := -Wall -g
LDFLAGS := $(OPENCV) -lpthread  -lrt -lusb-1.0  -lm -ldl
#-lavcodec -lavformat -lavutil -lswscale -lswresample -lx264 

#for hard access
#LDFLAGS += -lmmal_core -lmmal_util -lmmal_vc_client -lbcm_host -L/opt/vc/lib/

#INCS += -I/opt/libusb/include/libusb-1.0
#LDFLAGS += -L/opt/libusb/lib 

PWD=$(shell pwd)

TARGET =  $(notdir $(PWD))
#TARGET =  TestRtsp
# change to you project file dir
VPATH = ./  
# the obj dir
OBJDIR = obj
BINDIR = bin
###########################################################################
# source files
SRCS = $(foreach dir,$(subst :, ,$(VPATH)),$(wildcard $(dir)/*.cpp))
SRCSC = $(foreach dir,$(subst :, ,$(VPATH)),$(wildcard $(dir)/*.c))
# obj files
OBJS_1 = $(addsuffix .o,$(basename $(SRCS)))
OBJSC_1 = $(addsuffix .o,$(basename $(SRCSC)))
OBJS = $(foreach n,$(notdir $(OBJS_1)),$(OBJDIR)/$(n))
OBJSC = $(foreach n,$(notdir $(OBJSC_1)),$(OBJDIR)/$(n))
# head files
HEADERS = $(foreach dir,$(subst :, ,$(VPATH)),$(wildcard $(dir)/*.h))
HEADERS += $(foreach dir,$(subst :, ,$(VPATH)),$(wildcard $(dir)/*.hpp))
HEADERS += $(foreach dir,$(subst :, ,$(VPATH)),$(wildcard $(dir)/*.inc))

INCS += $(patsubst %,-I%,$(subst :, ,$(VPATH)))
CFLAGS += $(INCS)
CXXFLAGS += $(INCS)

all: $(TARGET)

$(TARGET): $(OBJS) $(OBJSC) 
	@echo "C_SOURCES: $(SRCSC)"
	@echo "CPP_SOURCES: $(SRCS)"
	@echo "TARGET: $(TARGET)"
	$(CXX) -o $@ $^ $(LIVE555_LIBS) $(PUBLIC_LIB)  $(CXXFLAGS) $(LDFLAGS) 
#	@test -d $(BINDIR) | mkdir -p $(BINDIR)
#	mv $@ $(BINDIR)/
	
# *.cpp files commpare
$(OBJS): $(SRCS) $(HEADERS)
	@test -d $(OBJDIR) | mkdir -p $(OBJDIR)
	$(CXX) -c $(SRCS) $(CXXFLAGS)
	mv *.o $(OBJDIR)/

# *.c file commpare
$(OBJSC): $(SRCSC) $(HEADERS)
	@test -d $(OBJDIR) | mkdir -p $(OBJDIR)
	$(CC) -c $(SRCSC) $(CFLAGS)
	mv *.o $(OBJDIR)/

clean:
	rm -rf $(OBJDIR)
	rm -f  $(BINDIR)/$(TARGET)

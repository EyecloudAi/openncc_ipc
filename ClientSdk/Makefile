#CROSS_COMPILE =aarch64-linux-gnu-
#CROSS_COMPILE =arm-hisiv500-linux-
CC = $(CROSS_COMPILE)gcc 
CPP = $(CROSS_COMPILE)g++ -std=c++11
CXX = $(CROSS_COMPILE)g++ -std=c++11
AR = $(CROSS_COMPILE)ar 
LINK=$(CROSS_COMPILE)g++

INCS =   -Iinc -I../public/include
COMMFLAG=  -fPIC 

CFLAGS = -D_GNU_SOURCE -Wall  -O2 -g
LDFLAGS = -lpthread

PWD=$(shell pwd)
TARGET_LIB = libClientSdk.a
TARGET =  CliTest

# change to you project file dir
VPATH =  sdk
# the obj dir
OBJDIR = obj
BINDIR = .
###########################################################################

TEST_SOURCES = $(wildcard *.cpp)
TEST_OBIS = $(TEST_SOURCES:%.cpp=%.o)

# head files
HEADERS = $(foreach dir,$(subst :, ,$(VPATH)),$(wildcard $(dir)/*.h))
HEADERS += $(foreach dir,$(subst :, ,$(VPATH)),$(wildcard $(dir)/*.hpp))

INCS += $(patsubst %,-I%,$(subst :, ,$(VPATH)))
CFLAGS += $(INCS) $(COMMFLAG)
CXXFLAGS += $(INCS) $(COMMFLAG)

all:$(TARGET)

$(TARGET): $(TEST_OBIS)   
	@echo "TEST_SOURCES: $(TEST_SOURCES)"
	$(CXX) -o $@ $^  $(TARGET_LIB) $(CXXFLAGS) $(LDFLAGS) 
		
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

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<  

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o: %.cxx
	$(CXX) -c $(CXXFLAGS)  -o $@ $<
		
clean:
	rm -rf $(OBJDIR)
	rm -f  $(TARGET)

 
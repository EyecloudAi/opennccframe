#CROSS_COMPILE = aarch64-linux-gnu-

PWD=$(shell pwd)
TARGET =  $(notdir $(PWD))

CC  := $(CROSS_COMPILE) gcc
CXX := $(CROSS_COMPILE) g++ -std=c++11
AR  := $(CROSS_COMPILE) ar 

CFLAGS = -D _GNU_SOURCE -Wall  -O2 
INCDIRS = /usr/include/openncc \

OPENCV   = `pkg-config opencv --cflags --libs` 

LIBS     = -lrt -ldl  -lusb-1.0  -lpthread 
LDFLAGS +=  $(OPENCV) 

TARGET_LIB = /usr/lib/openncc/libOpenNCC_native.so
BINDIR = build
###########################################################################
TEST_SOURCES = $(wildcard *.cpp)
TEST_OBIS = $(TEST_SOURCES:%.cpp=%.o)

INCLUDE= -I $(INCDIRS)
CFLAGS += $(INCLUDE)   -D__PC__
CXXFLAGS += $(INCLUDE) -D__PC__

help:
	@echo
	@echo "\t|--------------------------------------------------------------|"
	@echo "\t| Help                                                         |"
	@echo "\t| ----                                                         |"
	@echo "\t| make all                                                     |"
	@echo "\t|--------------------------------------------------------------|"
	@echo
	
all: $(TARGET)

$(TARGET): $(TEST_OBIS)  
	@echo "TEST_SOURCES: $(TEST_SOURCES)"
	@test -d $(BINDIR) | mkdir -p $(BINDIR)
	$(CXX) -o $@ $^ $(TARGET_LIB) $(CXXFLAGS) $(LIBS) $(LDFLAGS)
	mv $@ $(BINDIR)/
	mv *.o $(BINDIR)/

%.o: %.c
	$(CC) -c $(CFLAGS)-o $@ $<
    
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

clean:
	rm -r  $(BINDIR)

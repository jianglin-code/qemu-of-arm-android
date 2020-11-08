############################################################# 
# Makefile for shared library.
# 编译动态链接库
#############################################################
#set your own environment option
#GCC	:= arm-linux-gnueabihf-gcc
#GXX	:= arm-linux-gnueabihf-g++
#LD	:= arm-linux-gnueabihf-ld
#AR	:= arm-linux-gnueabihf-ar
#CC = /home/jl/Downloads/gcc-linaro-aarch64-linux-gnu-4.8-2014.04_linux/bin/aarch64-linux-gnu-g++
CC_FLAG = -D__linux__ -DWITH_GLES2 -fPIC

#set your inc and lib 
INC = 
LIB = -lpthread -ldl

#make target lib and relevant obj 
PRG   = opengAPI
SRCS := $(wildcard *.cpp)  
OBJ  := $(patsubst %cpp,%o,$(SRCS)) 

#all target
all:$(PRG)

$(PRG):$(OBJ)
	#$(CC) -shared -o $@ $(OBJ) $(LIB)
	$(CC) $(INC) $(LIB) -o $@ $(OBJ)  

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)

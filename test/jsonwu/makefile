src=$(wildcard *.cpp)
objects=$(patsubst %.cpp, %.o, $(src))
VPATH = ../../common
INCLUDE_DIR=-I. -I../../common -I../../include -I../../libs/core/src
LIB_DIR=-L../../libs
LIB=-licore
target=mytest
CC=g++
MACROS=-DTEST

all:$(target)

$(target):$(objects) client.o
	$(CC) $(LIB_DIR)  $(objects) client.o  $(LIB)  -o $@ 

client.o:client.cpp
	$(CC) $(INCLUDE_DIR) $(MACROS) -g  -c $< -o  $@

%.o:%.cpp
	$(CC) $(INCLUDE_DIR) $(MACROS) -g  -c $< -o  $@

.PHONY:all clean

clean:
	-rm  -rf *.o $(target)

src=$(wildcard *.cpp)
obj=$(patsubst %.cpp, %.o, $(src))
cc=g++
include_dir=-I. -I../../include -I../../common
libs_dir=-L../../libs
libs=-licore
target=svc_test
debug_macor=-DTEST_DEBUG

all:$(target)

$(target):$(obj)
	$(cc) $(libs_dir) -o $@ $(obj) $(libs)
	@echo --------------------------------
	@echo $(target) ok
	@echo --------------------------------

%.o:%.cpp
	$(cc) -Wall $(include_dir) $(debug_macor) -g -c $< -o $@

.PHONY: all clean tag
clean:
	-rm -f $(target) $(obj)
tag:
	ctags -R *

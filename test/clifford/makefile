
#                           _              
#   ~Q~                    (_)             
#                 ___ _   ___ __ __      __
#                / __| | | | / __\ \ /\ / /
#               | (__  |_| | \__ \\ V  V / 
#                \___|\__,_|_|___/ \_/\_/  
#
# Copyright (C) 2008 - 8002, Cui Shaowei, <shaovie@gmail.com>, It's free.
# This is a general makefile template.
###########################################################################

                               BIN_DIR = ./
                                TARGET = $(BIN_DIR)/test
                              C_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                MACROS = -D_REENTRANT -D__USE_POSIX \
																				 -DSVC_EDITION=\"${svc_edition}\" \
																				 -DBIN_V=\"${bin_v}\"
                                    CC = gcc
                            CPP_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                  MAKE = make
                                LINKER = g++
                          INCLUDE_DIRS = -I../../include -I../../common
                                  LIBS = -lpthread -licore
                            OPTIM_FLAG = -O3 -g
                                   CPP = g++
                                LFLAGS = -Wl -fPIC
                              LIB_DIRS = -L../../libs
                                 VPATH = 
                            OBJECT_DIR = ./.obj/
                              CPPFILES = \
																				 test.cpp  \
																				 my_client.cpp

                                CFILES = 

# To use 'make quiet=1' all the build command will be hidden.
ifdef quiet
	ifeq ("$(origin quiet)", "command line")
		ifeq ($(quiet), 1)
	    Q = @
	  endif
	endif
endif

# To use 'make debug=0' build release edition.
ifdef debug
	ifeq ("$(origin debug)", "command line")
		ifeq ($(debug), 0)
	    MACROS += -DNDEBUG
		else
		  MACROS += -DDO_DEBUG
	  endif
	endif
else
  MACROS += -DDO_DEBUG
endif

# To use 'make publish=0' build release edition.
ifdef publish 
	ifeq ("$(origin publish)", "command line")
		ifeq ($(publish), 1)
		  MACROS += -DPUBLISH
	  endif
	endif
endif

# To use 'make platform=-DFOR_QQ' build release edition.
ifdef platform
	ifeq ("$(origin platform)", "command line")
	  MACROS += $(platform)
	endif
else
	MACROS += -DFOR_QQ
endif

OBJECTS := $(addprefix $(OBJECT_DIR), $(notdir $(CPPFILES:%.cpp=%.o)))
OBJECTS += $(addprefix $(OBJECT_DIR), $(notdir $(CFILES:%.c=%.o)))

CALL_CFLAGS := $(C_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
CPPALL_CFLAGS := $(CPP_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
LFLAGS += $(LIB_DIRS) $(LIBS) $(OPTIM_FLAG)

all: checkdir $(TARGET)

$(TARGET): $(OBJECTS)
	$(Q)$(LINKER) $(strip $(LFLAGS)) -o $@ $(OBJECTS)

$(OBJECT_DIR)%.o:%.cpp
	$(Q)$(CPP) $(strip $(CPPALL_CFLAGS)) -c $< -o $@

$(OBJECT_DIR)%.o:%.c
	$(Q)$(CC) $(strip $(CALL_CFLAGS)) -c $< -o $@

checkdir:
	@if ! [ -d "$(BIN_DIR)" ]; then \
		mkdir $(BIN_DIR) ; \
		fi
	@if ! [ -d "$(OBJECT_DIR)" ]; then \
		mkdir $(OBJECT_DIR); \
		fi
clean:
	$(Q)rm -f $(OBJECTS)
cleanall: clean
	$(Q)rm -f $(TARGET)

tag:
	$(Q)ctags -R ../common/* src/*
.PHONY: all clean cleanall checkdir tag

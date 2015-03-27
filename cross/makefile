###########################################################################
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

                               BIN_DIR = ./bin
                                TARGET = $(BIN_DIR)/icross
                              C_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                MACROS = -D_REENTRANT -D__USE_POSIX -DICROSS \
																				 -DSVC_EDITION=\"${svc_edition}\" \
																				 -DBIN_V=\"${bin_v}\"
                                    CC = gcc
                            CPP_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                  MAKE = make
                                LINKER = g++
                          INCLUDE_DIRS = -Isrc -I../include -I../common -I../common/db
                                  LIBS = -lpthread -licore -ljson
                            OPTIM_FLAG = -O3 -g
                                   CPP = g++
                                LFLAGS = -Wl -fPIC
                              LIB_DIRS = -L../libs
                                 VPATH = src ../common ../common/db
                            OBJECT_DIR = ./.obj/
                              CPPFILES = \
																				 daemon.cpp  \
																				 load_json.cpp  \
																				 sys_log.cpp \
																				 client.cpp  \
																				 time_util.cpp  \
																				 svc_config.cpp  \
																				 sys.cpp \
																				 global_timer.cpp \
																				 world_svc.cpp \
																				 world_svc_mgr.cpp \
																				 session_mgr.cpp  \
																				 request_mgr.cpp  \
																				 cross.cpp

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

# To use 'make platform=-DFOR_IROBOT' build release edition.
ifdef platform
	ifeq ("$(origin platform)", "command line")
	  MACROS += $(platform)
	endif
endif

OBJECTS := $(addprefix $(OBJECT_DIR), $(notdir $(CPPFILES:%.cpp=%.o)))
OBJECTS += $(addprefix $(OBJECT_DIR), $(notdir $(CFILES:%.c=%.o)))

CALL_CFLAGS := $(C_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
CPPALL_CFLAGS := $(CPP_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
LFLAGS += $(OPTIM_FLAG) $(LIB_DIRS) $(LIBS)

all: checkdir $(TARGET)
	@echo
	@echo -en "\\033[0;33m" ; more README
	@echo -en "\\033[0;39m"

$(TARGET): $(OBJECTS)
	$(Q)$(LINKER) -o $@ $(OBJECTS) $(strip $(LFLAGS))

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
	ctags --langmap=c++:+.inl -h +.inl --fields=+aiKSz --extra=+q -R ../common/* src/*
.PHONY: all clean cleanall checkdir tag

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
                                TARGET = $(BIN_DIR)/robotd
                              C_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                MACROS = -D_REENTRANT -D__USE_POSIX -DIROBOT -DDO_DEBUG
																				 #-DDO_STRACE
                                    CC = gcc
                            CPP_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                  MAKE = make
                                LINKER = g++
                          INCLUDE_DIRS = -Isrc -I../include -I../common -I../common/db -I../world/src/scene \
																				 -I../world/src/fight -I../world/src/bf -I../world/src/item \
																				 -Icommon -Isrc/model -Isrc/control -Isrc/view -I../world/src \
																				 -I../world/src/scp/ -I../world/src/activity
                                  LIBS = -lpthread -lz -ljson -licore -llua -lm -ldl -ltolua++ \
																				 -lncursesw -lpanel
                            OPTIM_FLAG = -O2 -g
                                   CPP = g++
                                LFLAGS = -Wl -fPIC
                              LIB_DIRS = -L../libs -L./
                                 VPATH = src ../common ../world/src/scene ../world/src/fight \
																				 ../world/src/bf ../world/src/item/ common/ ../world/src \
																				 src/model src/control src/view ../world/src/scp ../world/src/activity
                            OBJECT_DIR = ./.obj/
                              CPPFILES = \
                                         daemon.cpp  \
                                         client.cpp  \
                                         robot_ncurses.cpp  \
                                         snap_obj.cpp  \
                                         load_json.cpp  \
                                         time_util.cpp  \
																				 scene_unit_obj.cpp  \
																				 player.cpp  \
																				 lua_istream_pkg.cpp  \
																				 scene_config.cpp  \
																				 skill_config.cpp  \
																				 buff_config.cpp  \
																				 item_config.cpp  \
																				 item_effect_def.cpp  \
																				 scene_obj.cpp  \
																				 scene_mgr.cpp  \
																				 ai_func.cpp  \
																				 social_module.cpp  \
																				 team_module.cpp  \
																				 chat_module.cpp \
																				 market_module.cpp  \
																				 vlayer_map.cpp  \
																				 vscene_world.cpp  \
																				 director.cpp  \
																				 team_show.cpp \
																				 window.cpp  \
																				 chat_show.cpp \
																				 char_base_show.cpp \
																				 global_param_cfg.cpp  \
																				 gm_window.cpp \
																				 log_window.cpp \
																				 scp_config.cpp \
																				 robot.cpp

                                CFILES = 

# To use 'make one=1' build release edition.
ifdef one
	ifeq ("$(origin one)", "command line")
		IS_ONE = $(one)
	endif
endif
ifeq ($(IS_ONE), 1)
	MACROS += -DFOR_ONEROBOT
endif

# To use 'make quiet=1' all the build command will be hidden.
# To use 'make quiet=0' all the build command will be displayed.
ifdef quiet
	ifeq ("$(origin quiet)", "command line")
		QUIET = $(quiet)
	endif
endif
ifeq ($(QUIET), 1)
	Q = @
else
	Q =
endif

OBJECTS := $(addprefix $(OBJECT_DIR), $(notdir $(CPPFILES:%.cpp=%.o)))
OBJECTS += $(addprefix $(OBJECT_DIR), $(notdir $(CFILES:%.c=%.o)))

CALL_CFLAGS := $(C_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
CPPALL_CFLAGS := $(CPP_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
LFLAGS += $(LIB_DIRS) $(LIBS) $(OPTIM_FLAG)

all: checkdir $(TARGET)

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
	rm -f $(OBJECTS)
cleanall: clean
	rm -f $(TARGET)

tag:
	ctags -R ../common/* ./*
.PHONY: all clean cleanall checkdir tag

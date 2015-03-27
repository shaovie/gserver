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
                                TARGET = $(BIN_DIR)/db_proxy
                              C_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                MACROS = -D_REENTRANT -D__USE_POSIX -DDB_PROXY \
																				 -DSVC_EDITION=\"${svc_edition}\" \
																				 -DBIN_V=\"${bin_v}\"
                                    CC = gcc
                            CPP_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                  MAKE = make
                                LINKER = g++
                          INCLUDE_DIRS = -I. -I../include -I../common -I../common/db -Isrc
                                  LIBS = -lpthread -licore -lmysqlclient -ljson
                            OPTIM_FLAG = -O3 -g
                                   CPP = g++
                                LFLAGS = -Wl -fPIC
                              LIB_DIRS = -L../libs
                                 VPATH = src ../common ../common/db src/tb
                            OBJECT_DIR = ./.obj/
                              CPPFILES = \
																				 db_async_store.cpp  \
																				 mysql_db.cpp  \
																				 load_json.cpp  \
																				 daemon.cpp  \
																				 sys_log.cpp  \
																				 svc_config.cpp  \
																				 sql_measuring.cpp  \
																				 proxy_obj.cpp  \
																				 client.cpp  \
																				 tb_msg_map.cpp  \
																				 account_info_tb.cpp  \
																				 char_info_tb.cpp  \
																				 char_extra_info_tb.cpp  \
																				 daily_clean_info_tb.cpp  \
																				 char_db_msg_queue_tb.cpp  \
																				 social_tb.cpp  \
																				 task_tb.cpp  \
																				 task_bit_array_tb.cpp  \
																				 daily_task_tb.cpp  \
																				 skill_tb.cpp  \
																				 passive_skill_tb.cpp  \
																				 item_tb.cpp  \
																				 recharge_log_tb.cpp  \
																				 consume_log_tb.cpp  \
																				 mall_buy_log_tb.cpp  \
																				 baowu_mall_tb.cpp  \
																				 buff_tb.cpp  \
																				 mail_info_tb.cpp \
																				 guild_tb.cpp \
																				 guild_member_tb.cpp \
																				 guild_apply_tb.cpp \
																				 guild_skill_tb.cpp  \
																				 market_info_tb.cpp  \
																				 char_attr_tb.cpp  \
																				 tui_tu_log_tb.cpp  \
																				 scp_log_tb.cpp  \
																				 vip_tb.cpp  \
																				 cheng_jiu_info_tb.cpp  \
																				 bao_shi_tb.cpp  \
																				 title_info_tb.cpp  \
																				 sys_settings_tb.cpp  \
																				 forbid_opt_tb.cpp  \
																				 notice_info_tb.cpp  \
																				 huo_yue_du_info_tb.cpp  \
																				 jing_ji_rank_tb.cpp  \
																				 service_info_tb.cpp  \
																				 jing_ji_log_tb.cpp  \
																				 ltime_act_tb.cpp  \
																				 char_recharge_tb.cpp  \
																				 lucky_turn_tb.cpp  \
																				 lucky_turn_score_tb.cpp  \
																				 lucky_turn_big_award_tb.cpp  \
																				 ltime_recharge_award_tb.cpp  \
																				 worship_info_tb.cpp  \
																				 water_tree_tb.cpp  \
																				 tianfu_skill_tb.cpp  \
																				 lueduo_item_tb.cpp  \
																				 lueduo_log_tb.cpp  \
																				 dxc_info_tb.cpp  \
																				 kai_fu_act_log.cpp  \
																				 db_proxy.cpp

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

OBJECTS := $(addprefix $(OBJECT_DIR), $(notdir $(CPPFILES:%.cpp=%.o)))
OBJECTS += $(addprefix $(OBJECT_DIR), $(notdir $(CFILES:%.c=%.o)))

CALL_CFLAGS := $(C_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
CPPALL_CFLAGS := $(CPP_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
LFLAGS += $(OPTIM_FLAG) $(LIB_DIRS) $(LIBS)

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
	$(Q)rm -f $(OBJECTS)
cleanall: clean
	$(Q)rm -f $(TARGET)

tag:
	ctags -R ../common/* src/*
.PHONY: all clean cleanall checkdir tag

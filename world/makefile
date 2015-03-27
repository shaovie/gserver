###########################################################################
#                           _
#   ~Q~                    (_)
#                 ___ _   ___ __ __      __
#                / __| | | | / __\ \ /\ / /
#               | (__  |_| | \__ \\ V  V /
#               \___|\__,_|_|___/ \_/\_/
#
# Copyright (C) 2008 - 8002, Cui Shaowei, <shaovie@gmail.com>, It's free.
# This is a general makefile template.
###########################################################################

                               BIN_DIR = ./bin
                                TARGET = $(BIN_DIR)/iworld
                              C_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                MACROS = -D_REENTRANT -D__USE_POSIX -DIWORLD \
																				 -DSVC_EDITION=\"${svc_edition}\" \
																				 -DBIN_V=\"${bin_v}\"
                                    CC = gcc
                            CPP_CFLAGS = -Wall -W -Wpointer-arith -pipe -fPIC
                                  MAKE = make
                                LINKER = g++
                          INCLUDE_DIRS = -Isrc -I../include -I../common -I../common/db -Isrc/http \
																				 -Isrc/player -Isrc/scene -Isrc/monster -Isrc/task \
																				 -Isrc/fight -Isrc/item -Isrc/chat -Isrc/bf -Isrc/trade \
																				 -Isrc/scp -Isrc/activity
                                  LIBS = -lpthread -licore -ljson -lmysqlclient
                            OPTIM_FLAG = -O3 -g
                                   CPP = g++
                                LFLAGS = -Wl -fPIC
                              LIB_DIRS = -L../libs
                                 VPATH = src ../common ../common/db src/player src/scene src/fight \
																				 src/task src/monster src/http src/item src/chat src/bf src/trade \
																				 src/scp src/activity
                            OBJECT_DIR = ./.obj/
                              CPPFILES = \
																				 async_task.cpp \
																				 mysql_db.cpp \
																				 daemon.cpp \
																				 load_json.cpp \
																				 client.cpp \
																				 sys.cpp \
																				 sys_log.cpp \
																				 time_util.cpp \
																				 db_proxy.cpp \
																				 db_proxy_logic.cpp \
																				 db_session_mgr.cpp \
																				 preload_db.cpp \
																				 svc_config.cpp \
																				 svc_monitor.cpp \
																				 all_char_info.cpp \
																				 global_timer.cpp \
																				 behavior_log.cpp \
																				 global_param_cfg.cpp \
																				 basic_attr_cfg.cpp \
																				 zhan_li_calc_cfg.cpp \
																				 equip_score_cfg.cpp \
																				 lvl_param_cfg.cpp \
																				 scene_config.cpp \
																				 scene_monster_cfg.cpp \
																				 exit_transfer_cfg.cpp \
																				 scene_unit.cpp \
																				 scene_obj.cpp \
																				 scene_mgr.cpp \
																				 aev_dispatch.cpp \
																				 sev_dispatch.cpp \
																				 obj_attr.cpp \
																				 char_obj.cpp \
																				 use_item.cpp \
																				 use_skill.cpp \
																				 skill_config.cpp \
																				 passive_skill_cfg.cpp \
																				 passive_skill_upgrade_cfg.cpp \
																				 buff_config.cpp \
																				 buff_module.cpp \
																				 buff_obj.cpp \
																				 fighter_mgr.cpp \
																				 player_obj.cpp \
																				 player_mgr.cpp \
																				 login_module.cpp \
																				 basic_module.cpp \
																				 skill_module.cpp \
																				 passive_skill_module.cpp \
																				 attr_module.cpp \
																				 equip_module.cpp \
																				 vip_module.cpp \
																				 vip_config.cpp \
																				 mall_module.cpp \
																				 baowu_mall_config.cpp \
																				 item_extra_info_opt.cpp \
																				 sync_ev_dispatch.cpp \
																				 async_ev_dispatch.cpp \
																				 player_snap.cpp \
																				 char_msg_queue_module.cpp \
																				 money_opt.cpp \
																				 gm_cmd.cpp \
																				 web_service.cpp \
																				 http_srv.cpp \
																				 social_module.cpp \
																				 team_mgr.cpp \
																				 team_module.cpp \
																				 monster_cfg.cpp \
																				 monster_drop_cfg.cpp \
																				 monster_mgr.cpp \
																				 monster_obj.cpp \
																				 movable_monster_obj.cpp \
																				 fighting_monster_obj.cpp \
																				 zhuzai_fen_shen_mst.cpp \
																				 player_copy_mst.cpp \
																				 ghz_wang_zuo_mst.cpp \
																				 ghz_shou_wei_mst.cpp \
																				 tu_teng_mst.cpp \
																				 monster_ai.cpp \
																				 spawn_monster.cpp \
																				 monster_pos_cfg.cpp \
																				 group_monster.cpp \
																				 wild_boss_config.cpp \
																				 wild_boss_refresh.cpp \
																				 timing_mst_config.cpp \
																				 timing_mst_module.cpp \
																				 item_config.cpp \
																				 gift_config.cpp \
																				 item_effect_def.cpp \
																				 dropped_item.cpp \
																				 dropped_item_mgr.cpp \
																				 effect_obj.cpp \
																				 effect_obj_mgr.cpp \
																				 iu_config.cpp \
																				 iu_module.cpp \
																				 package_module.cpp \
																				 task_config.cpp \
																				 task_ai.cpp \
																				 task_module.cpp \
																				 chat_module.cpp \
																				 channel.cpp \
																				 channel_mgr.cpp \
																				 mail_config.cpp \
																				 mail_module.cpp \
																				 trade_module.cpp \
																				 shop_config.cpp \
																				 npc_trade.cpp \
																				 notice_module.cpp \
																				 battle_notice_module.cpp \
																				 guild_config.cpp \
																				 guild_module.cpp \
																				 market_module.cpp \
																				 rank_module.cpp \
																				 rank_data.cpp \
																				 scp_module.cpp \
																				 scp_config.cpp \
																				 scp_mgr.cpp \
																				 scp_obj.cpp \
																				 sub_scp_obj.cpp \
																				 guild_zhu_di_scp.cpp \
																				 clt_impl_scp.cpp \
																				 xszc_scp.cpp \
																				 tui_tu_config.cpp \
																				 tui_tu_module.cpp \
																				 mis_award_cfg.cpp \
																				 mis_award_module.cpp \
																				 zhan_xing_cfg.cpp \
																				 zhan_xing_module.cpp \
																				 cheng_jiu_cfg.cpp \
																				 cheng_jiu_module.cpp \
																				 title_cfg.cpp \
																				 title_module.cpp \
																				 cache_module.cpp \
																				 bao_shi_module.cpp \
																				 bao_shi_cfg.cpp \
																				 forbid_opt_module.cpp \
																				 gm_notice_module.cpp \
																				 activity_module.cpp \
																				 activity_mgr.cpp \
																				 activity_obj.cpp \
																				 activity_cfg.cpp \
																				 award_ti_li_act.cpp \
																				 month_card_award_act.cpp \
																				 ghz_activity_obj.cpp \
																				 ghz_module.cpp \
																				 xszc_activity_obj.cpp \
																				 goods_award_act.cpp \
																				 kai_fu_act_obj.cpp \
																				 kai_fu_act_cfg.cpp \
																				 ltime_act_obj.cpp \
																				 ltime_act_mgr.cpp \
																				 ltime_act_cfg.cpp \
																				 ltime_act_module.cpp \
																				 lucky_turn_act.cpp \
																				 lucky_turn_cfg.cpp \
																				 lucky_turn_module.cpp \
																				 lucky_egg_module.cpp \
																				 huo_yue_du_cfg.cpp \
																				 huo_yue_du_module.cpp \
																				 jing_ji_module.cpp \
																				 jing_ji_scp.cpp \
																				 jing_ji_cfg.cpp \
																				 name_storage_cfg.cpp \
																				 recharge_config.cpp \
																				 recharge_module.cpp \
																				 cross_svc.cpp \
																				 product_cfg.cpp \
																				 produce_module.cpp \
																				 cross_module.cpp \
																				 mobai_module.cpp \
																				 guild_scp_mst.cpp \
																				 guild_scp.cpp \
																				 water_tree_cfg.cpp \
																				 water_tree_module.cpp \
																				 tianfu_skill_module.cpp \
																				 tianfu_skill_upgrade_cfg.cpp \
																				 tianfu_skill_cfg.cpp \
																				 suit_config.cpp \
																				 dxc_module.cpp \
																				 dxc_config.cpp \
																				 lue_duo_module.cpp \
																				 lue_duo_scp.cpp \
																				 fa_bao_config.cpp \
																				 fa_bao_module.cpp \
																				 strength_addition_cfg.cpp \
																				 goods_award_module.cpp \
																				 lucky_goods_turn_cfg.cpp \
																				 lucky_goods_draw_cfg.cpp \
																				 lue_duo_config.cpp \
																				 world.cpp

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
	@echo -en "\\033[0;33m"
	$(Q)more README
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

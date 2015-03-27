-- 新加的放后面: 保证执行顺序和添加顺序一致
-- <author> <date> <idx>
-- sparkcui 02-14
-- alter table char_info add sheng_ming int not null default 0 after coin;
-- alter table char_info add fa_li int not null default 0 after sheng_ming;

-- michaelwang 02-20
-- drop table if exists mail;
-- create table mail (
--   id                  int not null auto_increment primary key,# 邮件ID
--   char_id             int not null default 0,                 # 收件人ID
--   sender_id           int not null default 0,                 # 发件人ID
--   sender_name         varchar(30) not null default '',        # 发件人名字
--   title               varchar(60) not null default '',        # 邮件主题
--   send_time           int not null default 0,                 # 邮件发送时间
--   readed              tinyint not null default 0,             # 已读/未读      1/0
--   attach_num          tinyint not null default 0,             # 附件个数       n/0
--   mail_type           tinyint not null default 0,             # 系统邮件/私人  1/0
--   coin                int not null default 0,                 # 携带的金币
--   diamond             int not null default 0,                 # 携带的钻石
--   content             text not null default '',               # 邮件内容
--   items               varchar(6000) not null default '',      # 携带物品
--   delete_time         int not null default 0,                 # 邮件删除时间
--   lvl_limit           int not null default 0,                 # 等级限制
--   key i_mail(`char_id`, `delete_time`, `send_time`)
-- )engine=InnoDB;

-- cuissw 02-21
-- alter table char_info add zhan_li int not null default 0 after fa_li;

-- cuissw 02-22
-- create table char_extra_info (
--   char_id             int not null primary key,               # 玩家ID
--   yes_or_no           int not null default 0,                 # 保存一些是否状态
--   check_sys_mail_time int not null default 0                  # 提取系统邮件的时间
-- )engine=InnoDB;
-- create table daily_clear_info (
--   char_id             int not null primary key,               # 玩家ID
--   clean_time          int not null default 0                  # 清零的时间
-- )engine=InnoDB;
-- insert into daily_clean_info(char_id, clean_time)(select char_id, 0 from char_info);

-- cuisw 02-27
-- alter table item add attr_float_coe smallint not null default 0 after amount;

-- cuisw 03-06
-- create table passive_skill(
--   char_id             int not null,                           # 玩家ID
--   cid                 int not null default 0,                 # 技能cid
--   lvl                 smallint not null default 1,            # 技能级别
--   up_cnt              int not null default 0,                 # 升级次数
--   bless               int not null default 0,                 # 祝福值
--   primary key (`char_id`, `cid`)
-- )engine=InnoDB;

-- cuisw 03-08
-- alter table char_info add pk_mode tinyint not null default 0 after scene_cid;
-- alter table char_info add sin_val smallint not null default 0 after pk_mode;
-- alter table char_info add sin_ol_time int not null default 0 after sin_val;

-- cuisw 03-11
-- alter table item add score int not null default 0 after amount;

-- sevenwu 03-19
-- alter table daily_clean_info add ti_li smallint not null default 0 after clean_time;
-- alter table daily_clean_info add ti_li_time int not null default 0 after ti_li;

-- cuisw 03-23
-- drop table if exists guild;
-- create table guild (
--   guild_id            int not null primary key,               # 公会id
--   name                varchar(30) not null default '',        # 公会名称
--   chairman_id         int not null default 0,                 #
--   lvl                 tinyint not null default 1,             # 等级
--   purpose             varchar(450) default '',                # 宗旨
--   purpose_mtime       int not null default 0,                 # 宗旨上一次修改时间
--   c_time              int not null default 0                  # 创建时间
-- )engine=InnoDB;

-- michaelwang 03-24
-- drop table if exists guild_member;
-- create table guild_member (
--   char_id             int not null primary key,               # 玩家id
--   guild_id            int not null default 0,                 # 公会id
--   contrib             int not null default 0,                 # 贡献
--   postion             tinyint not null default 0              # 成员职位
-- )engine=InnoDB;
-- drop table if exists guild_apply;
-- create table guild_apply (
--   guild_id            int not null default 0,                 # 公会id
--   char_id             int not null default 0,                 # 申请者id
--   apply_time          int not null default 0,                 # 申请时间
--   primary key(`guild_id`,`char_id`)
-- )engine=InnoDB;

-- michaelwang 03-25
-- alter table guild_member change column postion position tinyint not null default 0;

-- michaelwang 03-25
-- alter table guild_member add total_contrib int not null default 0 after contrib;

-- michaelwang 03-27
-- alter table guild add apply_dj_limit smallint not null default 0 after c_time;
-- alter table guild add apply_zl_limit int not null default 0 after apply_dj_limit;

-- michaelwang 03-28
-- alter table guild add guild_resource int not null default 0 after apply_dj_limit;

-- sevenwu 03-31
-- drop table if exists market;
-- create table market (
--   market_id           int not null primary key,               # 拍卖行商品id
--   char_id             int not null default 0,                 # 玩家id
--   price               int not null default 0,                 # 商品价格
--   price_type          tinyint not null default 0,             # 价格类型
--   begin_time          int not null default 0,                 # 出售时间
--   cid                 int not null default 0,                 # 物品cid
--   amount              int not null default 0,                 # 物品数量
--   extra_info          varchar(1024) not null default ''            # 装备额外信息
-- )engine=InnoDB;

-- cuisw 03-31
-- drop table if exists mall_buy_log;
-- create table mall_buy_log (
--   char_id             int not null default 0,                 # 玩家id
--   npc_cid             int not null default 0,                 #
--   item_cid            int not null default 0,                 # 购买道具
--   amount              int not null default 1,                 # 购买数量
--   buy_time            int not null default 0,                 # 购买时间
--   key i_select(`char_id`, `buy_time`)
-- )engine=InnoDB;

-- michaelwang 04-02
-- alter table guild drop column purpose_mtime;

-- michaelwang 04-08
-- alter table guild add zq_lvl tinyint not null default 1 after guild_resource;
-- alter table guild add tld_lvl tinyint not null default 1 after zq_lvl;
-- alter table guild add jdb_lvl tinyint not null default 1 after tld_lvl;
-- alter table daily_clean_info add guild_jb_cnt smallint not null default 0 after ti_li;

-- michaelwang 04-09
-- alter table daily_clean_info add guild_cost_jb_cnt tinyint not null default 0 after guild_jb_cnt;
-- alter table daily_clean_info change column guild_jb_cnt guild_free_jb_cnt tinyint not null default 0;

-- cuisw 04-10
-- alter table char_info add last_coord_x smallint not null default 0 after scene_cid;
-- alter table char_info add last_coord_y smallint not null default 0 after last_coord_x;
-- alter table char_info add last_scene_cid int not null default 0 after last_coord_y;

-- michaelwang 04-10
-- alter table guild change column jdb_lvl jbd_lvl tinyint not null default 1;
-- alter table guild add last_summon_boss_time int not null default 0 after jbd_lvl;

-- cliffordsun 04-11
-- create table char_attr (
--   char_id             int not null primary key,               # 玩家id
--   sheng_ming          int not null default 0,                 # 生命
--   fa_li               int not null default 0,                 # 法力
--   gong_ji             int not null default 0,                 # 攻击
--   fang_yu             int not null default 0,                 # 防御
--   ming_zhong          int not null default 0,                 # 命中
--   shan_bi             int not null default 0,                 # 闪避
--   bao_ji              int not null default 0,                 # 暴击
--   kang_bao            int not null default 0,                 # 抗暴
--   shang_mian          int not null default 0                  # 伤免
-- )engine=InnoDB;

-- sevenwu 04-11
-- alter table market add score int not null default 0 after amount;
-- alter table market add attr_float_coe smallint not null default 0 after score;

-- michaelwang 04-14
-- alter table daily_clean_info add guild_promote_skill_cnt smallint not null default 0 after guild_cost_jb_cnt;

-- michaelwang 04-15
-- create table guild_skill(
--   char_id             int not null,                           # 玩家ID
--   cid                 int not null default 0,                 # 技能cid
--   lvl                 smallint not null default 1,            # 技能级别
--   primary key (`char_id`, `cid`)
-- )engine=InnoDB;

-- cuisw 04-15
-- drop table if exists tui_tu_log;
-- create table tui_tu_log (
--   char_id             int not null default 0,                 # 玩家id
--   cid                 int not null default 0,                 # 推图CID
--   score               int not null default 0,                 # 推图CID
--   first_award         tinyint not null default 0,             # 是否领过首次通关奖励
--   state               tinyint not null default 0,             # 关卡状态
--   primary key (`char_id`, `cid`)
-- )engine=InnoDB;

-- michaelwang 04-15
-- alter table daily_clean_info add online_award_get_idx tinyint not null default 0 after guild_promote_skill_cnt;
-- alter table daily_clean_info add login_award_if_get tinyint not null default 0 after online_award_get_idx;

-- michaelwang 04-16
-- alter table daily_clean_info change column online_award_get_idx ol_award_get_idx tinyint not null default 0;

-- michaelwang 04-17
-- create table vip (
--   char_id             int not null primary key,               # 玩家id
--   vip_lvl             tinyint not null default 0,             # vip等级
--   vip_exp             int not null default 0,                 # vip当前等级经验
--   get_idxs            varchar(127) not null default ''        # 领取奖励信息
-- )engine=InnoDB;

-- sevenwu 04-17
-- alter table char_extra_info add zhan_xing_fu int not null default 0 after yes_or_no;

-- cuisw 04-18
-- alter table char_extra_info change zhan_xing_fu zx_value int not null default 0;

-- cuisw 04-19
-- drop table if exists daily_task;
-- create table daily_task (
--   char_id             int not null default 0,                 # 玩家ID
--   task_cid            int not null default 0,                 # 任务CID
--   done_cnt            int not null default 0,                 # 任务已接次数
--   primary key  (`char_id`,`task_cid`)
-- )engine=InnoDB;

-- cuisw 04-21
-- alter table daily_clean_info  add guild_task_done_cnt smallint not null default 0 after login_award_if_get;

-- michaelwang 04-21
-- alter table daily_clean_info add ol_award_start_time int not null default 0 after guild_task_done_cnt;

-- sevenwu 04-22
-- drop table if exists cheng_jiu;
-- create table cheng_jiu (
--   char_id             int not null,                           # 玩家ID
--   cj_type             int not null default 0,                 # 成就类型
--   id                  int not null default 0,                 # 成就id
--   value               int not null default 0,                 # 成就数值
--   primary key  (`char_id`,`cj_type`)
-- )engine=InnoDB;

-- cuisw 04-23
-- alter table char_info add ol_time int not null default 0 after storage_cap;
-- alter table daily_clean_info add con_login_days int not null default 0 after ol_award_start_time;

-- michaelwang 04-24
-- create table bao_shi (
--   char_id             int not null default 0,                 # 玩家ID
--   pos                 tinyint not null default 0,             # 部位
--   bs_idx              tinyint not null default 0,             # 第几个
--   bs_lvl              smallint not null default 0,            # 宝石等级
--   bs_exp              int not null default 0,                 # 当前等级经验
--   primary key  (`char_id`,`pos`,`bs_idx`)
-- )engine=InnoDB;

-- sevenwu 04-25
-- alter table char_info add cur_title smallint not null default 0 after storage_cap;

-- cuisw 04-28
-- alter table tui_tu_log add free_turn_cnt tinyint not null default 0 after state;
-- drop table if exists sys_settings;
-- create table sys_settings (
--   account             varchar(63) not null primary key,       # 玩家所属账号
--   data                varchar(512) not null default ''        #
-- )engine=InnoDB;

-- GM 管理
-- drop table if exists forbid_opt;
-- create table forbid_opt (
--   id                  int primary key not null,               # ID
--   opt                 int not null default 0,                 # 控制类型 1:禁止帐户登陆
--                                                               #          2:禁止ip登陆
--                                                               #          3:禁止角色登陆
--                                                               #          4:禁止帐户发言
--   begin_time          int not null default 0,                 # 开始控制时间
--   end_time            int not null default 0,                 # 结束时间
--   target              varchar(63) not null default ''         # 值
-- )engine=InnoDB;

-- sevenwu 04-30
-- drop table if exists notice;
-- create table notice (
--   id                  int not null primary key,               # 公告id
--   begin_time          int not null default 0,                 # 公告开始时间
--   end_time            int not null default 0,                 # 公告结束时间
--   interval_time       int not null default 0,                 # 公告间隔时间
--   content             varchar(512) not null default ''        # 公告内容
-- )engine=InnoDB;

-- sevenwu 05-04
-- drop table if exists title;
-- create table title (
--   char_id             int not null default 0,                 # 玩家ID
--   title_cid           smallint not null default 0,            # 称号cid
--   primary key  (`char_id`, `title_cid`)
-- )engine=InnoDB;

-- sevenwu 05-04
-- drop table if exists huo_yue_du;
-- create table huo_yue_du (
--   char_id             int not null default 0,                 # 玩家ID
--   cid                 smallint not null default 0,            # 活跃度cid
--   score               int not null default 0,                 # 积分
--   primary key  (`char_id`, `cid`)
-- )engine=InnoDB;

-- sevenwu 05-05
-- alter table char_extra_info add huo_yue_score smallint not null default 0 after zx_value;

-- cuisw 05-06
-- alter table account_info drop column b_diamond;
-- alter table char_info add b_diamond int not null default 0 after coin;
-- alter table tui_tu_log drop column free_turn_cnt;

-- michaelwang 05-07
-- alter table char_extra_info add get_lvl_award_idx smallint not null default 0 after huo_yue_score;

-- sevenwu 05-07
-- alter table huo_yue_du change score score smallint not null default 0;

-- cliffordsun 05-09
-- alter table char_attr add zhu_wu int not null default 0 after shang_mian;
-- alter table char_attr add fu_wu int not null default 0 after zhu_wu;

-- cuisw 05-09
-- create table scp_log (
--   char_id             int not null default 0,                 # 玩家id
--   cid                 int not null default 0,                 # 副本CID
--   enter_cnt           tinyint not null default 0,             # 副本进入次数
--   primary key (`char_id`, `cid`)
-- )engine=InnoDB;
-- drop table if exists jing_ji_rank;
-- create table jing_ji_rank (
--   char_id             int not null primary key,               # 玩家id
--   rank                int not null default 0                  # 排名
-- )engine=InnoDB;
-- alter table daily_clean_info add jing_ji_cnt tinyint not null default 0 after con_login_days;
-- alter table daily_clean_info add free_refresh_jj_cnt tinyint not null default 0 after jing_ji_cnt;
--
-- sevenwu 05-13
-- alter table char_extra_info add jing_ji_max_rank int not null default 0 after get_lvl_award_idx;

-- cuisw 05-13
-- alter table char_info change sheng_ming hp int not null default 0;
-- alter table char_info change fa_li mp int not null default 0;
-- alter table char_attr change sheng_ming hp int not null default 0;
-- alter table char_attr change fa_li mp int not null default 0;

-- sevenwu 05-13
-- alter table char_extra_info add last_jing_ji_time int not null default 0 after jing_ji_max_rank;

-- sevenwu 05-14
-- alter table char_extra_info add jing_ji_score int not null default 0 after last_jing_ji_time;

-- sevenwu 05-15
-- drop table if exists jing_ji_log;
-- create table jing_ji_log (
--   char_id             int not null default 0,                 # 玩家id
--   competitor_id       int not null default 0,                 # 对手id
--   rank_change         int not null default 0,                 # 排名变化
--   time                int not null default 0,                 # 记录时间
--   key i_id(`char_id`, `competitor_id`)
-- )engine=InnoDB;

-- michaelwang 05-15
-- alter table vip add guild_jb_cnt tinyint not null default 0 after vip_exp;

-- sevenwu 05-15
-- alter table jing_ji_rank add award_time int not null default 0 after rank;

-- cuisw 05-16
-- alter table mail add b_diamond int not null default 0 after diamond;

-- cuisw 05-20
-- alter table guild add is_ghz_winner tinyint not null default 0 after jbd_lvl;

-- cuisw 05-25
-- alter table social drop column rel_value;

-- cuisw 05-29
-- alter table service_info add ghz_closed_time int not null default 0 after opened;

-- cuisw 06-05
-- alter table tui_tu_log change score used_time int not null default 0;

-- cuisw 06-13
-- drop table if exists limit_time_act;
-- create table limit_time_act (
--   act_id             int not null primary key,                # 活动id
--   begin_time         datetime not null,                       #
--   end_time           datetime not null                        #
-- )engine=InnoDB;

-- michaelwang 06-16
-- alter table bao_shi add up_cnt smallint not null default 0 after bs_lvl;

-- cuisw 06-17
-- alter table char_extra_info add got_ti_li_award_time int not null default 0 after jing_ji_score;

-- cliffordsun 06-18
-- alter table char_extra_info add last_buy_mc_time int not null default 0 after check_sys_mail_time;
-- alter table char_extra_info add recharge_rebate int not null default 0 after last_buy_mc_time;
-- alter table daily_clean_info add if_mc_rebate tinyint not null default 0 after ti_li_time;

-- cliffordsun 06-20
-- alter table char_extra_info drop last_buy_mc_time;
-- alter table char_extra_info drop recharge_rebate;
-- create table char_recharge (
--   char_id             int not null primary key,               # 玩家ID
--   last_buy_mc_time    int not null default 0,                 # 上次购买月卡时间
--   data                varchar(127) not null default "{}"      # 购买信息
-- )engine=InnoDB;

-- cuisw 06-21
-- drop table if exists limit_time_act;
-- drop table if exists ltime_act;
-- create table ltime_act (
--   act_id             int not null primary key,                # 活动id
--   begin_time         datetime not null,                       #
--   end_time           datetime not null                        #
-- )engine=InnoDB;

-- 幸运转盘
-- drop table if exists lucky_turn;
-- create table lucky_turn (
--   acc_award           int not null default 0                  #
-- )engine=InnoDB;
-- insert into lucky_turn values(0);

-- 幸运转盘-获奖记录
-- drop table if exists lucky_turn_big_award;
-- create table lucky_turn_big_award (
--   char_id             int not null default 0,                 # 玩家id
--   award_time          int not null default 0,                 # 获奖时间
--   award_v             int not null default 0                  # 获奖数额
-- )engine=InnoDB;

-- 幸运转盘-角色积分
-- drop table if exists lucky_turn_score;
-- create table lucky_turn_score (
--   char_id             int not null primary key,               # 玩家id
--   score               int not null default 0,                 # 积分
--   turn_time           int not null default 0                  # 转盘时间
-- )engine=InnoDB;

-- cuisw 06-25
-- alter table daily_clean_info drop column guild_task_done_cnt;
-- drop table if exists daily_task;

-- cuisw 06-26
-- alter table daily_clean_info  add daily_task_done_cnt smallint not null default 0 after login_award_if_get;
-- alter table daily_clean_info  add guild_task_done_cnt smallint not null default 0 after daily_task_done_cnt;
--
-- alter table item drop column score;
-- alter table market drop column score;
-- drop table if exists ltime_recharge_award;
-- create table ltime_recharge_award (
--   account             varchar(63) not null default '',        # 玩家所属账号
--   act_id              int not null default 0,                 # 限时活动ID
--   sub_id              int not null default 0,                 # 限时活动子ID
--   award_time          int not null default 0,                 # 领取时间
--   primary key  (`account`,`act_id`,`sub_id`)
-- )engine=InnoDB;

-- 限时充值领奖记录
-- drop table if exists ltime_recharge_award;
-- create table ltime_recharge_award (
--   char_id             int not null default 0,                 #
--   act_id              int not null default 0,                 # 限时活动ID
--   sub_id              int not null default 0,                 # 限时活动子ID
--   award_time          int not null default 0,                 # 领取时间
--   primary key  (`char_id`,`act_id`,`sub_id`)
-- )engine=InnoDB;

-- 充值记录
-- drop table if exists recharge_log;
-- create table recharge_log (
--   char_id             int not null default 0,                 # 充值角色
--   rc_time             int not null default 0,                 # 充值时间
--   value               int not null default 0                  # 单笔充值数额
-- )engine=InnoDB;

-- 消费记录
-- drop table if exists consume_log;
-- create table consume_log (
--   char_id             int not null default 0,                 #
--   cs_type             int not null default 0,                 # 消费类型
--   cs_time             int not null default 0,                 # 消费时间
--   value               int not null default 0                  # 单笔消费数额
-- )engine=InnoDB;
--
-- alter table account_info add char_id int not null default 0 after  diamond;

-- drop table if exists sys_settings;
-- create table sys_settings (
--   char_id             int not null primary key,               #
--   data                varchar(512) not null default ''        #
-- )engine=InnoDB;

-- michaelwang 07-18
-- alter table char_extra_info add seven_day_login varchar(7) not null default '' after got_ti_li_award_time;

-- cuisw 07-18
-- alter table account_info add track_st tinyint not null default 0 after  account;

-- cuisw 07-22
-- alter table account_info add channel varchar(30) not null  after  char_id;
-- alter table account_info add ip varchar(15) not null  after  channel;

-- michaelwang 07-29
-- alter table daily_clean_info add worship_cnt smallint not null default 0 after if_mc_rebate;
-- alter table service_info add worship_chairman int not null default 0 after ghz_closed_time;
-- alter table service_info add worship_left_coin int not null default 0 after worship_chairman;
-- alter table service_info add worship_acc_coin int not null default 0 after worship_left_coin;
-- alter table service_info add acc_worship_cnt smallint not null default 0 after worship_acc_coin;
-- alter table char_extra_info add worship_time int not null default 0 after check_sys_mail_time;

-- michaelwang 07-31
-- update char_extra_info set worship_time = 0;

-- michaelwang 08-04
-- drop table if exists worship_info;
-- create table worship_info (
--   chairman            int not null default 0,                 # 被崇拜公会会长
--   left_coin           int not null default 0,                 # 被崇拜王族可领金币
--   acc_coin            int not null default 0,                 # 被崇拜王族累积金币
--   acc_worship_cnt     int not null default 0                  # 王族被崇拜次数
-- )engine=InnoDB;
-- insert into worship_info values(0,0,0,0);
-- 
-- alter table service_info drop column worship_chairman;
-- alter table service_info drop column worship_left_coin;
-- alter table service_info drop column worship_acc_coin;
-- alter table service_info drop column acc_worship_cnt

-- michaelwang 08-05
-- alter table worship_info drop column chairman;

-- cuisw 08-05
-- alter table item change column extra_info extra_info varchar(512) not null default '';

-- michaelwang 08-05
-- alter table guild add scp_lvl tinyint not null default 1 after apply_zl_limit;
-- alter table guild add scp_opened tinyint not null default 0 after scp_lvl;

-- michaelwang 08-06
-- alter table guild change column scp_opened scp_last_open_time int not null default 0;

-- sevenwu 08-06
-- alter table char_extra_info add water_tree_time int not null default 0 after check_sys_mail_time;

-- sevenwu 08-06
-- drop table if exists water_tree;
-- create table water_tree (
--   lvl                 int not null default 0,                 # 树木等级
--   cheng_zhang         int not null default 0                  # 树木成长
-- )engine=InnoDB;
-- insert into water_tree values(1,0);

-- cuisw 08-07
-- alter table account_info drop column ip;
-- alter table char_info add ip varchar(15) not null default '' after cur_title;

-- sevenwu 08-07
-- alter table char_extra_info add ti_li smallint not null default 0 after got_ti_li_award_time;
-- alter table char_extra_info add ti_li_time int not null default 0 after ti_li;
-- update char_extra_info ce set ti_li = (select ti_li from daily_clean_info dc where ce.char_id = dc.char_id);
-- alter table daily_clean_info drop column ti_li;
-- alter table daily_clean_info drop column ti_li_time;
-- alter table huo_yue_du add get_award tinyint not null default 0 after score;

-- cliffordsun 08-07
-- create table tianfu_skill(
--   char_id             int not null,                           # 玩家ID
--   cid                 int not null default 0,                 # 技能cid
--   lvl                 smallint not null default 1,            # 技能级别
--   exp                 int not null default 0,                 # 经验值
--   primary key (`char_id`, `cid`)
-- )engine=InnoDB;
 
-- cuisw 08-14
-- alter table vip add buy_ti_li_cnt tinyint not null default 0 after guild_jb_cnt;

-- cuisw 08-20
-- alter table vip add exchange_coin_cnt tinyint not null default 0 after buy_ti_li_cnt;
-- alter table scp_log add enter_time int not null default 0 after enter_cnt;

-- ciffordsun 08-26
-- alter table daily_clean_info add dxc_normal_cnt tinyint not null default 0 after worship_cnt;
-- alter table daily_clean_info add dxc_hard_cnt tinyint not null default 0 after dxc_normal_cnt;
-- 
-- -- ciffordsun 08-27
-- alter table char_extra_info add dxc_normal_scene int not null default 0 after worship_time;
-- alter table char_extra_info add dxc_hard_scene int not null default 0 after dxc_normal_scene;
-- 
-- -- ciffordsun 08-27
-- alter table char_extra_info add jing_li smallint not null default 0 after dxc_hard_scene;
-- alter table char_extra_info add jing_li_time int not null default 0 after jing_li;

-- sevenwu 09-04
-- alter table char_extra_info add xszc_honor int not null default 0 after jing_li_time;

-- ciffordsun 09-05
-- drop table if exists lueduo_item;
-- create table lueduo_item (
--   char_id             int not null default 0,                 # 玩家ID
--   cid                 int not null default 0,                 # 物品cid
--   bind_amount         int not null default 0,                 #
--   unbind_amount       int not null default 0,                 #
--   primary key (`char_id`, `cid`)
-- )engine=InnoDB;
-- create table lueduo_log (
--   char_id             int not null default 0,                 # 玩家ID
--   robber              int not null default 0,                 # 强盗ID
--   time                int not null default 0,                 # 抢夺时间
--   material            int not null default 0,                 # 抢夺材料
--   amount              int not null default 0,                 # 抢夺数量
--   result              tinyint not null default 0,             # 是否成功
--   if_revenge          tinyint not null default 0,             # 是否已复仇
--   primary key (`char_id`, `robber`, `time`)
-- )engine=InnoDB;

-- cuisw 09-11
-- alter table char_extra_info add fa_bao_dj tinyint not null default 0 after jing_li_time;
-- alter table char_extra_info add fa_bao varchar(16) not null default '' after fa_bao_dj;

-- cliffordsun 09-16
-- alter table char_extra_info drop COLUMN dxc_normal_scene;
-- alter table char_extra_info drop COLUMN dxc_hard_scene;
-- alter table daily_clean_info drop COLUMN dxc_normal_cnt;
-- alter table daily_clean_info drop COLUMN dxc_hard_cnt;
-- alter table daily_clean_info add dxc_enter_cnt tinyint not null default 0 after worship_cnt;
-- create table dxc_info (
--   char_id             int not null default 0,                 # 玩家ID
--   scene_cid           int not null default 0,                 # 场景id
--   grade               tinyint not null default 0,             # 通关难度等级
--   primary key (`char_id`, `scene_cid`)
-- )engine=InnoDB;

-- cuisw 09-18
-- alter table char_extra_info add wt_goods_cnt tinyint not null default 0 after fa_bao;
-- alter table char_extra_info add wx_shared_goods_cnt tinyint not null default 0 after wt_goods_cnt;

-- cuisw 09-22
-- alter table daily_clean_info add goods_lucky_turn_cnt tinyint not null default 0 after worship_cnt;
-- alter table daily_clean_info add daily_wx_shared tinyint not null default 0 after goods_lucky_turn_cnt;
-- alter table daily_clean_info add if_got_goods_lucky_draw tinyint not null default 0 after daily_wx_shared;
-- alter table char_extra_info drop column wx_shared_goods_cnt;
-- alter table char_extra_info add lucky_turn_goods_cnt tinyint not null default 0 after wt_goods_cnt;
-- alter table char_extra_info add daily_goods_lucky_draw_cnt tinyint not null default 0 after lucky_turn_goods_cnt;

-- cuisw 09-23
-- alter table daily_clean_info add free_relive_cnt tinyint not null default 0 after if_got_goods_lucky_draw;
-- alter table vip add free_relive_cnt tinyint not null default 0 after exchange_coin_cnt;

-- sevenwu 09-25
-- drop table if exists baowu_mall;
-- create table baowu_mall (
--   char_id             int not null primary key,               # 玩家ID
--   refresh_time        int not null default 0,                 # 场景id
--   item_info           varchar(128) not null default ''        # 随机物品信息
-- )engine=InnoDB;

-- michaelwang 10-14
-- alter table char_extra_info add guild_scp_time int not null default 0 after xszc_honor;

-- cuisw 10-21
alter table char_extra_info add total_mstar int not null default 0 after xszc_honor;

-- cuisw 11-03
drop table if exists kai_fu_act_log;
create table kai_fu_act_log (
  char_id             int not null default 0,                 # 玩家ID
  act_type            int not null default 0,                 # 活动类型
  value               int not null default 0,                 # 单条活动数值
  index i_act(`char_id`)
)engine=InnoDB;

-- -- -- rc
-- dont delete these sql
delete from mail where send_time < (UNIX_TIMESTAMP() - 15*86400);
delete from mall_buy_log where buy_time < (UNIX_TIMESTAMP() - 7*86400);

-- tinyint     0~127
-- smallint    0~32767
-- int         0~2187483647
-- bigint
-- engine=InnoDB  or  engine=MyISAM
-- account len = 63
-- name len    = 30

/*##################################### 帐户表 ##################################*/
-- 帐户信息
drop table if exists account_info;
create table account_info (
  account             varchar(63) not null primary key,       # 玩家所属账号
  track_st            tinyint not null default 0,             # 帐号状态追踪
  diamond             int not null default 0,                 # 钻石数
  char_id             int not null default 0,                 # 上次登陆的角色ID
  channel             varchar(30) not null,                   # 渠道
  c_time              int not null default 0                  # 创建时间
)engine=InnoDB;

/*##################################### 角色表 ##################################*/
-- 角色基本信息
drop table if exists char_info;
create table char_info (
  char_id             int not null primary key,               # 玩家ID
  account             varchar(63) not null,                   # 玩家所属账号
  name                varchar(30) not null,                   # 玩家名字

  career              tinyint not null,                       # 职业

  exp                 bigint not null default 0,              # 当前经验值
  lvl                 smallint not null default 1,            # 玩家级别

  coin                int not null default 0,                 # 金币
  b_diamond           int not null default 0,                 # 绑定钻石数
  hp                  int not null default 0,                 #
  mp                  int not null default 0,                 #
  zhan_li             int not null default 0,                 # 战力

  dir                 tinyint not null default 5,             # 当前朝向
  coord_x             smallint not null default 0,            # 当前所在位置X坐标
  coord_y             smallint not null default 0,            # 当前所在位置Y坐标
  scene_id            int not null default 0,                 # 玩家当前所在副本ID
  scene_cid           int not null default 0,                 # 玩家当前所在场景
  last_coord_x        smallint not null default 0,            # 上次所在位置X坐标
  last_coord_y        smallint not null default 0,            # 上次所在位置Y坐标
  last_scene_cid      int not null default 0,                 # 玩家上次所在场景
  pk_mode             tinyint not null default 0,             #
  sin_val             smallint not null default 0,            # 罪恶值
  sin_ol_time         int not null default 0,                 # 罪恶值累积在线时长

  pkg_cap             smallint not null default 0,            # 背包容量
  storage_cap         smallint not null default 0,            # 仓库容量
  cur_title           smallint not null default 0,            # 当前使用称号

  ip                  varchar(15) not null default '',        # IP
  ol_time             int not null default 0,                 # 在线时长
  c_time              int not null default 0,                 # 角色创建时间
  in_time             int not null default 0,                 # 登陆时间
  out_time            int not null default 0,                 # 登出时间

  index i_account(`account`),
  index i_name(`name`)
)engine=InnoDB;

-- 系统设置
drop table if exists sys_settings;
create table sys_settings (
  char_id             int not null primary key,               #
  data                varchar(512) not null default ''        #
)engine=InnoDB;

-- 角色额外信息
drop table if exists char_extra_info;
create table char_extra_info (
  char_id             int not null primary key,               # 玩家ID
  yes_or_no           int not null default 0,                 # 保存一些是否状态
  zx_value            int not null default 0,                 # 占星点数
  huo_yue_score       smallint not null default 0,            # 活跃度积分
  get_lvl_award_idx   smallint not null default 0,            # 等级奖励
  jing_ji_max_rank    int not null default 0,                 # 竞技场最好排名
  last_jing_ji_time   int not null default 0,                 # 上次竞技场挑战时间
  jing_ji_score       int not null default 0,                 # 竞技场积分
  got_ti_li_award_time int not null default 0,                # 领取体力值奖励时间
  ti_li               smallint not null default 0,            # 体力值
  ti_li_time          int not null default 0,                 # 体力更新时间
  seven_day_login     varchar(7) not null default '',         # 七天登陆奖励
  check_sys_mail_time int not null default 0,                 # 提取系统邮件的时间
  water_tree_time     int not null default 0,                 # 浇树时间
  worship_time        int not null default 0,                 # 崇拜王族时间
  jing_li             smallint not null default 0,            # 精力值
  jing_li_time        int not null default 0,                 # 精力恢复时间
  fa_bao_dj           tinyint not null default 0,             # 法宝等阶
  fa_bao              varchar(15) not null default '',        # 法宝装备信息
  wt_goods_cnt        tinyint not null default 0,             # 灌溉中获得实物奖励个数
  lucky_turn_goods_cnt tinyint not null default 0,            # 微信分享中获得实物奖励个数
  daily_goods_lucky_draw_cnt tinyint not null default 0,      # 日常抽奖次数(实物奖励)
  xszc_honor          int not null default 0,                 # 雄狮战场荣誉
  total_mstar         int not null default 0,                 # 关卡星级总和
  guild_scp_time      int not null default 0                  # 公会副本时间
)engine=InnoDB;

-- 角色每日要清零的信息
drop table if exists daily_clean_info;
create table daily_clean_info (
  char_id             int not null primary key,               # 玩家ID
  clean_time          int not null default 0,                 # 清零的时间
  guild_free_jb_cnt   tinyint not null default 0,             # 玩家当日免费公会聚宝次数
  guild_cost_jb_cnt   tinyint not null default 0,             # 玩家当日花钱公会聚宝次数
  guild_promote_skill_cnt smallint not null default 0,        # 玩家当日提升公会技能次数
  ol_award_get_idx    tinyint not null default 0,             # 当日领取在线奖励索引
  login_award_if_get  tinyint not null default 0,             # 当日是否领取登陆奖励
  daily_task_done_cnt smallint not null default 0,            # 日常任务已完成次数
  guild_task_done_cnt smallint not null default 0,            # 公会任务已完成次数
  ol_award_start_time int not null default 0,                 # 开始时间
  con_login_days      int not null default 0,                 # 连续登陆天数
  jing_ji_cnt         tinyint not null default 0,             # 竞技次数
  free_refresh_jj_cnt tinyint not null default 0,             # 免费刷新竞技次数
  if_mc_rebate        tinyint not null default 0,             # 今日是否领取过月卡返还
  worship_cnt         smallint not null default 0,            # 今日崇拜王族次数
  goods_lucky_turn_cnt tinyint not null default 0,            # 今日实物奖励转盘次数
  daily_wx_shared     tinyint not null default 0,             # 实物奖励,每日微信分享     
  if_got_goods_lucky_draw tinyint not null default 0,         # 是否抽取日常实物抽奖
  free_relive_cnt     tinyint not null default 0,             # 免费复活次数
  dxc_enter_cnt       tinyint not null default 0              # 当日地下城挑战次数
)engine=InnoDB;

-- 角色充值信息
drop table if exists char_recharge;
create table char_recharge (
  char_id             int not null primary key,               # 玩家ID
  last_buy_mc_time    int not null default 0,                 # 上次购买月卡时间
  data                varchar(127) not null default "{}"      # 购买信息
)engine=InnoDB;

-- 消息队列
drop table if exists char_db_msg_queue;
create table char_db_msg_queue (
  char_id             int not null default 0,                 # 角色ID
  msg_id              int not null default 0,                 # 消息ID
  param               varchar(127) not null default '',       # 消息参数
  index i_mq(`char_id`)
)engine=InnoDB;

-- 物品
drop table if exists item;
create table item (
  char_id             int not null default 0,                 # 玩家ID
  id                  int not null default 0,                 # 物品ID
  pkg                 tinyint not null default 0,             # 容器类型
  cid                 int not null default 0,                 # 物品cid
  bind                tinyint not null default 0,             #
  amount              int not null default 1,                 #
  attr_float_coe      smallint not null default 0,            # 装备属性浮动系数

  extra_info          varchar(512) not null default '',       # 道具额外信息,json格式
  primary key (`char_id`, `id`)
)engine=InnoDB;

-- 人物技能
drop table if exists skill;
create table skill(
  char_id             int not null,                           # 玩家ID
  cid                 int not null default 0,                 # 技能cid
  lvl                 smallint not null default 1,            # 技能级别
  use_time            int not null default 0,                 # 技能施放时间
  primary key (`char_id`, `cid`)
)engine=InnoDB;

-- 人物被动技能
drop table if exists passive_skill;
create table passive_skill(
  char_id             int not null,                           # 玩家ID
  cid                 int not null default 0,                 # 技能cid
  lvl                 smallint not null default 1,            # 技能级别
  up_cnt              int not null default 0,                 # 升级次数
  bless               int not null default 0,                 # 祝福值
  primary key (`char_id`, `cid`)
)engine=InnoDB;

-- 人物公会技能
drop table if exists guild_skill;
create table guild_skill(
  char_id             int not null,                           # 玩家ID
  cid                 int not null default 0,                 # 技能cid
  lvl                 smallint not null default 1,            # 技能级别
  primary key (`char_id`, `cid`)
)engine=InnoDB;

-- 人物天赋技能
drop table if exists tianfu_skill;
create table tianfu_skill(
  char_id             int not null,                           # 玩家ID
  cid                 int not null default 0,                 # 技能cid
  lvl                 smallint not null default 1,            # 技能级别
  exp                 int not null default 0,                 # 经验值
  primary key (`char_id`, `cid`)
)engine=InnoDB;

-- 主角BUFF
drop table if exists buff;
create table buff(
  char_id             int not null primary key,               # 玩家ID
  data                varchar(256) not null default ''        #
)engine=InnoDB;

-- 社群
drop table if exists social;
create table social (
  char_id             int not null,                           # 玩家ID
  relation            tinyint not null default 0,             # 关系类型
  socialer_id         int not null default 0,                 # 关系人id
  index s_char(`char_id`, `relation`, `socialer_id`)
)engine=InnoDB;

-- 任务
drop table if exists task;
create table task (
  char_id             int not null,                           # 玩家ID
  task_cid            int not null,                           # 任务CID
  state               tinyint not null default 1,             #
  value_0             int not null default 0,                 #
  value_1             int not null default 0,                 #
  value_2             int not null default 0,                 #
  extra_value         int not null default 0,                 #
  accept_time         int not null default 0,                 #
  primary key  (`char_id`,`task_cid`)
)engine=InnoDB;

-- 任务位数组
drop table if exists task_bit_array;
create table task_bit_array(
  char_id             int not null primary key,               # 玩家ID
  trunk_bit           varchar(512) not null default '',       #
  branch_bit          varchar(512) not null default ''        #
)engine=InnoDB;

-- 邮件
drop table if exists mail;
create table mail (
  id                  int not null auto_increment primary key,# 邮件ID
  char_id             int not null default 0,                 # 收件人ID
  sender_id           int not null default 0,                 # 发件人ID
  sender_name         varchar(30) not null default '',        # 发件人名字
  title               varchar(60) not null default '',        # 邮件主题
  send_time           int not null default 0,                 # 邮件发送时间
  readed              tinyint not null default 0,             # 已读/未读      1/0
  attach_num          tinyint not null default 0,             # 附件个数       n/0
  mail_type           tinyint not null default 0,             # 系统邮件/私人  1/0
  coin                int not null default 0,                 # 携带的金币
  diamond             int not null default 0,                 # 携带的钻石
  b_diamond           int not null default 0,                 # 携带的绑定钻石
  content             text not null default '',               # 邮件内容
  items               varchar(1024) not null default '',      # 携带物品
  delete_time         int not null default 0,                 # 邮件删除时间
  lvl_limit           int not null default 0,                 # 等级限制
  index i_mail(`char_id`, `delete_time`),
  index i_mt(`mail_type`)   -- For `get_system_mail_list_sql'
)engine=InnoDB;

-- 玩家商城购买记录，可用于限购
drop table if exists mall_buy_log;
create table mall_buy_log (
  char_id             int not null default 0,                 # 玩家id
  npc_cid             int not null default 0,                 #
  item_cid            int not null default 0,                 # 购买道具
  amount              int not null default 1,                 # 购买数量
  buy_time            int not null default 0,                 # 购买时间
  index i_select(`char_id`, `buy_time`)
)engine=InnoDB;

-- 角色战斗属性
drop table if exists char_attr;
create table char_attr (
  char_id             int not null primary key,               # 玩家id
  hp                  int not null default 0,                 # 生命
  mp                  int not null default 0,                 # 法力
  gong_ji             int not null default 0,                 # 攻击
  fang_yu             int not null default 0,                 # 防御
  ming_zhong          int not null default 0,                 # 命中
  shan_bi             int not null default 0,                 # 闪避
  bao_ji              int not null default 0,                 # 暴击
  kang_bao            int not null default 0,                 # 抗暴
  shang_mian          int not null default 0,                 # 伤免
  zhu_wu              int not null default 0,                 # 主武
  fu_wu               int not null default 0                  # 副武
)engine=InnoDB;

-- 玩家进入副本记录
drop table if exists scp_log;
create table scp_log (
  char_id             int not null default 0,                 # 玩家id
  cid                 int not null default 0,                 # 副本CID
  enter_cnt           tinyint not null default 0,             # 副本进入次数
  enter_time          int not null default 0,                 # 进入时间
  primary key (`char_id`, `cid`)
)engine=InnoDB;

-- 玩家推图记录
drop table if exists tui_tu_log;
create table tui_tu_log (
  char_id             int not null default 0,                 # 玩家id
  cid                 int not null default 0,                 # 推图CID
  used_time           int not null default 0,                 # 推图历史最短时间
  first_award         tinyint not null default 0,             # 是否领过首次通关奖励
  state               tinyint not null default 0,             # 关卡状态
  primary key (`char_id`, `cid`)
)engine=InnoDB;

-- vip
drop table if exists vip;
create table vip (
  char_id             int not null primary key,               # 玩家id
  vip_lvl             tinyint not null default 0,             # vip等级
  vip_exp             int not null default 0,                 # vip当前等级经验
  guild_jb_cnt        tinyint not null default 0,             # 公会聚宝次数
  buy_ti_li_cnt       tinyint not null default 0,             # 购买体力值次数
  exchange_coin_cnt   tinyint not null default 0,             # 兑换金币次数
  free_relive_cnt     tinyint not null default 0,             # 免费复活次数
  get_idxs            varchar(127) not null default ''        # 领取奖励信息
)engine=InnoDB;

-- 成就
drop table if exists cheng_jiu;
create table cheng_jiu (
  char_id             int not null,                           # 玩家ID
  cj_type             int not null default 0,                 # 成就类型
  id                  int not null default 0,                 # 成就id
  value               int not null default 0,                 # 成就数值
  primary key  (`char_id`,`cj_type`)
)engine=InnoDB;

-- 宝石
drop table if exists bao_shi;
create table bao_shi (
  char_id             int not null default 0,                 # 玩家ID
  pos                 tinyint not null default 0,             # 部位
  bs_idx              tinyint not null default 0,             # 第几个
  bs_lvl              smallint not null default 0,            # 宝石等级
  up_cnt              smallint not null default 0,            # 当前等级提升次数
  bs_exp              int not null default 0,                 # 当前等级经验
  primary key  (`char_id`,`pos`,`bs_idx`)
)engine=InnoDB;

-- 称号
drop table if exists title;
create table title (
  char_id             int not null default 0,                 # 玩家ID
  title_cid           smallint not null default 0,            # 称号cid
  primary key  (`char_id`, `title_cid`)
)engine=InnoDB;

-- 活跃度
drop table if exists huo_yue_du;
create table huo_yue_du (
  char_id             int not null default 0,                 # 玩家ID
  cid                 smallint not null default 0,            # 活跃度cid
  score               smallint not null default 0,            # 积分
  get_award           tinyint not null default 0,             # 是否领取奖励
  primary key  (`char_id`, `cid`)
)engine=InnoDB;

-- 竞技挑战记录
drop table if exists jing_ji_log;
create table jing_ji_log (
  char_id             int not null default 0,                 # 玩家id
  competitor_id       int not null default 0,                 # 对手id
  rank_change         int not null default 0,                 # 排名变化
  time                int not null default 0,                 # 记录时间
  index i_id(`char_id`, `competitor_id`)
)engine=InnoDB;

-- 限时充值领奖记录
drop table if exists ltime_recharge_award;
create table ltime_recharge_award (
  char_id             int not null default 0,                 #
  act_id              int not null default 0,                 # 限时活动ID
  sub_id              int not null default 0,                 # 限时活动子ID
  award_time          int not null default 0,                 # 领取时间
  primary key  (`char_id`,`act_id`,`sub_id`)
)engine=InnoDB;

-- 充值记录
drop table if exists recharge_log;
create table recharge_log (
  char_id             int not null default 0,                 # 充值角色
  rc_time             int not null default 0,                 # 充值时间
  value               int not null default 0                  # 单笔充值数额
)engine=InnoDB;

-- 消费记录
drop table if exists consume_log;
create table consume_log (
  char_id             int not null default 0,                 #
  cs_type             int not null default 0,                 # 消费类型
  cs_time             int not null default 0,                 # 消费时间
  value               int not null default 0                  # 单笔消费数额
)engine=InnoDB;

-- 掠夺物品
drop table if exists lueduo_item;
create table lueduo_item (
  char_id             int not null default 0,                 # 玩家ID
  cid                 int not null default 0,                 # 物品cid
  bind_amount         int not null default 0,                 #
  unbind_amount       int not null default 0,                 #
  primary key (`char_id`, `cid`)
)engine=InnoDB;

-- 掠夺日志
drop table if exists lueduo_log;
create table lueduo_log (
  char_id             int not null default 0,                 # 玩家ID
  robber              int not null default 0,                 # 强盗ID
  time                int not null default 0,                 # 抢夺时间
  material            int not null default 0,                 # 抢夺材料
  amount              int not null default 0,                 # 抢夺数量
  result              tinyint not null default 0,             # 是否成功
  if_revenge          tinyint not null default 0,             # 是否已复仇
  primary key (`char_id`, `robber`, `time`)
)engine=InnoDB;

-- 地下城信息
drop table if exists dxc_info;
create table dxc_info (
  char_id             int not null default 0,                 # 玩家ID
  scene_cid           int not null default 0,                 # 场景id
  grade               tinyint not null default 0,             # 通关难度等级
  primary key (`char_id`, `scene_cid`)
)engine=InnoDB;

-- 个人随机商城
drop table if exists baowu_mall;
create table baowu_mall (
  char_id             int not null primary key,               # 玩家ID
  refresh_time        int not null default 0,                 # 场景id
  item_info           varchar(128) not null default ''        # 随机物品信息
)engine=InnoDB;

-- 开服活动达成记录
drop table if exists kai_fu_act_log;
create table kai_fu_act_log (
  char_id             int not null default 0,                 # 玩家ID
  act_type            int not null default 0,                 # 活动类型
  value               int not null default 0,                 # 单条活动数值
  index i_act(`char_id`)
)engine=InnoDB;

/*##################################### 全局表 ##################################*/
-- 本服本全局信息
drop table if exists service_info;
create table service_info (
  open_time           datetime not null,                      # 开服时间
  opened              tinyint not null default 0,
  ghz_closed_time     int not null default 0                  # 公会战结束时间
)engine=InnoDB;
insert into service_info values(now(),0,0);

-- 公会基本信息
drop table if exists guild;
create table guild (
  guild_id            int not null primary key,               # 公会id
  name                varchar(30) not null default '',        # 公会名称
  chairman_id         int not null default 0,                 # 会长
  lvl                 tinyint not null default 1,             # 大厅等级
  purpose             varchar(450) default '',                # 宗旨
  c_time              int not null default 0,                 # 创建时间
  apply_dj_limit      smallint not null default 0,            # 等级限制
  guild_resource      int not null default 0,                 # 公会资源
  zq_lvl              tinyint not null default 1,             # 战旗等级
  tld_lvl             tinyint not null default 1,             # 屠龙殿等级
  jbd_lvl             tinyint not null default 1,             # 聚宝袋等级
  is_ghz_winner       tinyint not null default 0,             # 是否是公会战胜利者
  last_summon_boss_time int not null default 0,               # 上一次召唤时间
  apply_zl_limit      int not null default 0,                 # 战力限制
  scp_lvl             tinyint not null default 1,             # 公会副本等级
  scp_last_open_time  int not null default 0                  # 开启公会副本时间
)engine=InnoDB;

-- 公会成员
drop table if exists guild_member;
create table guild_member (
  char_id             int not null primary key,               # 玩家id
  guild_id            int not null default 0,                 # 公会id
  contrib             int not null default 0,                 # 当前贡献
  total_contrib       int not null default 0,                 # 历史贡献
  position            tinyint not null default 0              # 成员职位
)engine=InnoDB;

-- 加入公会申请
drop table if exists guild_apply;
create table guild_apply (
  guild_id            int not null default 0,                 # 公会id
  char_id             int not null default 0,                 # 申请者id
  apply_time          int not null default 0,                 # 申请时间
  primary key(`guild_id`,`char_id`)
)engine=InnoDB;

-- 拍卖行信息
drop table if exists market;
create table market (
  market_id           int not null primary key,               # 拍卖行商品id
  char_id             int not null default 0,                 # 玩家id
  price               int not null default 0,                 # 商品价格
  price_type          tinyint not null default 0,             # 价格类型
  begin_time          int not null default 0,                 # 出售时间
  cid                 int not null default 0,                 # 物品cid
  amount              int not null default 0,                 # 物品数量
  attr_float_coe      smallint not null default 0,            # 装备属性浮动系数
  extra_info          varchar(256) not null default ''        # 装备额外信息
)engine=InnoDB;

-- GM 管理
drop table if exists forbid_opt;
create table forbid_opt (
  id                  int primary key not null,               # ID
  opt                 int not null default 0,                 # 控制类型 1:禁止帐户登陆
                                                              #          2:禁止ip登陆
                                                              #          3:禁止角色登陆
                                                              #          4:禁止帐户发言
  begin_time          int not null default 0,                 # 开始控制时间
  end_time            int not null default 0,                 # 结束时间
  target              varchar(63) not null default ''         # 值
)engine=InnoDB;

-- GM 公告
drop table if exists notice;
create table notice (
  id                  int not null primary key,               # 公告id
  begin_time          datetime not null,                      # 公告开始时间
  end_time            datetime not null,                      # 公告结束时间
  interval_time       int not null default 0,                 # 公告间隔时间
  content             varchar(512) not null default ''        # 公告内容
)engine=InnoDB;

-- 竞技排行
drop table if exists jing_ji_rank;
create table jing_ji_rank (
  char_id             int not null primary key,               # 玩家id
  rank                int not null default 0,                 # 排名
  award_time          int not null default 0                  # 获得奖励时间
)engine=InnoDB;

-- 限时活动
drop table if exists ltime_act;
create table ltime_act (
  act_id              int not null primary key,               # 活动id
  begin_time          datetime not null,                      #
  end_time            datetime not null                       #
)engine=InnoDB;

-- 幸运转盘
drop table if exists lucky_turn;
create table lucky_turn (
  acc_award           int not null default 0                  #
)engine=InnoDB;
insert into lucky_turn values(0);

-- 幸运转盘-获奖记录
drop table if exists lucky_turn_big_award;
create table lucky_turn_big_award (
  char_id             int not null default 0,                 # 玩家id
  award_time          int not null default 0,                 # 获奖时间
  award_v             int not null default 0                  # 获奖数额
)engine=InnoDB;

-- 幸运转盘-角色积分
drop table if exists lucky_turn_score;
create table lucky_turn_score (
  char_id             int not null primary key,               # 玩家id
  score               int not null default 0,                 # 积分
  turn_time           int not null default 0                  # 转盘时间
)engine=InnoDB;

-- 王族膜拜
drop table if exists worship_info;
create table worship_info (
  left_coin           int not null default 0,                 # 被崇拜王族可领金币
  acc_coin            int not null default 0,                 # 被崇拜王族累积金币
  acc_worship_cnt     int not null default 0                  # 王族被崇拜次数
)engine=InnoDB;
insert into worship_info values(0,0,0);

-- 浇树
drop table if exists water_tree;
create table water_tree (
  lvl                 int not null default 0,                 # 树木等级
  cheng_zhang         int not null default 0                  # 树木成长
)engine=InnoDB;
insert into water_tree values(1,0);


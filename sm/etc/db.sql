-- tinyint     0~127
-- smallint    0~32767
-- int         0~2187483647
-- bigint
-- engine=InnoDB  or  engine=MyISAM

drop table if exists gs_info;
create table gs_info (
  id                  int not null primary key,               # 游戏服务器ID
  name                varchar(128) not null default '',       # 游戏服务器名
  group_id            varchar(128) not null default '',       # 所在组
  ip                  varchar(128) not null default '',       # IP/域名
  port                int not null default 0,                 # 游戏端口
  state               int not null default 0,                 # 服务器状态
  ol_player           int not null default 0,                 # 在线人数
  open_time           int not null default 0                  # 开服时间
)engine=InnoDB;

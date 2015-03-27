// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-05-29 17:02
 */
//========================================================================

#ifndef DEF_H_
#define DEF_H_

// Lib header

#define SEQ_NO_BEGIN        20121221

class proto_head
{
public:
  proto_head(const int seq, const int id, const int re, const int len) :
    seq_(seq),
    id_(id),
    result_(re),
    len_(len)
  { }
  proto_head() : seq_(0), id_(0), result_(0), len_(0)
  { }

  void set(const int seq, const int id, const int re, const int len)
  {
    this->seq_    = seq;
    this->id_     = id;
    this->result_ = re;
    this->len_    = len;
  }
public:
  int seq_;            // seq
  int id_;             // msg id
  int result_;         // result. for response
  int len_;            // msg body length
};

//= coordinate struct define
class coord_t
{
public:
  coord_t() : x_(0), y_(0) { }
  coord_t(const short x, const short y) : x_(x), y_(y) { }
  coord_t(const coord_t &ct)
  {
    this->x_ = ct.x_;
    this->y_ = ct.y_;
  }
  void set(const short x, const short y)
  {
    this->x_ = x;
    this->y_ = y;
  }
  coord_t &operator = (const coord_t &ct)
  {
    if (this != &ct)
    {
      this->x_ = ct.x_;
      this->y_ = ct.y_;
    }
    return *this;
  }

  bool operator == (const coord_t &ct) const
  { return this->x_ == ct.x_ && this->y_ == ct.y_; }

  bool operator != (const coord_t &ct) const
  { return this->x_ != ct.x_ || this->y_ != ct.y_; }

  //
  short x_;
  short y_;
};
//= coordinate struct define
class scene_coord_t
{
public:
  scene_coord_t() : cid_(0) { }
  scene_coord_t(const int cid, const short x, const short y) :
    cid_(cid), coord_(x, y)
  { }
  scene_coord_t(const scene_coord_t &v)
  {
    this->cid_ = v.cid_;
    this->coord_ = v.coord_;
  }
  void set(const int cid, const short x, const short y)
  {
    this->cid_ = cid;
    this->coord_.set(x, y);
  }
  scene_coord_t &operator = (const scene_coord_t &v)
  {
    if (this != &v)
    {
      this->cid_ = v.cid_;
      this->coord_ = v.coord_;
    }
    return *this;
  }

  bool operator == (const scene_coord_t &v) const
  { return this->cid_ == v.cid_ && this->coord_ == v.coord_; }

  bool operator != (const scene_coord_t &v) const
  { return this->cid_ != v.cid_ || this->coord_ != v.coord_; }

  int cid_;
  coord_t coord_;
};
//= item_amount_bind struct define
class item_amount_bind_t
{
public:
  item_amount_bind_t() : bind_(0), cid_(0), amount_(0) { }
  item_amount_bind_t(const int cid, const int amt, const char bind) :
    bind_(bind), cid_(cid), amount_(amt)
  { }
  item_amount_bind_t(const item_amount_bind_t &v)
  {
    this->cid_ = v.cid_;
    this->amount_ = v.amount_;
    this->bind_ = v.bind_;
  }
  void set(const int cid, const int amt, const char bind)
  {
    this->cid_ = cid;
    this->amount_ = amt;
    this->bind_ = bind;
  }
  item_amount_bind_t &operator = (const item_amount_bind_t &v)
  {
    if (this != &v)
    {
      this->cid_ = v.cid_;
      this->amount_ = v.amount_;
      this->bind_ = v.bind_;
    }
    return *this;
  }

  bool operator == (const item_amount_bind_t &v) const
  { return this->cid_ == v.cid_ && this->amount_ == v.amount_ && this->bind_ == v.bind_; }

  bool operator != (const item_amount_bind_t &v) const
  { return this->cid_ != v.cid_ || this->amount_ != v.amount_ || this->bind_ != v.bind_; }

  char bind_;
  int cid_;
  int amount_;
};
class blog_t // behavior_log
{
public:
  blog_t(const int t, const int s1, const int s2) :
    type_(t),
    src1_(s1),
    src2_(s2)
  { }
  int type_; // behavior_id
  int src1_; //
  int src2_;
};
//= pair
template<typename T>
class pair_t
{
public:
  pair_t() { }
  pair_t(const T &t1, const T &t2) :
    first_(t1),
    second_(t2)
  { }

  pair_t(const pair_t &t)
  {
    first_  = t.first_;
    second_ = t.second_;
  }
  pair_t &operator = (const pair_t &t)
  {
    if (&t == this) return *this;

    this->first_  = t.first_;
    this->second_ = t.second_;
    return *this;
  }

  void set(const T &f, const T &s)
  {
    this->first_  = f;
    this->second_ = s;
  }
  bool operator == (const pair_t &pt) const
  { return this->first_ == pt.first_ && this->second_ == pt.second_; }

  T first_;
  T second_;
};
class itime
{
public:
  itime(const int h = 0, const int m = 0, const int s = 0) :
    hour_(h),
    min_(m),
    sec_(s)
  { }

  bool operator < (const itime &it)
  { return this->hour_ < it.hour_
    || (this->hour_ == it.hour_ && this->min_ < it.min_)
      || (this->hour_ == it.hour_ && this->min_ == it.min_ && this->sec_ < it.sec_);
  }
  bool operator > (const itime &it)
  { return this->hour_ > it.hour_
    || (this->hour_ == it.hour_ && this->min_ > it.min_)
      || (this->hour_ == it.hour_ && this->min_ == it.min_ && this->sec_ > it.sec_);
  }
  int operator - (const itime &it)
  {
    return (this->hour_ * 3600 + this->min_ * 60 + this->sec_)
      - (it.hour_ * 3600 + it.min_ * 60 + it.sec_);
  }

  int hour_;
  int min_;
  int sec_;
};
//= direction define
enum
{
  DIR_XX                        = 0,
  DIR_UP                        = 1,
  DIR_RIGHT_UP                  = 2,
  DIR_RIGHT                     = 3,
  DIR_RIGHT_DOWN                = 4,
  DIR_DOWN                      = 5,
  DIR_LEFT_DOWN                 = 6,
  DIR_LEFT                      = 7,
  DIR_LEFT_UP                   = 8
};
#define DIR_STEP(N) static short N[9][2] = {{0,0},{0,-1},{1,-1},{1,0},{1,1},\
  {0,1},{-1,1},{-1,0},{-1,-1}}
//= limits
enum
{
  MONEY_UPPER_LIMIT             = 99999999,
};
//= login type
enum
{
  LOGIN_NORMAL                  = 1,
  LOGIN_CROSS_SVC               = 2
};
//= logout type
enum
{
  LOGOUT_NORMAL                 = 1,
  LOGOUT_KICK_OUT               = 2,
  LOGOUT_CROSS_SVC              = 3,
  LOGOUT_SVC_CRASH              = 4
};
//=
enum
{
  SEX_MALE                      = 1,
  SEX_FEMALE                    = 2
};

//= career
enum
{
  CAREER_LI_LIANG               = 1,  // 力量型职业
  CAREER_MIN_JIE                = 2,  // 敏捷型职业
  CAREER_ZHI_LI                 = 3,  // 智力型职业
  CAREER_ZHONG_LI               = 4,  // 无类型职业

  CHAR_CAREER_CNT               = 3,
};
enum
{
  M_DIAMOND                     = 1,  // 钻石
  M_BIND_DIAMOND                = 2,  // 绑定钻石
  M_COIN                        = 3,  // 金币
  M_BIND_UNBIND_DIAMOND         = 4,  // 先扣绑定后扣非绑
  V_JING_JI_SCORE               = 5,  // 竞技积分
  V_XSZC_HONOR                  = 6,  // 雄狮战场
  M_DEF_CNT,
};
//= async db result
enum
{
  DB_ASYNC_OK                   = 0
};

enum OBJ_STATUS
{
  OBJ_DEAD                      = 1L << 0,  // 1
  OBJ_IN_TEAM                   = 1L << 1,  // 2
  OBJ_TEAM_LEADER               = 1L << 2,  // 4
  OBJ_RIDE_ON                   = 1L << 3,  // 8
  OBJ_FIGHTING                  = 1L << 4,  // 16

  OBJ_END
};

// buff type
enum
{
  BF_MAX_LEFT_TIME              = 100000000,
  BF_MAX_LEFT_VALUE             = 100000000,
  BF_END
};
// equip
enum
{
  PART_XXX                      = 0,
  PART_ZHU_WU                   = 1,     // 主武
  PART_FU_WU                    = 2,     // 副武
  PART_YI_FU                    = 3,     // 衣服
  PART_KU_ZI                    = 4,     // 裤子
  PART_XIE_ZI                   = 5,     // 鞋子
  PART_SHOU_TAO                 = 6,     // 手套
  PART_XIANG_LIAN               = 7,     // 项链
  PART_JIE_ZHI                  = 8,     // 戒指
  PART_END
};
enum
{
  ICOLOR_WHITE                  = 1,     // 白
  ICOLOR_GREEN                  = 2,     // 绿
  ICOLOR_BULE                   = 3,     // 蓝
  ICOLOR_PURPLE                 = 4,     // 紫
  ICOLOR_ORANGE                 = 5,     // 橙
  ICOLOR_CNT
};
enum
{
  QUALITY_PUTONG                = 1,
  QUALITY_YOUXIU                = 2,
  QUALITY_JINGLIANG             = 3,
  QUALITY_ZHUOYUE               = 4,
  QUALITY_WANMEI                = 5,
  QUALITY_SHENSHENG             = 6,
  QUALITY_CNT
};
enum
{
  PKG_EQUIP                     = 1, // 身上装备
  PKG_PACKAGE                   = 2, // 背包
  PKG_STORAGE                   = 3, // 仓库
  PKG_END                       = 9
};
enum // pk mode
{
  PK_MODE_PEACE                 = 1,     // 和平
  PK_MODE_FIGHT                 = 2      // 全体
};
enum
{
  // 物品的属性
  UNBIND_TYPE                   = 0,     // 非绑
  BIND_TYPE                     = 1,     // 绑定

  // 先找绑定再找非绑(一般用于查询和删除)
  BIND_UNBIND                   = 3,
  // 先找非绑再找绑定(一般用于查询和删除)
  UNBIND_BIND                   = 4
};
enum iattr_t
{
  ATTR_T_XXX                    = 0,
  ATTR_T_HP                     = 1,
  ATTR_T_MP                     = 2,
  ATTR_T_GONG_JI                = 3,
  ATTR_T_FANG_YU                = 4,
  ATTR_T_MING_ZHONG             = 5,
  ATTR_T_SHAN_BI                = 6,
  ATTR_T_BAO_JI                 = 7,
  ATTR_T_KANG_BAO               = 8,
  ATTR_T_SHANG_MIAN             = 9,
  ATTR_T_ITEM_CNT
};
enum
{
  NULL_CHANNEL                  = 0,
  WORLD_CHANNEL                 = 1,
  GUILD_CHANNEL                 = 2,
  TEAM_CHANNEL                  = 3,
  PRIVATE_CHANNEL               = 4,
  CHANNEL_CNT
};
enum group_t  // 分阵营、分组
{
  GROUP_1                       = 1,
  GROUP_2                       = 2,
};

#endif // DEF_H_


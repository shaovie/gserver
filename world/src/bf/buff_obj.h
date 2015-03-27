// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-01-08 10:36
 */
//========================================================================

#ifndef BUFF_OBJ_H_
#define BUFF_OBJ_H_

// Lib header
#include "singleton.h"
#include "time_value.h"

#include "error.h"
#include "obj_pool.h"
#include "buff_effect_id.h"

// Forward declarations
class char_obj;

/**
 * @class buff_obj
 * 
 * @brief
 */
class buff_obj
{
public:
  enum
  {
    BF_BIT_ACTIVED       = 1L << 0, // 
    BF_BIT_SHOW_TO_SELF  = 1L << 1, // 是否显示给自己
    BF_BIT_SHOW_TO_OTHER = 1L << 2, // 是否显示给自己
    BF_BIT_DEAD_CLR      = 1L << 3, // 死亡后是否清除
    BF_BIT_OFFLINE_CLR   = 1L << 4,
    BF_BIT_SCP_ACTIVITY  = 1L << 5, // 副本活动类buff
    BF_BIT_END
  };
  buff_obj() { }
  virtual ~buff_obj() { }

  void init(char_obj *owner,
            const int bf_id,
            const buff_effect_id_t effect,
            const int src_id,
            const int buff_rel_cid,
            const int left_value,
            const int left_time,
            const int interval,
            const int value,
            const time_value &now,
            const int bits,
            const int param);

  virtual void release() = 0;
  virtual void remove() { this->release(); }
  virtual int run(const time_value &) = 0;
  virtual int do_dj(const int left_value, const int left_time) = 0;
public:
  char dj_cnt_;                    // 叠加数量
  buff_effect_id_t effect_;
  int  id_;                        // BUFF id
  int  value_;                     // BUFF效果值
  int  src_id_;                    // 此BUF的施加者
  int  buff_rel_cid_;              // 源CID  skill or item or ....
  int  left_value_;
  int  left_time_;
  int  interval_;                  // BUFF间隔值
  int  bits_;                      //
  int  any_param_;
  int  param_1_;
  int  param_2_;
  int  param_3_;

  char_obj *owner_;
  time_value last_update_time_;    // 
};
/**
 * @class only_timing_buff
 *
 * @brief
 */
class only_timing_buff : public buff_obj
{
  typedef buff_obj super;
public:
  only_timing_buff() { }

  virtual int do_dj(const int , const int );
  virtual int run(const time_value &);
  virtual void release();
};
class only_timing_buff_alloctor : public singleton<only_timing_buff_alloctor>
{
  friend class singleton<only_timing_buff_alloctor>;
public:
  only_timing_buff *alloc() { return this->pool_.alloc(); }
  void release(only_timing_buff *p) { this->pool_.release(p); }
private:
  obj_pool<only_timing_buff, obj_pool_std_allocator<only_timing_buff> > pool_;
};
/**
 * @class last_area_hurt_buff
 *
 * @brief 以施放者为中心，持续进行伤害
 */
class last_area_hurt_buff : public buff_obj
{
  typedef buff_obj super;
public:
  last_area_hurt_buff() { }

  virtual int do_dj(const int , const int ) { return ERR_BUFF_EXIST; }
  virtual int run(const time_value &);
  virtual void release();
private:
  time_value last_action_time_;
};
class last_area_hurt_buff_alloctor : public singleton<last_area_hurt_buff_alloctor>
{
  friend class singleton<last_area_hurt_buff_alloctor>;
public:
  last_area_hurt_buff *alloc() { return this->pool_.alloc(); }
  void release(last_area_hurt_buff *p) { this->pool_.release(p); }
private:
  obj_pool<last_area_hurt_buff, obj_pool_std_allocator<last_area_hurt_buff> > pool_;
};
/**
 * @class last_percent_hurt_buff
 *
 * @brief 携带此BUF的单位持续受到伤害
 */
class last_percent_hurt_buff : public buff_obj
{
  typedef buff_obj super;
public:
  last_percent_hurt_buff() { }

  virtual int do_dj(const int , const int ) { return ERR_BUFF_EXIST; }
  virtual int run(const time_value &);
  virtual void release();
private:
  time_value last_action_time_;
};
class last_percent_hurt_buff_alloctor : public singleton<last_percent_hurt_buff_alloctor>
{
  friend class singleton<last_percent_hurt_buff_alloctor>;
public:
  last_percent_hurt_buff *alloc() { return this->pool_.alloc(); }
  void release(last_percent_hurt_buff *p) { this->pool_.release(p); }
private:
  obj_pool<last_percent_hurt_buff, obj_pool_std_allocator<last_percent_hurt_buff> > pool_;
};
/**
 * @class jian_su_buff
 *
 * @brief
 */
class jian_su_buff : public buff_obj
{
  typedef buff_obj super;
public:
  jian_su_buff() { }

  virtual int do_dj(const int , const int ) { return ERR_BUFF_EXIST; }
  virtual int run(const time_value &);
  virtual void release();
  virtual void remove();
protected:
  void do_update_attr(const bool );
};
class jian_su_buff_alloctor : public singleton<jian_su_buff_alloctor>
{
  friend class singleton<jian_su_buff_alloctor>;
public:
  jian_su_buff *alloc() { return this->pool_.alloc(); }
  void release(jian_su_buff *p) { this->pool_.release(p); }
private:
  obj_pool<jian_su_buff, obj_pool_std_allocator<jian_su_buff> > pool_;
};
/**
 * @class add_all_attr_r_buff
 *
 * @brief
 */
class add_all_attr_r_buff : public buff_obj
{
  typedef buff_obj super;
public:
  add_all_attr_r_buff() { }

  virtual int do_dj(const int , const int ) { return ERR_BUFF_EXIST; }
  virtual int run(const time_value &);
  virtual void release();
  virtual void remove();
protected:
  void do_update_attr(const bool );
};
class add_all_attr_r_buff_alloctor : public singleton<add_all_attr_r_buff_alloctor>
{
  friend class singleton<add_all_attr_r_buff_alloctor>;
public:
  add_all_attr_r_buff *alloc() { return this->pool_.alloc(); }
  void release(add_all_attr_r_buff *p) { this->pool_.release(p); }
private:
  obj_pool<add_all_attr_r_buff, obj_pool_std_allocator<add_all_attr_r_buff> > pool_;
};
/**
 * @class add_attr_r_buff
 *
 * @brief
 */
class add_attr_r_buff : public buff_obj
{
  typedef buff_obj super;
public:
  add_attr_r_buff() { }

  virtual int do_dj(const int , const int ) { return ERR_BUFF_EXIST; }
  virtual int run(const time_value &);
  virtual void release();
  virtual void remove();
protected:
  void do_update_attr(const bool );
};
class add_attr_r_buff_alloctor : public singleton<add_attr_r_buff_alloctor>
{
  friend class singleton<add_attr_r_buff_alloctor>;
public:
  add_attr_r_buff *alloc() { return this->pool_.alloc(); }
  void release(add_attr_r_buff *p) { this->pool_.release(p); }
private:
  obj_pool<add_attr_r_buff, obj_pool_std_allocator<add_attr_r_buff> > pool_;
};
/**
 * @class bian_shen_buff
 *
 * @brief
 */
class bian_shen_buff : public buff_obj
{
  typedef buff_obj super;
public:
  bian_shen_buff() { }

  virtual int do_dj(const int , const int ) { return ERR_BUFF_EXIST; }
  virtual int run(const time_value &);
  virtual void release();
  virtual void remove();
protected:
  void do_update_attr(const bool );
};
class bian_shen_buff_alloctor : public singleton<bian_shen_buff_alloctor>
{
  friend class singleton<bian_shen_buff_alloctor>;
public:
  bian_shen_buff *alloc() { return this->pool_.alloc(); }
  void release(bian_shen_buff *p) { this->pool_.release(p); }
private:
  obj_pool<bian_shen_buff, obj_pool_std_allocator<bian_shen_buff> > pool_;
};
/**
 * @class last_add_hp_buff
 *
 * @brief
 */
class last_add_hp_buff : public buff_obj
{
  typedef buff_obj super;
public:
  last_add_hp_buff() { }

  virtual int do_dj(const int , const int ) { return ERR_BUFF_EXIST; }
  virtual int run(const time_value &);
  virtual void release();
private:
  time_value last_action_time_;
};
class last_add_hp_buff_alloctor : public singleton<last_add_hp_buff_alloctor>
{
  friend class singleton<last_add_hp_buff_alloctor>;
public:
  last_add_hp_buff *alloc() { return this->pool_.alloc(); }
  void release(last_add_hp_buff *p) { this->pool_.release(p); }
private:
  obj_pool<last_add_hp_buff, obj_pool_std_allocator<last_add_hp_buff> > pool_;
};
/**
 * @class consume_value_buff
 *
 * @brief
 */
class consume_value_buff : public buff_obj
{
  typedef buff_obj super;
public:
  consume_value_buff() { }

  virtual int do_dj(const int , const int ) { return ERR_BUFF_EXIST; }
  virtual int run(const time_value &);
  virtual void release();
};
class consume_value_buff_alloctor : public singleton<consume_value_buff_alloctor>
{
  friend class singleton<consume_value_buff_alloctor>;
public:
  consume_value_buff *alloc() { return this->pool_.alloc(); }
  void release(consume_value_buff *p) { this->pool_.release(p); }
private:
  obj_pool<consume_value_buff, obj_pool_std_allocator<consume_value_buff> > pool_;
};

#endif // BUFF_OBJ_H_

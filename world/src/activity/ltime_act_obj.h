// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2014-06-20 19:09
 */
//========================================================================

#ifndef LTIME_ACT_OBJ_H_
#define LTIME_ACT_OBJ_H_

#include "ilist.h"

// Forward declarations
class player_obj;
class ltime_recharge_award;

/**
 * @class ltime_act_obj
 * 
 * @brief
 */
class ltime_act_obj
{
public:
  enum
  {
    ST_OPENED   = 211,
    ST_CLOSED   = 212,
  };
  ltime_act_obj(const int act_id,
                const int begin_time,
                const int end_time);
  virtual ~ltime_act_obj();

  virtual int open()  { return 0; }
  virtual int close() { return 0; }

  virtual void run(const int );
  virtual int  do_ai(const int ) { return 0; }
  virtual bool is_opened() { return this->status_ == ST_OPENED; } 

  void on_time_update(const int begin_time, const int end_time);
  void on_enter_game(player_obj *player);
  int act_id() const { return this->act_id_; }
  int left_time();
  int begin_time() const { return this->begin_time_; }
  int end_time()   const { return this->end_time_;   }
protected:
  virtual void on_activity_opened();
  virtual void on_activity_closed();
  bool do_check_if_opened(const int now);

  void do_send_mail_when_opened(const int mail_id);
protected:
  int act_id_;
  int status_;
  int begin_time_;
  int end_time_;
};

/**
 * @class ltime_exp_act_obj
 * 
 * @brief
 */
class ltime_exp_act_obj : public ltime_act_obj
{
  typedef ltime_act_obj super;
public:
  ltime_exp_act_obj(const int act_id,
                    const int begin_time,
                    const int end_time) :
    ltime_act_obj(act_id, begin_time, end_time)
  { }
  virtual void on_activity_opened();
};

/**
 * @class ltime_drop_act_obj
 * 
 * @brief
 */
class ltime_drop_act_obj : public ltime_act_obj
{
  typedef ltime_act_obj super;
public:
  ltime_drop_act_obj(const int act_id,
                     const int begin_time,
                     const int end_time) :
    ltime_act_obj(act_id, begin_time, end_time)
  { }
  virtual void on_activity_opened();
};

/**
 * @class ltime_acc_recharge_act_obj
 * 
 * @brief
 */
class ltime_acc_recharge_act_obj : public ltime_act_obj
{
  typedef ltime_act_obj super;
public:
  ltime_acc_recharge_act_obj(const int act_id,
                             const int begin_time,
                             const int end_time) :
    ltime_act_obj(act_id, begin_time, end_time)
  { }
  virtual void on_activity_opened();

  static void on_recharge_ok(player_obj *player);
  static bool if_got_award(player_obj *player,
                           ilist<ltime_recharge_award *> &award_l,
                           const int act_id,
                           const int sub_id,
                           const int begin_time,
                           const int end_time);
  static int acc_recharge(player_obj *);
};

/**
 * @class ltime_daily_recharge_act_obj
 * 
 * @brief
 */
class ltime_daily_recharge_act_obj : public ltime_act_obj
{
  typedef ltime_act_obj super;
public:
  ltime_daily_recharge_act_obj(const int act_id,
                               const int begin_time,
                               const int end_time) :
    ltime_act_obj(act_id, begin_time, end_time)
  { }
  virtual void on_activity_opened();

  static void on_recharge_ok(player_obj *player);
  static bool if_got_award(player_obj *player,
                           ilist<ltime_recharge_award *> &award_l,
                           const int act_id,
                           const int sub_id,
                           const int begin_time,
                           const int end_time);
  static int daily_recharge(player_obj *);
};

/**
 * @class ltime_acc_consume_act_obj
 * 
 * @brief
 */
class ltime_acc_consume_act_obj : public ltime_act_obj
{
  typedef ltime_act_obj super;
public:
  ltime_acc_consume_act_obj(const int act_id,
                             const int begin_time,
                             const int end_time) :
    ltime_act_obj(act_id, begin_time, end_time)
  { }
  virtual void on_activity_opened();

  static void on_consume_diamond(player_obj *player);
  static bool if_got_award(player_obj *player,
                           ilist<ltime_recharge_award *> &award_l,
                           const int act_id,
                           const int sub_id,
                           const int begin_time,
                           const int end_time);
  static int acc_consume(player_obj *);
};

/**
 * @class ltime_daily_consume_act_obj
 * 
 * @brief
 */
class ltime_daily_consume_act_obj : public ltime_act_obj
{
  typedef ltime_act_obj super;
public:
  ltime_daily_consume_act_obj(const int act_id,
                               const int begin_time,
                               const int end_time) :
    ltime_act_obj(act_id, begin_time, end_time)
  { }
  virtual void on_activity_opened();

  static void on_consume_diamond(player_obj *player);
  static bool if_got_award(player_obj *player,
                           ilist<ltime_recharge_award *> &award_l,
                           const int act_id,
                           const int sub_id,
                           const int begin_time,
                           const int end_time);
  static int daily_consume(player_obj *);
};
#endif // LTIME_ACT_OBJ_H_


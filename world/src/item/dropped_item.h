// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-12-21 09:18
 */
//========================================================================

#ifndef DROPPED_ITEM_H_
#define DROPPED_ITEM_H_

// Lib header
#include "singleton.h"

#include "obj_pool.h"
#include "scene_unit.h"

#define DROP_AREA_RADIUS    4

// Forward declarations

/**
 * @class dropped_item
 * 
 * @brief
 */
class dropped_item : public scene_unit
{
public:
  dropped_item() { this->reset(); }

  void reset()
  {
    this->notice_       = false;
    this->bind_type_    = BIND_TYPE;
    this->amount_       = 0;
    this->owner_id_     = 0;
    this->src_cid_      = 0;
    this->src_id_       = 0;
    this->dropped_time_ = 0;
    this->cid_          = 0;
    this->id_           = 0;
    this->coord_x_      = 0;
    this->coord_y_      = 0;
    this->scene_id_     = 0;
    this->scene_cid_    = 0;
  }

  //= inline method
  virtual int unit_type()   const { return scene_unit::DROPPED_ITEM; }
  virtual bool can_block()  const { return false; }

  virtual bool do_build_snap_info(const int char_id,
                                  out_stream &os,
                                  ilist<pair_t<int> > *old_snap_list,
                                  ilist<pair_t<int> > *new_snap_list);

  void reset_pos(const int scene_id,
                 const int scene_cid,
                 const short x,
                 const short y);
  
  int do_enter_scene();
  int do_exit_scene();

  static void do_clear_dropped_item_list();
  static void do_build_drop_item_list(const int owner_id,
                                      const int mst_id,
                                      const int mst_cid,
                                      ilist<dropped_item *> &dropped_item_list);
  static void do_drop_item_from_box(const int owner_id,
                                    const int src_id,
                                    const int src_cid,
                                    const int box_cid,
                                    ilist<dropped_item *> &dropped_item_list);
  static dropped_item *do_build_one_dropped_item(const int owner_id,
                                                 const int src_id,
                                                 const int src_cid,
                                                 const bool notice,
                                                 const char bind,
                                                 const int amount,
                                                 const int item_cid);
private:
  void do_build_snap_base_info(out_stream &);
public:
  bool notice_;
  char bind_type_;
  int  amount_;
  int  dropped_time_;
  int  owner_id_;
  int  src_id_;
  int  src_cid_;

  static ilist<dropped_item *> dropped_item_list;
};
class dropped_item_pool : public singleton<dropped_item_pool>
{
  friend class singleton<dropped_item_pool>;
public:
  dropped_item *alloc()
  { return this->pool_.alloc(); }

  void release(dropped_item *p)
  { 
    p->reset();
    this->pool_.release(p); 
  }
private:
  dropped_item_pool() { }
  obj_pool<dropped_item, obj_pool_std_allocator<dropped_item> > pool_;
};

#endif // DROPPED_ITEM_H_


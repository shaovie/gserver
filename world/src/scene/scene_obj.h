// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-08-14 15:28
 */
//========================================================================

#ifndef SCENE_OBJ_H_
#define SCENE_OBJ_H_

// Lib header

#include "def.h"
#include "ilist.h"
#include "iheap.h"
#include "scene_unit.h"

// Forward declarations
class mblock;
class time_value;
class scene_unit;

class find_path_coord
{
public:
  enum
  {
    STATUS_UNKNOWN = 0,
    STATUS_OPEN,
    STATUS_CLOSE
  };

  find_path_coord() : status_(STATUS_UNKNOWN), parent_(NULL) { }

  short distance() const { return this->g_ + this->h_; }
  void  distance(const short g, const short h) { this->g_ = g; this->h_ = h; }
  inline bool operator <(const find_path_coord &c)
  { return this->distance() < c.distance(); }

public:
  short g_;
  short h_;

  int status_;
  coord_t coord_;
  find_path_coord *parent_;
};
/**
 * @class scene_obj
 * 
 * @brief
 */
class scene_obj
{
public:
  class scene_grid_info
  {
  public:
    scene_grid_info() : block_layer_(0) { }

    int block_layer_;
    ilist<scene_unit *> unit_list_;
  }; 
  scene_obj(const int scene_cid);
  virtual ~scene_obj();

  virtual int init(const short x_len, const short y_len);

  int unit_cnt(const int t) const { return this->unit_cnt_[t]; }

  //= 
  virtual int enter_scene(scene_unit *su, const short cur_x, const short cur_y);

  virtual int exit_scene(scene_unit *su, const short cur_x, const short cur_y);

  inline int change_coord(scene_unit *su,
                          const short old_x,
                          const short old_y,
                          const short new_x,
                          const short new_y)
  {
    if (old_x == new_x && old_y == new_y)
      return 0;

    this->exit_scene(su, old_x, old_y);
    this->enter_scene(su, new_x, new_y);
    return 0;
  }

  inline bool can_move(const short x, const short y)
  {
    scene_grid_info *sginfo = this->scene_grid_info_[y * this->scene_width_ + x];
    if (sginfo == NULL) return true;
    return sginfo->block_layer_ <= 0;
  }
  int find_path(const coord_t &start,
                const coord_t &end,
                const bool can_chuan_tou,
                const int max_path,
                coord_t *path,
                int &path_size);

  //
  int broadcast(const int char_id,
                const bool to_self,
                const short x,
                const short y,
                mblock *mb);
  //
  virtual int scan_scene_unit_info(const short cur_x,
                                   const short cur_y,
                                   const int char_id,
                                   ilist<mblock *> &slice_mb_list,
                                   ilist<pair_t<int> > *old_scene_unit_list,
                                   ilist<pair_t<int> > *new_scene_unit_list);
  int get_scene_unit_list(const int result_size,
                          const int include_unit_type,
                          const int not_include_st,
                          const short cur_x,
                          const short cur_y,
                          const short area_radius,
                          int got_result[]);
  int get_1_4_view_obj_set(const int result_size,
                           const int include_unit_type,
                           const int not_include_st,
                           const short cur_x,
                           const short cur_y,
                           const char dir,
                           const int area_radius,
                           int result[]);
  int get_line_area_obj_set(const int result_size,
                            const int include_unit_type,
                            const int not_include_st,
                            const short cur_x,
                            const short cur_y,
                            const char dir,
                            const short end_x,
                            const short end_y,
                            const int area_radius,
                            int result[]);
  int get_scene_empty_pos_list(scene_unit *su,
                               const int result_size,
                               const short area_radius,
                               coord_t got_result[]);
protected:
  int scan_grid_unit_info(ilist<scene_unit *> &unit_list,
                          const int char_id,
                          ilist<mblock *> &slice_mb_list,
                          ilist<pair_t<int> > *old_scene_unit_list,
                          ilist<pair_t<int> > *new_scene_unit_list);

private:
  void set_block_radius_value(scene_unit *,
                              const short cur_x,
                              const short cur_y,
                              const int block_v);
  short scene_width_;
  short scene_height_;
  int   scene_cid_;

  int unit_cnt_[scene_unit::UNIT_CNT];

  find_path_coord         *gh_map_;
  iheap<find_path_coord>   open_heap_;
  scene_grid_info **scene_grid_info_;
};

#endif // SCENE_OBJ_H_


#include "scene_obj.h"
#include "scene_unit.h"
#include "scene_config.h"
#include "global_macros.h"
#include "mblock_pool.h"
#include "scene_unit.h"
#include "istream.h"
#include "sys_log.h"
#include "util.h"
#include "def.h"

// Lib header
#include "macros.h"

// For a start find path
#define A_STAR_DISTANCE(F_X,F_Y,T_X,T_Y) ((abs(F_X-T_X)+abs(F_Y-T_Y))*ZHI_VALUE)
#define A_STAR_IDX(X,Y,MAX_Y)            ((X)*(MAX_Y)+(Y))
#define ZHI_VALUE                        10
#define XIE_VALUE                        14

#define CALC_VIEW_BEGIN_XY_END_XY(VIEW_WIDTH_RADIUS, VIEW_HEIGHT_RADIUS) \
  short begin_x = cur_x - VIEW_WIDTH_RADIUS; \
short begin_y = cur_y - VIEW_HEIGHT_RADIUS; \
if (begin_x < 0) begin_x = 0; \
if (begin_y < 0) begin_y = 0; \
short end_x = cur_x + VIEW_WIDTH_RADIUS + 1; \
short end_y = cur_y + VIEW_HEIGHT_RADIUS + 1; \
if (end_x > this->scene_width_) end_x = this->scene_width_; \
if (end_y > this->scene_height_) end_y = this->scene_height_

static ilog_obj *e_log = err_log::instance()->get_ilog("scene");
static ilog_obj *s_log = err_log::instance()->get_ilog("scene");

scene_obj::scene_obj(const int scene_cid) :
  scene_width_(0),
  scene_height_(0),
  scene_cid_(scene_cid),
  gh_map_(NULL),
  scene_grid_info_(NULL)
{
  ::memset(unit_cnt_, 0, sizeof(unit_cnt_));
}
scene_obj::~scene_obj()
{
  // release all scene unit pointer list
  if (this->scene_grid_info_ != NULL)
  {
    for (int i = 0; i < this->scene_height_ * this->scene_width_; ++i)
      delete this->scene_grid_info_[i];
  }

  // release all nine grid cell
  if (this->scene_grid_info_ != NULL)
    delete []this->scene_grid_info_;
  if (this->gh_map_ != NULL) delete []this->gh_map_;
}
int scene_obj::init(const short x_l, const short y_l)
{
  this->scene_width_  = x_l;
  this->scene_height_ = y_l;

  this->scene_grid_info_ = new scene_grid_info *[this->scene_height_*this->scene_width_]();
  for (int i = 0; i < this->scene_height_ * this->scene_width_; ++i)
    this->scene_grid_info_[i] = NULL;

  this->gh_map_ = new find_path_coord[this->scene_height_* this->scene_width_]();
  return 0;
}
int scene_obj::enter_scene(scene_unit *su, const short cur_x, const short cur_y)
{
  scene_grid_info *sginfo = this->scene_grid_info_[cur_y * this->scene_width_ + cur_x];
  if (sginfo == NULL)
  {
    sginfo = new scene_grid_info();
    this->scene_grid_info_[cur_y * this->scene_width_ + cur_x] = sginfo;
  }
  if (su->can_block())
  {
    sginfo->block_layer_ += 1;
    this->set_block_radius_value(su, cur_x, cur_y, 1);
  }
  ++this->unit_cnt_[su->unit_type()];
  sginfo->unit_list_.push_back(su);
  return 0;
}
int scene_obj::exit_scene(scene_unit *su, const short cur_x, const short cur_y)
{
  scene_grid_info *sginfo = this->scene_grid_info_[cur_y * this->scene_width_ + cur_x];
  if (sginfo == NULL) return -1;
  if (sginfo->unit_list_.remove(su) != 0) return -1;

  if (su->can_block())
  {
    sginfo->block_layer_ -= 1;
    this->set_block_radius_value(su, cur_x, cur_y, -1);
  }
  --this->unit_cnt_[su->unit_type()];
  return 0;
}
void scene_obj::set_block_radius_value(scene_unit *su,
                                       const short cur_x,
                                       const short cur_y,
                                       const int block_v)
{
  int block_radius = su->block_radius();
  if (block_radius == 0) return ;

  CALC_VIEW_BEGIN_XY_END_XY(block_radius, block_radius);

  for (short y = begin_y; y < end_y; ++y) 
    for (short x = begin_x; x < end_x; ++x) 
    {
      if (!scene_config::instance()->can_move(su->scene_cid(), x, y))
        continue;
      scene_grid_info *pinfo = this->scene_grid_info_[y * this->scene_width_ + x];
      if (pinfo == NULL)
      {
        pinfo = new scene_grid_info();
        this->scene_grid_info_[y * this->scene_width_ + x] = pinfo;
      }
      pinfo->block_layer_ += block_v;
    }
  return ;
}
int scene_obj::find_path(const coord_t &start,
                         const coord_t &end,
                         const bool can_chuan_tou,
                         const int max_path,
                         coord_t *path,
                         int &path_size)
{
  static const int path_direct[][3] = {
    { 0, -1, ZHI_VALUE}, { 1, -1, XIE_VALUE},
    { 1,  0, ZHI_VALUE}, { 1,  1, XIE_VALUE},
    { 0,  1, ZHI_VALUE}, {-1,  1, XIE_VALUE},
    {-1,  0, ZHI_VALUE}, {-1, -1, XIE_VALUE}
  };

  if (start == end) return 0;

  this->open_heap_.reset();
  ::memset((char *)this->gh_map_,
           0,
           sizeof(find_path_coord) * this->scene_width_ * this->scene_height_);

  find_path_coord &beg = this->gh_map_[start.y_ * this->scene_width_ + start.x_];
  beg.status_ = find_path_coord::STATUS_OPEN;
  beg.distance(0, A_STAR_DISTANCE(start.x_, start.y_, end.x_, end.y_));
  beg.coord_ = start;

  this->open_heap_.push(&beg);

  find_path_coord *current = NULL;
  while (1)
  {
    current = this->open_heap_.pop();
    if (NULL == current)
    {
      e_log->error("find path failed! pop null");
      return -1;
    }

    if (current->coord_ == end)
    {
      find_path_coord * parent = current->parent_;
      while (parent->coord_ != start)
      {
        find_path_coord *p_parent = parent->parent_;
        parent->parent_ = current;

        current = parent;
        parent = p_parent;
      }
      parent->parent_ = current;

      //output  len nodes
      current = parent;
      for (int i = 0; i < max_path; ++i)
      {
        path[i] = current->coord_;
        ++path_size;

        if (current->coord_ == end)
          return 0;
        current = current->parent_;
      }
      return 0;
    }

    this->gh_map_[current->coord_.y_ * this->scene_width_ \
      + current->coord_.x_].status_ = find_path_coord::STATUS_CLOSE;

    for (size_t i = 0; i < sizeof(path_direct)/sizeof(path_direct[0]); ++i)
    {
      int x = current->coord_.x_ + path_direct[i][0];
      int y = current->coord_.y_ + path_direct[i][1];
      coord_t next_coord(x, y);

      int g = current->g_ + path_direct[i][2];
      int h = A_STAR_DISTANCE(next_coord.x_, next_coord.y_, end.x_, end.y_);

      find_path_coord &next_find_path_coord = this->gh_map_[next_coord.y_\
                                              * this->scene_width_ + next_coord.x_];
      if (scene_config::instance()->can_move(this->scene_cid_, x, y)
          && (can_chuan_tou || this->can_move(x, y))
          && next_find_path_coord.status_ != find_path_coord::STATUS_CLOSE)
      {
        if (next_find_path_coord.status_ == find_path_coord::STATUS_UNKNOWN)
        {
          next_find_path_coord.status_ = find_path_coord::STATUS_OPEN; 
          next_find_path_coord.distance(g, h);
          next_find_path_coord.coord_  = next_coord;
          next_find_path_coord.parent_ = current;
          this->open_heap_.push(&next_find_path_coord);
        }else
        {
          if (next_find_path_coord.distance() > (g + h))
          {
            next_find_path_coord.distance(g, h);
            next_find_path_coord.parent_ = current;
          }
        }
      }
    }
  }
  return -1;
}
int scene_obj::scan_scene_unit_info(const short cur_x,
                                    const short cur_y,
                                    const int char_id,
                                    ilist<mblock *> &slice_mb_list,
                                    ilist<pair_t<int> > *old_scene_unit_list,
                                    ilist<pair_t<int> > *new_scene_unit_list)
{
  if (this->scene_grid_info_ == NULL) return 0;
  CALC_VIEW_BEGIN_XY_END_XY(CLT_VIEW_WIDTH_RADIUS, CLT_VIEW_HEIGHT_RADIUS);

  int scan_cnt = 0;
  for (short y = begin_y; y < end_y; ++y)
    for (short x = begin_x; x < end_x; ++x)
    {
      scene_grid_info *sginfo = this->scene_grid_info_[y * this->scene_width_ + x];
      if (sginfo == NULL || sginfo->unit_list_.empty()) continue;
      scan_cnt += this->scan_grid_unit_info(sginfo->unit_list_,
                                            char_id,
                                            slice_mb_list,
                                            old_scene_unit_list,
                                            new_scene_unit_list);
    }
  return scan_cnt;
}
int scene_obj::scan_grid_unit_info(ilist<scene_unit *> &unit_list,
                                    const int char_id,
                                    ilist<mblock *> &slice_mb_list,
                                    ilist<pair_t<int> > *old_scene_unit_list,
                                    ilist<pair_t<int> > *new_scene_unit_list)
{
  int scan_cnt = 0;
  mblock *slice_mb = mblock_pool::instance()->alloc(MAX_SNAP_SLICE_MB_SIZE);
  for (ilist_node<scene_unit *> *itor = unit_list.head();
       itor != NULL;
       itor = itor->next_)
  {
    out_stream scene_unit_os(slice_mb->wr_ptr(), slice_mb->space());
    if (itor->value_->do_build_snap_info(char_id,
                                         scene_unit_os,
                                         old_scene_unit_list,
                                         new_scene_unit_list))
    {
      ++scan_cnt;
      slice_mb->wr_ptr(scene_unit_os.length());
      assert(scene_unit_os.length() < MAX_SNAP_ONE_UNIT_INFO_SIZE);
    }
    if (slice_mb->space() < MAX_SNAP_ONE_UNIT_INFO_SIZE)
    {
      slice_mb_list.push_back(slice_mb);
      slice_mb = mblock_pool::instance()->alloc(MAX_SNAP_SLICE_MB_SIZE);
    }
  }
  if (slice_mb->length() == 0)
    mblock_pool::instance()->release(slice_mb);
  else
    slice_mb_list.push_back(slice_mb);
  return scan_cnt;
}
int scene_obj::broadcast(const int char_id,
                         const bool to_self,
                         const short cur_x,
                         const short cur_y,
                         mblock *mb)
{
  if (this->scene_grid_info_ == NULL) return 0;
  CALC_VIEW_BEGIN_XY_END_XY(CLT_VIEW_WIDTH_RADIUS, CLT_VIEW_HEIGHT_RADIUS);

  for (int y = begin_y; y < end_y; ++y)
    for (int x = begin_x; x < end_x; ++x)
    {
      scene_grid_info *sginfo = this->scene_grid_info_[y * this->scene_width_ + x];
      if (sginfo == NULL || sginfo->unit_list_.empty()) continue;
      for (ilist_node<scene_unit *> *itor = sginfo->unit_list_.head();
           itor != NULL;
           itor = itor->next_)
      {
        if (itor->value_->id() == char_id && !to_self)
          continue;
        itor->value_->do_delivery(mb);
      }
    }
  return 0;
}
int scene_obj::get_scene_unit_list(const int result_size,
                                   const int include_unit_type,
                                   const int not_include_st,
                                   const short cur_x,
                                   const short cur_y,
                                   const short area_radius,
                                   int got_result[])
{
  if (this->scene_grid_info_ == NULL) return 0;
  CALC_VIEW_BEGIN_XY_END_XY(area_radius, area_radius);

  int cnt = 0;
  for (int y = begin_y; y < end_y; ++y)
    for (int x = begin_x; x < end_x; ++x)
    {
      scene_grid_info *sginfo = this->scene_grid_info_[y * this->scene_width_ + x];
      if (sginfo == NULL || sginfo->unit_list_.empty()) continue;
      for (ilist_node<scene_unit *> *itor = sginfo->unit_list_.head();
           itor != NULL;
           itor = itor->next_)
      {
        if (BIT_ENABLED(itor->value_->unit_type(), include_unit_type)
            && BIT_DISABLED(itor->value_->unit_status(), not_include_st))
        {
          got_result[cnt++] = itor->value_->id();
          if (cnt >= result_size)
            return cnt;
        }
      }
    }
  return cnt;
}
int scene_obj::get_1_4_view_obj_set(const int result_size,
                                    const int include_unit_type,
                                    const int not_include_st,
                                    const short cur_x,
                                    const short cur_y,
                                    const char dir,
                                    const int area_radius,
                                    int result[])
{
  /*
     o---------o
     |o   |   o|
     | o  |  o |
     |  o | o  |
     |   o|o   |
     |----o----|
     |   o|o   |
     |  o | o  |
     | o  |  o |
     |o   |   o|
     o---------o
     */
#define GET_GRID_UNITS(x, y) \
  scene_grid_info *sginfo = this->scene_grid_info_[y * this->scene_width_ + x]; \
  if (sginfo == NULL || sginfo->unit_list_.empty()) continue; \
  for (ilist_node<scene_unit *> *itor = sginfo->unit_list_.head(); \
       itor != NULL; \
       itor = itor->next_) \
  { \
    if (BIT_ENABLED(itor->value_->unit_type(), include_unit_type) \
        && BIT_DISABLED(itor->value_->unit_status(), not_include_st)) \
    { \
      result[cnt++] = itor->value_->id(); \
      if (cnt >= result_size) \
      return cnt; \
    } \
  }

  if (this->scene_grid_info_ == NULL) return 0;
  int cnt = 0;
  if (dir == DIR_UP)
  {
    short begin_y = cur_y;
    short end_y   = cur_y - area_radius;
    if (end_y < 0) end_y = 0;
    for (short y = begin_y, x_len = 0; y >= end_y; --y, ++x_len)
    {
      for (short x = cur_x - x_len; x <= cur_x + x_len; ++x)
      {
        if (x < 0 || x >= this->scene_width_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_DOWN)
  {
    short begin_y = cur_y;
    short end_y   = cur_y + area_radius + 1;
    if (end_y > this->scene_height_) end_y = this->scene_height_;
    for (short y = begin_y, x_len = 0; y < end_y; ++y, ++x_len)
    {
      for (short x = cur_x - x_len; x <= cur_x + x_len; ++x)
      {
        if (x < 0 || x >= this->scene_width_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_LEFT)
  {
    short begin_x = cur_x - area_radius;
    if (begin_x < 0) begin_x = 0;
    short end_x   = cur_x;
    for (short x = begin_x, y_len = area_radius; x <= end_x; ++x, --y_len)
    {
      for (short y = cur_y - y_len; y <= cur_y + y_len; ++y)
      {
        if (y < 0 || y >= this->scene_height_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_RIGHT)
  {
    short begin_x = cur_x;
    short end_x   = cur_x + area_radius + 1;
    if (end_x > this->scene_width_) end_x = this->scene_width_;
    for (short x = begin_x, y_len = 0; x < end_x; ++x, ++y_len)
    {
      for (short y = cur_y - y_len; y <= cur_y + y_len; ++y)
      {
        if (y < 0 || y >= this->scene_height_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_RIGHT_UP)
  {
    short begin_x = cur_x;
    short end_x   = cur_x + area_radius + 1;
    if (end_x > this->scene_width_) end_x = this->scene_width_;
    short begin_y = cur_y - area_radius;
    if (begin_y < 0) begin_y = 0;
    short end_y   = cur_y;
    for (short y = begin_y; y <= end_y; ++y)
    {
      for (short x = begin_x; x < end_x; ++x)
      {
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_RIGHT_DOWN)
  {
    short begin_x = cur_x;
    short end_x   = cur_x + area_radius + 1;
    if (end_x > this->scene_width_) end_x = this->scene_width_;
    short begin_y = cur_y;
    short end_y   = cur_y + area_radius + 1;
    if (end_y > this->scene_height_) end_y = this->scene_height_;
    for (short y = begin_y; y < end_y; ++y)
    {
      for (short x = begin_x; x < end_x; ++x)
      {
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_LEFT_UP)
  {
    short begin_y = cur_y - area_radius;
    if (begin_y < 0) begin_y = 0;
    short end_y   = cur_y;
    short begin_x = cur_x - area_radius;
    if (begin_x < 0) begin_x = 0;
    short end_x   = cur_x;
    for (short y = begin_y; y <= end_y; ++y)
    {
      for (short x = begin_x; x <= end_x; ++x)
      {
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_LEFT_DOWN)
  {
    short begin_y = cur_y;
    short end_y   = cur_y + area_radius + 1;
    if (end_y > this->scene_height_) end_y = this->scene_height_;
    short begin_x = cur_x - area_radius;
    if (begin_x < 0) begin_x = 0;
    short end_x   = cur_x;
    for (short y = begin_y; y < end_y; ++y)
    {
      for (short x = begin_x; x <= end_x; ++x)
      {
        GET_GRID_UNITS(x, y);
      }
    }
  }
  return cnt;
}
int scene_obj::get_line_area_obj_set(const int result_size,
                                     const int include_unit_type,
                                     const int not_include_st,
                                     const short cur_x,
                                     const short cur_y,
                                     const char dir,
                                     const short target_x,
                                     const short target_y,
                                     const int area_radius,
                                     int result[])
{
#define GET_GRID_UNITS(x, y) \
  scene_grid_info *sginfo = this->scene_grid_info_[y * this->scene_width_ + x]; \
  if (sginfo == NULL || sginfo->unit_list_.empty()) continue; \
  for (ilist_node<scene_unit *> *itor = sginfo->unit_list_.head(); \
       itor != NULL; \
       itor = itor->next_) \
  { \
    if (BIT_ENABLED(itor->value_->unit_type(), include_unit_type) \
        && BIT_DISABLED(itor->value_->unit_status(), not_include_st)) \
    { \
      result[cnt++] = itor->value_->id(); \
      if (cnt >= result_size) \
      return cnt; \
    } \
  }

  if (this->scene_grid_info_ == NULL) return 0;
  int cnt = 0;
  if (dir == DIR_UP)
  {
    if (cur_y <= 0) return 0;
    short begin_y = target_y;
    short end_y = cur_y - 1;
    for (short y = begin_y; y <= end_y; ++y)
    {
      for (short x = cur_x - area_radius; x <= cur_x + area_radius; ++x)
      {
        if (x < 0 || x >= this->scene_width_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_DOWN)
  {
    if (cur_y + 1 >= this->scene_height_) return 0;
    short begin_y = cur_y + 1;
    short end_y = target_y;
    for (short y = begin_y; y <= end_y; ++y)
    {
      for (short x = cur_x - area_radius; x <= cur_x + area_radius; ++x)
      {
        if (x < 0 || x >= this->scene_width_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_LEFT)
  {
    if (cur_x <= 0) return 0;
    short begin_x = target_x;
    short end_x = cur_x - 1;
    for (short x = begin_x; x <= end_x; ++x)
    {
      for (short y = cur_y - area_radius; y <= cur_y + area_radius; ++y)
      {
        if (y < 0 || y >= this->scene_height_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_RIGHT)
  {
    if (cur_x + 1 >= this->scene_width_) return 0;
    short begin_x = cur_x + 1;
    short end_x = target_x;
    for (short x = begin_x; x <= end_x; ++x)
    {
      for (short y = cur_y - area_radius; y <= cur_y + area_radius; ++y)
      {
        if (y < 0 || y >= this->scene_height_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_RIGHT_UP)
  {
    if (cur_x + 1 >= this->scene_width_) return 0;
    if (cur_y <= 0) return 0;
    short begin_x = target_x;
    short end_x = cur_x + 1;
    for (short x = begin_x, y_len = 0; x >= end_x; --x, ++y_len)
    {
      short begin_y = target_y + y_len;
      for (short y = begin_y - area_radius; y <= (begin_y + area_radius); ++y)
      {
        if (y < 0 || y >= this->scene_height_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_RIGHT_DOWN)
  {
    if (cur_x + 1 >= this->scene_width_) return 0;
    if (cur_y + 1 >= this->scene_height_) return 0;
    short begin_x = cur_x + 1;
    short end_x = target_x;
    for (short x = begin_x, y_len = 0; x <= end_x; ++x, ++y_len)
    {
      short begin_y = cur_y + 1 + y_len;
      for (short y = begin_y - area_radius; y <= begin_y + area_radius; ++y)
      {
        if (y < 0 || y >= this->scene_height_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_LEFT_UP)
  {
    if (cur_x <= 0) return 0;
    if (cur_y <= 0) return 0;
    short begin_x = target_x;
    short end_x = cur_x - 1;
    for (short x = begin_x, y_len = 0; x <= end_x; ++x, ++y_len)
    {
      short begin_y = target_y + y_len;
      for (short y = begin_y - area_radius; y <= begin_y + area_radius; ++y)
      {
        if (y < 0 || y >= this->scene_height_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }else if (dir == DIR_LEFT_DOWN)
  {
    if (cur_x <= 0) return 0;
    if (cur_y + 1 >= this->scene_height_) return 0;
    short begin_x = cur_x - 1;
    short end_x = target_x;
    for (short x = begin_x, y_len = 0; x >= end_x; --x, ++y_len)
    {
      short begin_y = cur_y + 1 + y_len;
      for (short y = begin_y - area_radius; y <= begin_y + area_radius; ++y)
      {
        if (y < 0 || y >= this->scene_height_) continue;
        GET_GRID_UNITS(x, y);
      }
    }
  }
  return cnt;
}
int scene_obj::get_scene_empty_pos_list(scene_unit *su,
                                        const int result_size,
                                        const short area_radius,
                                        coord_t got_result[])
{
  if (this->scene_grid_info_ == NULL) return 0;
  const int scene_cid = su->scene_cid();
  const short cur_x = su->coord_x();
  const short cur_y = su->coord_y();
  CALC_VIEW_BEGIN_XY_END_XY(area_radius, area_radius);

  int cnt = 0;
  for (int y = begin_y; y < end_y; ++y)
    for (int x = begin_x; x < end_x; ++x)
    {
      if (!scene_config::instance()->can_move(scene_cid, x, y))
        continue;
      got_result[cnt].x_ = x;
      got_result[cnt].y_ = y;
      ++cnt;
      if (cnt >= result_size)
        return cnt;
    }
  return cnt;
}

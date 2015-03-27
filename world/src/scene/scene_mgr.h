// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2013-08-13 13:47
 */
//========================================================================

#ifndef SCENE_MGR_H_
#define SCENE_MGR_H_

#include "singleton.h"

#include "def.h"
#include "ilist.h"

// Forward declarations
class mblock;
class scene_unit;
class scene_mgr_impl;

/**
 * @class scene_mgr
 * 
 * @brief
 */
class scene_mgr : public singleton<scene_mgr>
{
  friend class singleton<scene_mgr>;
public:
  int init();
  int insert(const int scene_cid,
             const int scene_id,
             const short x_len,
             const short y_len);
  void remove(const int scene_id);

  int unit_cnt(const int scene_id, const int t);

  int enter_scene(scene_unit *su,
                  const int scene_id,
                  const short cur_x,
                  const short cur_y);

  int exit_scene(scene_unit *su,
                 const int scene_id,
                 const short cur_x,
                 const short cur_y);

  int change_coord(scene_unit *su,
                   const int scene_id,
                   const short old_x,
                   const short old_y,
                   const short new_x,
                   const short new_y);

  bool can_move(const int scene_id, const short x, const short y);

  int find_path(const int scene_id,
                const coord_t &start,
                const coord_t &end,
                const bool can_chuan_tou,
                const int max_path,
                coord_t *path,
                int &path_size);

  int scan_scene_unit_info(const int scene_id,
                           const short cur_x,
                           const short cur_y,
                           const int char_id,
                           ilist<mblock *> &slice_mb_list,
                           ilist<pair_t<int> > *old_scene_unit_list,
                           ilist<pair_t<int> > *new_scene_unit_list);
  int broadcast(const int scene_id,
                const int char_id,
                const bool to_self,
                const short cur_x,
                const short cur_y,
                mblock *mb);
  int get_scene_unit_list(const int scene_id,
                          const int result_size,
                          const int include_unit_type,
                          const int not_include_st,
                          const short cur_x,
                          const short cur_y,
                          const short area_radius,
                          int got_result[]);
  int get_1_4_view_obj_set(const int scene_id,
                           const int result_size,
                           const int include_unit_type,
                           const int not_include_st,
                           const short cur_x,
                           const short cur_y,
                           const char dir,
                           const int area_radius,
                           int result[]);
  int get_line_area_obj_set(const int scene_id,
                            const int result_size,
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
                               const int scene_id,
                               const int result_size,
                               const short area_radius,
                               coord_t got_result[]);
private:
  scene_mgr();
  scene_mgr(const scene_mgr &);
  scene_mgr& operator= (const scene_mgr &);

  scene_mgr_impl *impl_;
};

#endif // SCENE_MGR_H_


#include "scene_mgr.h"
#include "scene_obj.h"
#include "scp_config.h"
#include "phony_scene_obj.h"
#include "array_t.h"
#include "global_macros.h"
#include "scene_config.h"
#include "sys_log.h"
#include "clsid.h"

// Lib header

static ilog_obj *e_log = err_log::instance()->get_ilog("scene");
static ilog_obj *s_log = sys_log::instance()->get_ilog("scene");

/**
 * @class scene_mgr_impl
 * 
 * @brief
 */
class scene_mgr_impl
{
public:
  scene_mgr_impl() : scene_obj_map_(MIN_SCP_ID + MAX_SCP_COUNT) { }
  int init()
  {
    ilist<int> &sl = scene_config::instance()->scene_list();
    for (ilist_node<int> *itor = sl.head();
         itor != NULL;
         itor = itor->next_)
    {
      int scene_cid = itor->value_;
      if (clsid::is_scp_scene(scene_cid)
          || clsid::is_tui_tu_scp_scene(scene_cid))
        continue; 
      if (this->insert(scene_cid,
                       scene_cid,
                       scene_config::instance()->x_len(scene_cid),
                       scene_config::instance()->y_len(scene_cid)) != 0)
        return -1;
    }
    return 0;
  }
  int insert(const int scene_cid,
             const int scene_id,
             const short x_len,
             const short y_len)
  {
    scene_obj *so = NULL;
    if (clsid::is_tui_tu_scp_scene(scene_cid)
        || (clsid::is_scp_scene(scene_cid)
            && scp_config::instance()->control_type(scene_cid) == scp_cfg_obj::SCP_CLT_IMPL
            )
        )
      so = new phony_scene_obj(scene_cid);
    else
      so = new scene_obj(scene_cid);
    if (so->init(x_len, y_len) != 0)
    {
      e_log->error("load %d grid failed!", scene_cid);
      delete so;
      return -1;
    }
    this->scene_obj_map_.insert(scene_id, so);
    return 0;
  }
  void remove(const int scene_id)
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    this->scene_obj_map_.remove(scene_id);
    if (so != NULL) delete so;
  }
  int unit_cnt(const int scene_id, const int t)
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return 0;
    return so->unit_cnt(t);
  }
  inline int enter_scene(scene_unit *su,
                         const int scene_id,
                         const short cur_x,
                         const short cur_y)
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->enter_scene(su, cur_x, cur_y);
  }
  inline int exit_scene(scene_unit *su,
                        const int scene_id,
                        const short cur_x,
                        const short cur_y)
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->exit_scene(su, cur_x, cur_y);
  }
  inline int change_coord(scene_unit *su,
                          const int scene_id,
                          const short old_x,
                          const short old_y,
                          const short new_x,
                          const short new_y)
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->change_coord(su, old_x, old_y, new_x, new_y);
  }
  inline bool can_move(const int scene_id,
                       const short x,
                       const short y)
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->can_move(x, y);
  }
  inline int find_path(const int scene_id,
                       const coord_t &start,
                       const coord_t &end,
                       const bool can_chuan_tou,
                       const int max_path,
                       coord_t *path,
                       int &path_size)
  { 
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->find_path(start, end, can_chuan_tou, max_path, path, path_size); 
  }
  inline int scan_scene_unit_info(const int scene_id,
                                  const short cur_x,
                                  const short cur_y,
                                  const int char_id,
                                  ilist<mblock *> &slice_mb_list,
                                  ilist<pair_t<int> > *old_scene_unit_list,
                                  ilist<pair_t<int> > *new_scene_unit_list)
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->scan_scene_unit_info(cur_x,
                                    cur_y,
                                    char_id,
                                    slice_mb_list,
                                    old_scene_unit_list,
                                    new_scene_unit_list);
  }
  inline int broadcast(const int scene_id,
                       const int char_id,
                       const bool to_self,
                       const short cur_x,
                       const short cur_y,
                       mblock *mb)
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->broadcast(char_id,
                         to_self,
                         cur_x,
                         cur_y,
                         mb);
  }
  inline int get_scene_unit_list(const int scene_id,
                                 const int result_size,
                                 const int include_unit_type,
                                 const int not_include_st,
                                 const short cur_x,
                                 const short cur_y,
                                 const short area_radius,
                                 int got_result[])
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->get_scene_unit_list(result_size,
                                   include_unit_type,
                                   not_include_st,
                                   cur_x,
                                   cur_y,
                                   area_radius,
                                   got_result);
  }
  inline int get_1_4_view_obj_set(const int scene_id,
                                  const int result_size,
                                  const int include_unit_type,
                                  const int not_include_st,
                                  const short cur_x,
                                  const short cur_y,
                                  const char dir,
                                  const int area_radius,
                                  int result[])
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->get_1_4_view_obj_set(result_size,
                                    include_unit_type,
                                    not_include_st,
                                    cur_x,
                                    cur_y,
                                    dir,
                                    area_radius,
                                    result);
  }
  inline int get_line_area_obj_set(const int scene_id,
                                   const int result_size,
                                   const int include_unit_type,
                                   const int not_include_st,
                                   const short cur_x,
                                   const short cur_y,
                                   const char dir,
                                   const short end_x,
                                   const short end_y,
                                   const int area_radius,
                                   int result[])
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->get_line_area_obj_set(result_size,
                                     include_unit_type,
                                     not_include_st,
                                     cur_x,
                                     cur_y,
                                     dir,
                                     end_x,
                                     end_y,
                                     area_radius,
                                     result);
  }
  inline int get_scene_empty_pos_list(scene_unit *su,
                                      const int scene_id,
                                      const int result_size,
                                      const short area_radius,
                                      coord_t got_result[])
  {
    scene_obj *so = this->scene_obj_map_.find(scene_id);
    if (so == NULL) return -1;
    return so->get_scene_empty_pos_list(su,
                                        result_size,
                                        area_radius,
                                        got_result);
  }
private:
  array_t<scene_obj *> scene_obj_map_;
};
// -------------------------------------------------------------
scene_mgr::scene_mgr() : impl_(new scene_mgr_impl()) { }
int scene_mgr::init()
{ return this->impl_->init(); }
int scene_mgr::insert(const int scene_cid,
                      const int scene_id,
                      const short x_len,
                      const short y_len)
{ return this->impl_->insert(scene_cid, scene_id, x_len, y_len); }
void scene_mgr::remove(const int scene_id)
{ this->impl_->remove(scene_id); }
int scene_mgr::unit_cnt(const int scene_id, const int t)
{ return this->impl_->unit_cnt(scene_id, t); }
int scene_mgr::enter_scene(scene_unit *su,
                           const int scene_id,
                           const short cur_x,
                           const short cur_y)
{
  return this->impl_->enter_scene(su,
                                  scene_id,
                                  cur_x,
                                  cur_y);
}
int scene_mgr::exit_scene(scene_unit *su,
                          const int scene_id,
                          const short cur_x,
                          const short cur_y)
{
  return this->impl_->exit_scene(su,
                                 scene_id,
                                 cur_x,
                                 cur_y);
}
int scene_mgr::change_coord(scene_unit *su,
                            const int scene_id,
                            const short old_x,
                            const short old_y,
                            const short new_x,
                            const short new_y)
{
  return this->impl_->change_coord(su,
                                   scene_id,
                                   old_x,
                                   old_y,
                                   new_x,
                                   new_y);
}
bool scene_mgr::can_move(const int scene_id, const short x, const short y)
{ return this->impl_->can_move(scene_id, x, y); }
int scene_mgr::find_path(const int scene_id,
                         const coord_t &start,
                         const coord_t &end,
                         const bool can_chuan_tou,
                         const int max_path,
                         coord_t *path,
                         int &path_size)
{
  return this->impl_->find_path(scene_id,
                                start,
                                end,
                                can_chuan_tou,
                                max_path,
                                path,
                                path_size);
}
int scene_mgr::scan_scene_unit_info(const int scene_id,
                                    const short cur_x,
                                    const short cur_y,
                                    const int char_id,
                                    ilist<mblock *> &slice_mb_list,
                                    ilist<pair_t<int> > *old_scene_unit_list,
                                    ilist<pair_t<int> > *new_scene_unit_list)
{
  return this->impl_->scan_scene_unit_info(scene_id,
                                           cur_x,
                                           cur_y,
                                           char_id,
                                           slice_mb_list,
                                           old_scene_unit_list,
                                           new_scene_unit_list);
}
int scene_mgr::broadcast(const int scene_id,
                         const int char_id,
                         const bool to_self,
                         const short cur_x,
                         const short cur_y,
                         mblock *mb)
{
  return this->impl_->broadcast(scene_id,
                                char_id,
                                to_self,
                                cur_x,
                                cur_y,
                                mb);
}
int scene_mgr::get_scene_unit_list(const int scene_id,
                                   const int result_size,
                                   const int include_unit_type,
                                   const int not_include_st,
                                   const short cur_x,
                                   const short cur_y,
                                   const short area_radius,
                                   int got_result[])
{
  return this->impl_->get_scene_unit_list(scene_id,
                                          result_size,
                                          include_unit_type,
                                          not_include_st,
                                          cur_x,
                                          cur_y,
                                          area_radius,
                                          got_result);
}
int scene_mgr::get_1_4_view_obj_set(const int scene_id,
                                    const int result_size,
                                    const int include_unit_type,
                                    const int not_include_st,
                                    const short cur_x,
                                    const short cur_y,
                                    const char dir,
                                    const int area_radius,
                                    int result[])
{
  return this->impl_->get_1_4_view_obj_set(scene_id,
                                           result_size,
                                           include_unit_type,
                                           not_include_st,
                                           cur_x,
                                           cur_y,
                                           dir,
                                           area_radius,
                                           result);
}
int scene_mgr::get_line_area_obj_set(const int scene_id,
                                     const int result_size,
                                     const int include_unit_type,
                                     const int not_include_st,
                                     const short cur_x,
                                     const short cur_y,
                                     const char dir,
                                     const short end_x,
                                     const short end_y,
                                     const int area_radius,
                                     int result[])
{
  return this->impl_->get_line_area_obj_set(scene_id,
                                            result_size,
                                            include_unit_type,
                                            not_include_st,
                                            cur_x,
                                            cur_y,
                                            dir,
                                            end_x,
                                            end_y,
                                            area_radius,
                                            result);
}
int scene_mgr::get_scene_empty_pos_list(scene_unit *su,
                                        const int scene_id,
                                        const int result_size,
                                        const short area_radius,
                                        coord_t got_result[])
{
  return this->impl_->get_scene_empty_pos_list(su,
                                               scene_id,
                                               result_size,
                                               area_radius,
                                               got_result);
}

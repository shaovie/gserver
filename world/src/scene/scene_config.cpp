#include "scene_config.h"
#include "global_macros.h"
#include "scene_mgr.h"
#include "load_json.h"
#include "array_t.h"
#include "sys_log.h"
#include "util.h"
#include "clsid.h"
#include "def.h"

// Lib header
#include "ilist.h"
#include <zlib.h>
#include <set>
#include <fstream>

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

#define UNMOVEABLE_POINT          1
#define MAX_UNZIP_MPT_SIZE        (1024*1024)

int g_scan_dir_order[8][9] = {
  {0, DIR_UP, DIR_RIGHT_UP, DIR_RIGHT, DIR_RIGHT_DOWN, DIR_DOWN, DIR_LEFT_DOWN, DIR_LEFT, DIR_LEFT_UP},
  {0, DIR_RIGHT_UP, DIR_UP, DIR_RIGHT_DOWN, DIR_RIGHT, DIR_LEFT_UP, DIR_LEFT_DOWN, DIR_LEFT, DIR_DOWN},
  {0, DIR_RIGHT, DIR_RIGHT_DOWN, DIR_UP, DIR_RIGHT_UP, DIR_LEFT, DIR_LEFT_UP, DIR_DOWN, DIR_LEFT_DOWN},
  {0, DIR_RIGHT_DOWN, DIR_RIGHT_UP, DIR_RIGHT, DIR_LEFT_UP, DIR_DOWN, DIR_LEFT, DIR_LEFT_DOWN, DIR_UP},
  {0, DIR_DOWN, DIR_RIGHT_UP, DIR_RIGHT, DIR_LEFT, DIR_LEFT_DOWN, DIR_UP, DIR_RIGHT_DOWN, DIR_LEFT_UP},
  {0, DIR_LEFT_DOWN, DIR_RIGHT_UP, DIR_RIGHT_DOWN, DIR_RIGHT, DIR_UP, DIR_DOWN, DIR_LEFT_UP, DIR_LEFT},
  {0, DIR_LEFT, DIR_RIGHT, DIR_RIGHT_UP, DIR_RIGHT_DOWN, DIR_DOWN, DIR_LEFT_DOWN, DIR_UP, DIR_LEFT_UP},
  {0, DIR_LEFT_UP, DIR_RIGHT_DOWN, DIR_RIGHT, DIR_LEFT_DOWN, DIR_DOWN, DIR_RIGHT_UP, DIR_LEFT, DIR_UP}
};

int g_search_table_size = (4*MAX_SEARCH_TABLE_RANGE)*(MAX_SEARCH_TABLE_RANGE+1);
pair_t<short> *g_search_table = new pair_t<short>[g_search_table_size];

static void build_search_table()
{
  for (int i = 0; i < g_search_table_size; ++i)
  {
    g_search_table[i].first_  = 0;
    g_search_table[i].second_ = 0;
  }
  int idx = 0;
  int z = 0;
  for (int i = 1; i <= MAX_SEARCH_TABLE_RANGE; ++i)
  {
    for (int left_up_x = 1; left_up_x <= i*2; ++left_up_x)
    {
      g_search_table[idx].first_  = -(left_up_x - 1 - z);
      g_search_table[idx].second_ = -i;
      ++idx;
    }
    for (int left_down_y = 1; left_down_y <= i*2; ++left_down_y)
    {
      g_search_table[idx].first_  = -i;
      g_search_table[idx].second_ = left_down_y - 1 - z;
      ++idx;
    }
    for (int right_down_x = 1; right_down_x <= i*2; ++right_down_x)
    {
      g_search_table[idx].first_  = right_down_x - 1 - z;
      g_search_table[idx].second_ = i;
      ++idx;
    }
    for (int right_up_y = 1; right_up_y <= i*2; ++right_up_y)
    {
      g_search_table[idx].first_  = i;
      g_search_table[idx].second_ = -(right_up_y - 1 - z);
      ++idx;
    }
    ++z;
  }
}

class mpt_obj
{
public:
  mpt_obj() :
    x_len_(-1),
    y_len_(-1),
    coord_point_(NULL)
  { }
  ~mpt_obj()
  {
    if (this->coord_point_)
      delete []this->coord_point_;
  }
  int load_mpt(const int scene_cid, const char *path)
  {
    if (clsid::is_tui_tu_scp_scene(scene_cid)) return 0;
    std::ifstream fin(path, std::ios::binary|std::ios::in);
    if (!fin) return -1;
    fin.seekg(0, std::ios::end);
    int file_size = fin.tellg();
    if (file_size == 0 || file_size >= MAX_UNZIP_MPT_SIZE)
    {
      e_log->error("get file [%s] size %d failed!", path, file_size);
      return -1;
    }

    fin.seekg(0, std::ios::beg);

    int zip_len = file_size;
    char zip_mpt[MAX_UNZIP_MPT_SIZE] = {0};
    fin.read(zip_mpt, file_size);
    fin.close();

    int unzip_len = MAX_UNZIP_MPT_SIZE;
    uLongf uzl = unzip_len;
    char unzip_mpt[MAX_UNZIP_MPT_SIZE] = {0};
    int result = uncompress((Bytef *)unzip_mpt, 
                            &uzl,
                            (const Bytef *)zip_mpt,
                            zip_len);
    if (result != Z_OK)
    {
      e_log->error("uncompress [%s] failed! [%d]",
                   path,
                   result);
      return -1;
    }
    int r_ptr = 0;
#define BIG_ENDIAN_READ(X, S)                         \
    for (size_t ii = 0; ii < S; ++ii)                 \
    {                                                 \
      ::memcpy((char *)&(X) + S - ii - 1,             \
               unzip_mpt + r_ptr + ii,                \
               sizeof(char));                         \
    }                                                 \
    r_ptr += S

    short cell_width   = 0;
    short cell_height  = 0;
    short map_width    = 0;
    short map_height   = 0;
    short i_point_y    = 0;
    BIG_ENDIAN_READ(cell_width, sizeof(short));
    BIG_ENDIAN_READ(cell_height, sizeof(short));
    BIG_ENDIAN_READ(this->x_len_, sizeof(short));
    BIG_ENDIAN_READ(this->y_len_, sizeof(short));
    BIG_ENDIAN_READ(map_width, sizeof(short));
    BIG_ENDIAN_READ(map_height, sizeof(short));
    BIG_ENDIAN_READ(i_point_y, sizeof(short));   // unkown

    if (this->x_len_ <= 0 || this->y_len_ <= 0)
    {
      e_log->error("invalid mpt header!");
      return -1;
    }
    if (scene_config::instance()->is_reload())
    {
      if (this->x_len_ != scene_config::instance()->x_len(scene_cid)
          || this->y_len_ != scene_config::instance()->y_len(scene_cid))
      {
        e_log->error("reload %d' mpt failed!, x_len y_len is diffrence!",
                     scene_cid);
        return -1;
      }
    }
    // unkown
    r_ptr += sizeof(int) * 10;

    // coord point.
    int c = this->y_len_ * this->x_len_;
    if (this->coord_point_ != NULL)
      delete []this->coord_point_;
    this->coord_point_ = new char[c];
    for (int i = 0; i < c; ++i)
    {
      BIG_ENDIAN_READ(this->coord_point_[i], sizeof(char));
    }
    return 0;
  }
  inline bool can_move(const short x, const short y)
  { 
    if (this->is_valid_coord(x, y))
      return this->coord_point_[y * this->x_len_ + x] != UNMOVEABLE_POINT;
    return false;
  }
  inline char coord_value(const short x, const short y)
  {
    if (this->is_valid_coord(x, y))
      return this->coord_point_[y * this->x_len_ + x];
    return -1;
  }
  inline bool is_valid_coord(const short x, const short y)
  { return (x >= 0 && x < this->x_len_ && y >= 0 && y < this->y_len_); }
public:
  short x_len_;
  short y_len_;

  char *coord_point_;
};
/**
 * @class scene_cfg_obj
 * 
 * @brief
 */
class scene_cfg_obj : public load_json
{
  friend class scene_config_impl;
public:
  scene_cfg_obj() : 
    can_pk_(true),
    can_drop_(true),
    lvl_limit_(1),
    src_cid_(0),
    scene_mpt_(NULL)
  { }
  ~scene_cfg_obj()
  {
    delete this->scene_mpt_;
  }
  int load_config(Json::Value &root, const char *cfg_root, const int scene_cid)
  {
    this->src_cid_ = root["src"].asInt();
    if (this->load_mpt(cfg_root, scene_cid) != 0
        && this->load_mpt(cfg_root, this->src_cid_) != 0)
    {
      e_log->error("load s%d.mpt failed!", scene_cid);
      return -1;
    }

    this->lvl_limit_ = root["limit_lvl"].asInt();
    this->can_pk_ = root["can_pk"].asBool();
    this->can_drop_ = root["can_lose"].asBool();
    this->relive_coord_.cid_ = root["relive_cid"].asInt();
    if (this->relive_coord_.cid_ > 0)
    {
      if (::sscanf(root["relive_coord"].asCString(), "%hd,%hd",
                   &(this->relive_coord_.coord_.x_),
                   &(this->relive_coord_.coord_.y_)) != 2)
        return -1;
    }

    char bf[512] = {0};
    ::strncpy(bf, root["disable_item_list"].asCString(), sizeof(bf) - 1);
    char *tok_p = NULL;
    char *token = NULL;
    for (token = ::strtok_r(bf, ",", &tok_p);
         token != NULL;
         token = ::strtok_r(NULL, ",", &tok_p))
      this->disable_item_set_.insert(::atoi(token));

    return 0;
  }
protected:
  int load_mpt(const char *cfg_root, const int scene_cid)
  {
    char mpt_name[MAX_FILE_PATH_LEN + 1] = {0};
    ::snprintf(mpt_name, sizeof(mpt_name), SCENE_MPT_CFG_PATH_FMT, scene_cid);

    char bf_mpt[MAX_FILE_PATH_LEN + 1] = {0};
    ::snprintf(bf_mpt, sizeof(bf_mpt), "%s/%s", cfg_root, mpt_name);
    this->scene_mpt_ = new mpt_obj();
    if (this->scene_mpt_->load_mpt(scene_cid, bf_mpt) != 0)
    {
      delete this->scene_mpt_;
      this->scene_mpt_ = NULL;
      return -1;
    }
    return 0;
  }
private:
  bool can_pk_;
  bool can_drop_;
  short lvl_limit_;
  int src_cid_;
  scene_coord_t relive_coord_;

  mpt_obj *scene_mpt_;
  std::set<int> disable_item_set_;
};
/**
 * @class scene_config_impl
 * 
 * @brief implement of scene_config
 */
class scene_config_impl : public load_json
{
  friend class scene_config;
public:
  scene_config_impl() :
    reload_(false),
    scene_cfg_obj_map_(MAX_SCENE_CID)
  { }
  ~scene_config_impl()
  { 
    for (int i = 0; i < this->scene_cfg_obj_map_.size(); ++i)
    {
      scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(i);
      if (sco != NULL) delete sco;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, SCENE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int scene_cid = ::atoi(iter.key().asCString());
      scene_cfg_obj *p = new scene_cfg_obj();
      this->scene_cfg_obj_map_.insert(scene_cid, p);
      if (p->load_config(*iter, cfg_root, scene_cid) != 0)
        return -1;
      this->scene_list_.push_back(scene_cid);
    }
    this->check_reload_new_scene();
    return 0;
  }
  void check_reload_new_scene()
  {
    if (!scene_config::instance()->is_reload()) return ;
    ilist<int> &old_scene_list = scene_config::instance()->scene_list();
    ilist<int> &new_scene_list = this->scene_list_;
    for (ilist_node<int> *itor = new_scene_list.head();
         itor != NULL;
         itor = itor->next_)
    {
      int scene_cid = itor->value_;
      if (!old_scene_list.find(scene_cid)
          && !clsid::is_scp_scene(scene_cid)
          && !clsid::is_tui_tu_scp_scene(scene_cid))
      {
        scene_mgr::instance()->insert(scene_cid,
                                      scene_cid,
                                      this->x_len(scene_cid),
                                      this->y_len(scene_cid));
      }
    }
  }
  short x_len(const int scene_cid)
  { 
    scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL) return -1;
    return sco->scene_mpt_->x_len_; 
  }
  short y_len(const int scene_cid)
  { 
    scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL) return -1;
    return sco->scene_mpt_->y_len_; 
  }
  inline char coord_value(const int scene_cid, const short x, const short y)
  {
    scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL) return -1;
    return sco->scene_mpt_->coord_value(x, y);
  }
  inline bool can_move(const int scene_cid, const short x, const short y)
  {
    scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL) return false;
    return sco->scene_mpt_->can_move(x, y);
  }
  inline bool can_pk(const int scene_cid)
  {
    scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL) return false;
    return sco->can_pk_;
  }
  inline bool can_drop(const int scene_cid)
  {
    scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL) return false;
    return sco->can_drop_;
  }
  inline bool can_use_item(const int scene_cid, const int item_cid)
  {
    scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL) return false;
    return sco->disable_item_set_.find(item_cid) == sco->disable_item_set_.end();
  }
  coord_t get_random_pos(const int scene_cid,
                         const short center_x,
                         const short center_y,
                         const short area_radius)
  {
    if (area_radius == 0)
      return coord_t(center_x, center_y);

    int idx = 0;
    int cnt = 8;
    while (cnt-- > 0)
    {
      idx = rand() % g_search_table_size;
      if (area_radius < MAX_SEARCH_TABLE_RANGE)
        idx = rand() % ((4 * area_radius) * (area_radius + 1));

      if (this->can_move(scene_cid,
                         center_x + g_search_table[idx].first_,
                         center_y + g_search_table[idx].second_))
        break;
    }
    if (cnt == -1)
      return coord_t(center_x, center_y);

    return coord_t(center_x + g_search_table[idx].first_,
                   center_y + g_search_table[idx].second_);
  }
  // center must in this scene_cid
  coord_t get_scene_random_pos(const int scene_cid,
                               const int center_x,
                               const int center_y)
  {
    const scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL)
      return coord_t(center_x, center_y);

    const short new_x = rand() % sco->scene_mpt_->x_len_;
    const short new_y = rand() % sco->scene_mpt_->y_len_;
    if (this->can_move(scene_cid, new_x, new_y))
      return coord_t(new_x, new_y);

    const short rand_area = CLT_VIEW_HEIGHT_RADIUS;
    coord_t ran_crd = this->get_random_pos(scene_cid, new_x, new_y, rand_area);
    if (ran_crd.x_ == new_x && ran_crd.y_ == new_y) // no find
      return this->get_random_pos(scene_cid, center_x, center_y, rand_area);
    return ran_crd;
  }
  inline short lvl_limit(const int scene_cid)
  {
    scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL) return 1;
    return sco->lvl_limit_;
  }
  inline scene_coord_t relive_coord(const int scene_cid)
  {
    scene_cfg_obj *sco = this->scene_cfg_obj_map_.find(scene_cid);
    if (sco == NULL) return scene_coord_t();
    return sco->relive_coord_;
  }
  inline ilist<int> &scene_list() { return this->scene_list_; }
private:
  bool reload_;
  array_t<scene_cfg_obj *> scene_cfg_obj_map_;
  ilist<int> scene_list_;
};
// ---------------------------------------------------------------------------
scene_config::scene_config() :
  impl_(new scene_config_impl())
{ 
  build_search_table();
}
int scene_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int scene_config::reload_config(const char *cfg_root)
{
  this->impl_->reload_ = true;
  scene_config_impl *tmp_impl = new scene_config_impl();
  if (tmp_impl->load_config(cfg_root) != 0)
  {
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
bool scene_config::is_reload()
{ return this->impl_->reload_; }
short scene_config::x_len(const int scene_cid)
{ return this->impl_->x_len(scene_cid); }
short scene_config::y_len(const int scene_cid)
{ return this->impl_->y_len(scene_cid); }
bool scene_config::can_move(const int scene_cid, const short x, const short y)
{ return this->impl_->can_move(scene_cid, x, y); }
char scene_config::coord_value(const int scene_cid, const short x, const short y)
{ return this->impl_->coord_value(scene_cid, x, y); }
coord_t scene_config::get_random_pos(const int scene_cid,
                                     const short center_x,
                                     const short center_y,
                                     const short area_radius)
{ return this->impl_->get_random_pos(scene_cid, center_x, center_y, area_radius); }
coord_t scene_config::get_scene_random_pos(const int scene_cid,
                                           const short center_x,
                                           const short center_y)
{ return this->impl_->get_scene_random_pos(scene_cid, center_x, center_y); }
short scene_config::lvl_limit(const int scene_cid)
{ return this->impl_->lvl_limit(scene_cid); }
bool scene_config::can_pk(const int scene_cid)
{ return this->impl_->can_pk(scene_cid); }
bool scene_config::can_drop(const int scene_cid)
{ return this->impl_->can_drop(scene_cid); }
bool scene_config::can_use_item(const int scene_cid, const int item_cid)
{ return this->impl_->can_use_item(scene_cid, item_cid); }
scene_coord_t scene_config::relive_coord(const int scene_cid)
{ return this->impl_->relive_coord(scene_cid); }
ilist<int> &scene_config::scene_list()
{ return this->impl_->scene_list(); }

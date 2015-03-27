#ifndef SNAP_OBJ_H_
#define SNAP_OBJ_H_

class snap_obj_mgr_impl;

const char T_CID_ID       = 1;
const char T_BASE_INFO    = 2;
const char T_STATUS       = 3;

/**
 * @class snap_obj
 *
 * @brief
 */
class snap_obj
{
public:
  snap_obj();

  char dir_;
  short coord_x_;
  short coord_y_;

  int obj_id_;
  int obj_cid_;

  int scene_id_;
  int scene_cid_;
};

/**
 * @class snap_obj_mgr
 *
 * @brief
 */
class snap_obj_mgr
{
public:
  snap_obj_mgr();

  void insert(const int obj_id, snap_obj *so);
  snap_obj *find(const int obj_id);
  int get_random_char_id();
private:
  snap_obj_mgr_impl *impl_;
};

#endif //SNAP_OBJ_H_

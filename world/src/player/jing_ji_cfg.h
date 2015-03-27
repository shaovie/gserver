// -*- C++ -*-

//========================================================================
/**
 * Author   : sevenwu
 * Date     : 2014-05-14 15:27
 */
//========================================================================

#ifndef JING_JI_CFG_H_
#define JING_JI_CFG_H_

// Lib header
#include "singleton.h"

#define JING_JI_AWARD_CFG_PATH        "jing_ji_award.json"
#define JING_JI_FIRST_AWARD_CFG_PATH  "jing_ji_first_award.json"

#define MAX_AWARD_KIND_COUNT          32
#define MAX_FIRST_AWARD_INDEX         32

class jing_ji_award_cfg_impl;
class jing_ji_first_award_cfg_impl;

/**
 * @class jj_award_obj
 *
 * @brief
 */
class jj_award_obj
{
public:
  jj_award_obj() :
    rank_min_(0),
    rank_max_(0),
    bind_diamond_(0),
    coin_(0),
    ji_fen_(0)
  {}

  int rank_min_;
  int rank_max_;
  int bind_diamond_;
  int coin_;
  int ji_fen_;
};

/**
 * @class jing_ji_award_cfg
 *
 * @brief
 */
class jing_ji_award_cfg : public singleton<jing_ji_award_cfg>
{
  friend class singleton<jing_ji_award_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  jj_award_obj *get_jj_award_obj(const int rank);
public:
  jing_ji_award_cfg();

  jing_ji_award_cfg_impl *impl_;
};

/**
 * @class jing_ji_first_award_cfg
 *
 * @brief
 */
class jing_ji_first_award_cfg : public singleton<jing_ji_first_award_cfg>
{
  friend class singleton<jing_ji_first_award_cfg>;
public:
  int load_config(const char *cfg_root);
  int reload_config(const char *cfg_root);

  int get_diamond(const int old_rank, const int new_rank);
  int get_coin(const int old_rank, const int new_rank);
  int get_ji_fen(const int old_rank, const int new_rank);
public:
  jing_ji_first_award_cfg();

  jing_ji_first_award_cfg_impl *impl_;
};

#endif //JING_JI_CFG_H_

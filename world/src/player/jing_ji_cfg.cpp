#include "jing_ji_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "array_t.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("config");
static ilog_obj *e_log = err_log::instance()->get_ilog("config");

/**
 * @class jing_ji_award_cfg_impl
 *
 * @brief
 */
class jing_ji_award_cfg_impl : public load_json
{
public:
  jing_ji_award_cfg_impl() :
    jj_award_obj_map_(MAX_AWARD_KIND_COUNT)
  { }
  ~jing_ji_award_cfg_impl()
  {
    for (int i = 0; i < this->jj_award_obj_map_.size(); ++i)
    {
      jj_award_obj *jao = this->jj_award_obj_map_.find(i);
      if (jao != NULL) delete jao;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, JING_JI_AWARD_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      if (id >= MAX_AWARD_KIND_COUNT) return -1;

      jj_award_obj *jao = new jj_award_obj();
      this->jj_award_obj_map_.insert(id, jao);

      jao->rank_min_     = (*iter)["rank_min"].asInt();
      jao->rank_max_     = (*iter)["rank_max"].asInt();
      jao->bind_diamond_ = (*iter)["bind_zuanshi"].asInt();
      jao->coin_         = (*iter)["coin"].asInt();
      jao->ji_fen_       = (*iter)["ji_fen"].asInt();
    }

    return 0;
  }
  jj_award_obj *get_jj_award_obj(const int rank)
  {
    for (int i = 0; i < this->jj_award_obj_map_.size(); ++i)
    {
      jj_award_obj *jao = this->jj_award_obj_map_.find(i);
      if (jao == NULL
          || rank < jao->rank_min_
          || rank > jao->rank_max_)
        continue;

      return jao;
    }
    return NULL;
  }
private:
  array_t<jj_award_obj *> jj_award_obj_map_;
};
jing_ji_award_cfg::jing_ji_award_cfg() : impl_(new jing_ji_award_cfg_impl()) { }
int jing_ji_award_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int jing_ji_award_cfg::reload_config(const char *cfg_root)
{
  jing_ji_award_cfg_impl *tmp_impl = new jing_ji_award_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", JING_JI_AWARD_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
jj_award_obj *jing_ji_award_cfg::get_jj_award_obj(const int rank)
{ return this->impl_->get_jj_award_obj(rank); }
//==========================================================================
/**
 * @class jj_first_award_obj
 *
 * @brief
 */
class jj_first_award_obj
{
public:
  jj_first_award_obj() :
    rank_min_(0),
    rank_max_(0),
    bind_diamond_(0),
    coin_(0),
    ji_fen_(0)
  { }

  int rank_min_;
  int rank_max_;
  int bind_diamond_;
  int coin_;
  int ji_fen_;
};
/**
 * @class jing_ji_first_award_cfg_impl
 *
 * @brief
 */
class jing_ji_first_award_cfg_impl : public load_json
{
public:
  jing_ji_first_award_cfg_impl() :
    jj_first_award_obj_map_(MAX_FIRST_AWARD_INDEX)
  { }
  ~jing_ji_first_award_cfg_impl()
  {
    for (int i = 0; i < this->jj_first_award_obj_map_.size(); ++i)
    {
      jj_first_award_obj *jfao = this->jj_first_award_obj_map_.find(i);
      if (jfao != NULL) delete jfao;
    }
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, JING_JI_FIRST_AWARD_CFG_PATH, root) != 0)
      return -1;

    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      if (id >= MAX_FIRST_AWARD_INDEX) return -1;

      jj_first_award_obj *jfao = new jj_first_award_obj();
      this->jj_first_award_obj_map_.insert(id, jfao);

      jfao->rank_min_     = (*iter)["rank_min"].asInt();
      jfao->rank_max_     = (*iter)["rank_max"].asInt();
      jfao->bind_diamond_ = (*iter)["bind_zuanshi"].asInt();
      jfao->coin_         = (*iter)["coin"].asInt();
      jfao->ji_fen_       = (*iter)["ji_fen"].asInt();
    }

    return 0;
  }
  int get_diamond(const int old_rank, const int new_rank)
  {
    int old_index = 0;
    int new_index = 0;

    if (old_rank == 0)
      old_index = MAX_FIRST_AWARD_INDEX;

    for (int i = 0; i < this->jj_first_award_obj_map_.size(); ++i)
    {
      jj_first_award_obj *jfao = this->jj_first_award_obj_map_.find(i);
      if (jfao == NULL) continue;

      if (old_rank >= jfao->rank_min_ && old_rank <= jfao->rank_max_)
        old_index = i;

      if (new_rank >= jfao->rank_min_ && new_rank <= jfao->rank_max_)
        new_index = i;

      if (old_index != 0 && new_index != 0) break;
    }
    int bind_diamond = 0;
    for (int i = new_index; i < old_index; ++i)
    {
      jj_first_award_obj *jfao = this->jj_first_award_obj_map_.find(i);
      if (jfao == NULL) continue;

      bind_diamond += jfao->bind_diamond_;
    }
    return bind_diamond;
  }
  int get_coin(const int old_rank, const int new_rank)
  {
    int old_index = 0;
    int new_index = 0;

    if (old_rank == 0)
      old_index = MAX_FIRST_AWARD_INDEX;

    for (int i = 0; i < this->jj_first_award_obj_map_.size(); ++i)
    {
      jj_first_award_obj *jfao = this->jj_first_award_obj_map_.find(i);
      if (jfao == NULL) continue;

      if (old_rank >= jfao->rank_min_ && old_rank <= jfao->rank_max_)
        old_index = i;

      if (new_rank >= jfao->rank_min_ && new_rank <= jfao->rank_max_)
        new_index = i;

      if (old_index != 0 && new_index != 0) break;
    }
    int coin = 0;
    for (int i = new_index; i < old_index; ++i)
    {
      jj_first_award_obj *jfao = this->jj_first_award_obj_map_.find(i);
      if (jfao == NULL) continue;

      coin += jfao->coin_;
    }
    return coin;
  }
  int get_ji_fen(const int old_rank, const int new_rank)
  {
    int old_index = 0;
    int new_index = 0;

    if (old_rank == 0)
      old_index = MAX_FIRST_AWARD_INDEX;

    for (int i = 0; i < this->jj_first_award_obj_map_.size(); ++i)
    {
      jj_first_award_obj *jfao = this->jj_first_award_obj_map_.find(i);
      if (jfao == NULL) continue;

      if (old_rank >= jfao->rank_min_ && old_rank <= jfao->rank_max_)
        old_index = i;

      if (new_rank >= jfao->rank_min_ && new_rank <= jfao->rank_max_)
        new_index = i;

      if (old_index != 0 && new_index != 0) break;
    }
    int ji_fen = 0;
    for (int i = new_index; i < old_index; ++i)
    {
      jj_first_award_obj *jfao = this->jj_first_award_obj_map_.find(i);
      if (jfao == NULL) continue;

      ji_fen += jfao->ji_fen_;
    }
    return ji_fen;
  }
private:
  array_t<jj_first_award_obj *> jj_first_award_obj_map_;
};
jing_ji_first_award_cfg::jing_ji_first_award_cfg() : impl_(new jing_ji_first_award_cfg_impl()) { }
int jing_ji_first_award_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int jing_ji_first_award_cfg::reload_config(const char *cfg_root)
{
  jing_ji_first_award_cfg_impl *tmp_impl = new jing_ji_first_award_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", JING_JI_FIRST_AWARD_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  //
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
int jing_ji_first_award_cfg::get_diamond(const int old_rank, const int new_rank)
{ return this->impl_->get_diamond(old_rank, new_rank); }
int jing_ji_first_award_cfg::get_coin(const int old_rank, const int new_rank)
{ return this->impl_->get_coin(old_rank, new_rank); }
int jing_ji_first_award_cfg::get_ji_fen(const int old_rank, const int new_rank)
{ return this->impl_->get_ji_fen(old_rank, new_rank); }

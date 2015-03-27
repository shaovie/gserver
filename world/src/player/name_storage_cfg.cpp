#include "name_storage_cfg.h"
#include "load_json.h"
#include "sys_log.h"
#include "def.h"
#include "global_macros.h"
#include "all_char_info.h"
#include "util.h"

// Lib header
#include <set>

static ilog_obj *e_log = err_log::instance()->get_ilog("config");
static ilog_obj *s_log = err_log::instance()->get_ilog("config");

const std::string emptystr = "";

/**
 * @class name_storage_cfg_impl
 *
 * @brief
 */
class name_storage_cfg_impl : public load_json
{
public:
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, NAME_STORAGE_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator itor = root.begin();
        itor != root.end();
        ++itor)
    {
      const char *name = (*itor)["hot_name"].asCString();
      if (::strlen(name) > 0
          && ::strlen(name) <= MAX_PURE_NAME_LEN
          && util::verify_name(name))
      {
        if (!this->store_name_.insert(std::string(name)).second)
          e_log->error("name -->%s<-- is repetitive!", name);
      }else
      {
        e_log->error("name -->%s<-- is illegal!", name);
        return -1;
      }
      if ((*itor).isMember("first_name"))
      {
        const char *f_name = (*itor)["first_name"].asCString();
        if (::strlen(f_name) > 0
            && ::strlen(f_name) <= MAX_PURE_NAME_LEN
            && util::verify_name(f_name))
          this->first_name_.insert(std::string(f_name));
      }
      if ((*itor).isMember("mid_name"))
      {
        const char *m_name = (*itor)["mid_name"].asCString();
        if (::strlen(m_name) > 0
            && ::strlen(m_name) <= MAX_PURE_NAME_LEN
            && util::verify_name(m_name))
          this->mid_name_.insert(std::string(m_name));
      }
      if ((*itor).isMember("last_name"))
      {
        const char *l_name = (*itor)["last_name"].asCString();
        if (::strlen(l_name) > 0
            && ::strlen(l_name) <= MAX_PURE_NAME_LEN
            && util::verify_name(l_name))
          this->last_name_.insert(std::string(l_name));
      }
    }
    return 0;
  }

  void adjust_data()
  {
    std::set<std::string>::iterator iter = this->store_name_.begin();
    while (iter != this->store_name_.end())
    {
      if (all_char_info::instance()->get_char_brief_info((*iter).c_str()) == NULL)
        ++ iter;
      else
        this->store_name_.erase(iter ++);
    }
    //s_log->rinfo("There is %d spare hot_name!", this->store_name_.size());
    //s_log->rinfo("There is %d spare first_name!", this->first_name_.size());
    //s_log->rinfo("There is %d spare mid_name!", this->mid_name_.size());
    //s_log->rinfo("There is %d spare last_name!", this->last_name_.size());
  }

  std::string get_name()
  {
    if (this->store_name_.empty() && !this->rec_name_.empty())
      this->store_name_.swap(this->rec_name_);

    int left_name = this->store_name_.size() + this->rec_name_.size();
    if (left_name < 30 && rand() % 30 >= left_name)
      return this->get_rand_name();

    int randnum = rand() % this->store_name_.size() + 1;
    std::set<std::string>::iterator iter = this->store_name_.begin();
    while (-- randnum) ++iter;
    std::pair<std::set<std::string>::iterator, bool> res = this->rec_name_.insert(*iter);
    this->store_name_.erase(iter);
    return (res.second ? (*res.first) : emptystr);
  }
  void on_create_char(const char *name)
  {
    std::string new_name(name);
    std::set<std::string>::iterator iter = this->rec_name_.find(new_name);
    if (iter != this->rec_name_.end())
    {
      this->rec_name_.erase(iter);
    }else
    {
      iter = this->store_name_.find(new_name);
      if (iter != this->store_name_.end())
        this->store_name_.erase(iter);
    }
  }
private:
  std::string get_rand_name()
  {
    std::string com_name = emptystr;
    if (!this->first_name_.empty())
    {
      int randnum = rand() % this->first_name_.size() + 1;
      std::set<std::string>::iterator iter = this->first_name_.begin();
      while(--randnum) ++ iter;
      if (com_name.length() + (*iter).length() > MAX_PURE_NAME_LEN)
        return com_name;
      com_name += *iter;
    }
    if (!this->mid_name_.empty())
    {
      int randnum = rand() % this->mid_name_.size() + 1;
      std::set<std::string>::iterator iter = this->mid_name_.begin();
      while(--randnum) ++ iter;
      if (com_name.length() + (*iter).length() > MAX_PURE_NAME_LEN)
        return com_name;
      com_name += *iter;
    }
    if (!this->last_name_.empty())
    {
      int randnum = rand() % this->last_name_.size() + 1;
      std::set<std::string>::iterator iter = this->last_name_.begin();
      while(--randnum) ++ iter;
      if (com_name.length() + (*iter).length() > MAX_PURE_NAME_LEN)
        return com_name;
      com_name += *iter;
    }
    return com_name;
  }

  std::set<std::string> rec_name_;
  std::set<std::string> store_name_;

  std::set<std::string> first_name_;
  std::set<std::string> mid_name_;
  std::set<std::string> last_name_;
};

name_storage_cfg::name_storage_cfg() : impl_(new name_storage_cfg_impl()) { }
int name_storage_cfg::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int name_storage_cfg::reload_config(const char *cfg_root)
{
  name_storage_cfg_impl *tmp_impl = new name_storage_cfg_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", NAME_STORAGE_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  this->impl_->adjust_data();
  return 0;
}
void name_storage_cfg::adjust_data()
{ this->impl_->adjust_data(); }
std::string name_storage_cfg::get_name()
{ return this->impl_->get_name(); }
void name_storage_cfg::on_create_char(const char *name)
{ this->impl_->on_create_char(name); }

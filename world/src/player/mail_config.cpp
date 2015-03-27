#include "mail_config.h"
#include "load_json.h"
#include "sys_log.h"
#include "array_t.h"
#include "util.h"

// Lib header

static ilog_obj *e_log = err_log::instance()->get_ilog("config");
static ilog_obj *s_log = err_log::instance()->get_ilog("config");

#define MAX_MAIL_CFG_ID  128

mail_obj::mail_obj()
{
  ::memset(this->sender_name_, 0, sizeof(this->sender_name_));
  ::memset(this->title_, 0, sizeof(this->title_));
  ::memset(this->content_, 0, sizeof(this->content_));
}
/**
 * @class mail_config_impl
 *
 * @brief
 */
class mail_config_impl : public load_json
{
public:
  mail_config_impl() : mail_obj_map_(MAX_MAIL_CFG_ID) { }
  ~mail_config_impl()
  {
    for (int i = 0; i < this->mail_obj_map_.size(); ++i)
    {
      mail_obj *mo= this->mail_obj_map_.find(i);
      if (mo != NULL) delete mo;
    }
  }

  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, MAIL_CFG_PATH, root) != 0)
      return -1;

    for(Json::Value::iterator iter = root.begin();
        iter != root.end();
        ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      mail_obj *mo = new mail_obj();
      this->mail_obj_map_.insert(id, mo);
      if (this->load_json(*iter, mo) != 0)
        return -1;
    }
    return 0;
  }
  int load_json(Json::Value &v, mail_obj *mo)
  {
    ::strncpy(mo->sender_name_, v["name"].asCString(), sizeof(mo->sender_name_) - 1);
    if (!util::verify_name(mo->sender_name_)
        || ::strlen(v["name"].asCString()) >= sizeof(mo->sender_name_) - 1)
    {
      e_log->debug("mail name [%s] illegality character!", v["name"].asCString());
      return -1;
    }
    ::strncpy(mo->title_, v["title"].asCString(), sizeof(mo->title_) - 1);
    if (!util::verify_db(mo->title_)
        || ::strlen(v["title"].asCString()) >= sizeof(mo->title_) - 1)
    {
      e_log->debug("mail title [%s] illegality character!", v["title"].asCString());
      return -1;
    }
    ::strncpy(mo->content_, v["content"].asCString(), sizeof(mo->content_) - 1);
    if (!util::verify_db(mo->content_)
        || ::strlen(v["content"].asCString()) >= sizeof(mo->content_) - 1)
    {
      e_log->debug("mail content [%s] illegality character!", v["content"].asCString());
      return -1;
    }
    return 0;
  }
  const mail_obj *get_mail_obj(const int mail_id)
  { return this->mail_obj_map_.find(mail_id); }
private:
  array_t<mail_obj *> mail_obj_map_;  
};
mail_config::mail_config() : impl_(new mail_config_impl()) { }
int mail_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); }
int mail_config::reload_config(const char *cfg_root)
{
  mail_config_impl *tmp_impl = new mail_config_impl();
  if (tmp_impl->load_config(cfg_root) == -1)
  {
    e_log->error("reload %s failed!", MAIL_CFG_PATH);
    delete tmp_impl;
    return -1;
  }
  delete this->impl_;
  this->impl_ = tmp_impl;
  return 0;
}
const mail_obj *mail_config::get_mail_obj(const int mail_id) 
{ return this->impl_->get_mail_obj(mail_id); }

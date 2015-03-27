#include "svc_config.h"
#include "load_json.h"
#include "global_macros.h"

// Lib header
#include <string.h>

/**
 * @class svc_config_impl
 *
 * @brief
 */
class svc_config_impl : public load_json
{
public:
  svc_config_impl()
  {
    service_sn_ = 1;
    max_online_ = 0;
    ::memset(cfg_root_, 0, sizeof(cfg_root_));
    ::memset(group_, 0, sizeof(group_));

    db_port_ = 0;
    ::memset(db_host_, 0, sizeof(db_host_));
    ::memset(db_name_, 0, sizeof(db_name_));
    ::memset(db_user_, 0, sizeof(db_user_));
    ::memset(db_passwd_, 0, sizeof(db_passwd_));

    //
    db_proxy_port_ = 0;
    ::memset(db_proxy_host_, 0, sizeof(db_proxy_host_));

    //
    world_port_ = 0;
    world_gm_port_ = 0;
    ::memset(world_gm_host_, 0, sizeof(world_gm_host_));

    //
    cross_port_ = 0;
    ::memset(cross_host_, 0, sizeof(cross_host_));
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, SVC_CFG_PATH, root) != 0)
      return -1;

    ::strncpy(this->group_, root["group"].asCString(), sizeof(this->group_)-1);
    this->service_sn_ = root["sn"].asInt();
    this->max_online_ = root["max_online"].asInt();
    if (this->service_sn_ < 0 || this->service_sn_ > MAX_SERVICE_SN)
      return -1;
    ::strncpy(this->cfg_root_, root["cfg_root"].asCString(), sizeof(this->cfg_root_)-1);
    Json::Value &db_v = root["db"];
    {
      Json::Value &port_v = db_v["port"];
      if (port_v.empty()) return -1;
      this->db_port_ = port_v.asInt();

      Json::Value &host_v = db_v["host"];
      if (host_v.empty()) return -1;
      ::strncpy(this->db_host_, host_v.asCString(), sizeof(this->db_host_)-1);

      Json::Value &user_v = db_v["user"];
      if (user_v.empty()) return -1;
      ::strncpy(this->db_user_, user_v.asCString(), sizeof(this->db_user_)-1);

      Json::Value &passwd_v = db_v["passwd"];
      if (passwd_v.empty()) return -1;
      ::strncpy(this->db_passwd_, passwd_v.asCString(), sizeof(this->db_passwd_)-1);

      Json::Value &db_name_v = db_v["db_name"];
      if (db_name_v.empty()) return -1;
      ::strncpy(this->db_name_, db_name_v.asCString(), sizeof(this->db_name_)-1);
    }

    Json::Value &db_proxy_v = root["db_proxy"];
    {
      Json::Value &port_v = db_proxy_v["port"];
      if (port_v.empty()) return -1;
      this->db_proxy_port_ = port_v.asInt();

      Json::Value &host_v = db_proxy_v["host"];
      if (host_v.empty()) return -1;
      ::strncpy(this->db_proxy_host_, host_v.asCString(), sizeof(this->db_proxy_host_));
    }

    Json::Value &world_v = root["world"];
    {
      Json::Value &port_v = world_v["port"];
      if (port_v.empty()) return -1;
      this->world_port_ = port_v.asInt();

      Json::Value &gm_port_v = world_v["gm_port"];
      if (gm_port_v.empty()) return -1;
      this->world_gm_port_ = gm_port_v.asInt();

      Json::Value &gm_host_v = world_v["gm_host"];
      if (gm_host_v.empty()) return -1;
      ::strncpy(this->world_gm_host_, gm_host_v.asCString(), sizeof(this->world_gm_host_)-1);
    }

    Json::Value &cross_v = root["cross"];
    {
      Json::Value &port_v = cross_v["port"];
      if (port_v.empty()) return -1;
      this->cross_port_ = port_v.asInt();

      Json::Value &host_v = cross_v["host"];
      if (host_v.empty()) return -1;
      ::strncpy(this->cross_host_, host_v.asCString(), sizeof(this->cross_host_)-1);
    }
    return 0;
  }
  const char* group(){ return this->group_; }
  int service_sn() { return this->service_sn_; }
  int max_online() { return this->max_online_; }
  const char* cfg_root() { return this->cfg_root_; }

  int db_port() { return this->db_port_; }
  const char *db_host() { return this->db_host_; }
  const char *db_user() { return this->db_user_; }
  const char *db_passwd() { return this->db_passwd_; }
  const char *db_name() { return this->db_name_; }

  int db_proxy_port() { return this->db_proxy_port_; }
  const char *db_proxy_host() { return this->db_proxy_host_; }

  int world_port() { return this->world_port_; }
  int world_gm_port() { return this->world_gm_port_; }
  const char *world_gm_host() { return this->world_gm_host_; }

  int cross_port() { return this->cross_port_; }
  const char *cross_host() { return this->cross_host_; }
private:
  int service_sn_;
  int max_online_;
  char cfg_root_[MAX_FILE_PATH_LEN + 1];
  char group_[MAX_GROUP_NAME_LEN + 1];

  //= db
  int  db_port_;
  char db_host_[MAX_HOST_NAME + 1];
  char db_user_[MAX_NAME_LEN + 1];
  char db_passwd_[MAX_NAME_LEN + 1];
  char db_name_[MAX_NAME_LEN + 1];

  //= db proxy
  int  db_proxy_port_;
  char db_proxy_host_[MAX_HOST_NAME + 1];

  //= world
  int  world_port_;
  int  world_gm_port_;
  char world_gm_host_[MAX_HOST_NAME + 1];

  //= cross
  int cross_port_;
  char cross_host_[MAX_HOST_NAME + 1];
};

// ------------------------------- svc config ------------------------------
svc_config::svc_config()
: impl_(new svc_config_impl())
{ }
int svc_config::load_config(const char *cfg_root)
{ return this->impl_->load_config(cfg_root); } 

const char* svc_config::group()
{ return this->impl_->group(); }
int svc_config::service_sn()
{ return this->impl_->service_sn(); }
int svc_config::max_online()
{ return this->impl_->max_online(); }
const char* svc_config::cfg_root()
{ return this->impl_->cfg_root(); }

int svc_config::db_port()
{ return this->impl_->db_port(); }
const char *svc_config::db_host()
{ return this->impl_->db_host(); }
const char *svc_config::db_user()
{ return this->impl_->db_user(); }
const char *svc_config::db_passwd()
{ return this->impl_->db_passwd(); }
const char *svc_config::db_name()
{ return this->impl_->db_name(); }

int svc_config::db_proxy_port()
{ return this->impl_->db_proxy_port(); }
const char *svc_config::db_proxy_host()
{ return this->impl_->db_proxy_host(); }

int svc_config::world_port()
{ return this->impl_->world_port(); }
int svc_config::world_gm_port()
{ return this->impl_->world_gm_port(); }
const char *svc_config::world_gm_host()
{ return this->impl_->world_gm_host(); }

int svc_config::cross_port()
{ return this->impl_->cross_port(); }
const char *svc_config::cross_host()
{ return this->impl_->cross_host(); }


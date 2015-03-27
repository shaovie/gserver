#include "mysql_db.h"

// Lib header
#include <cstring>
#include <stdio.h>
#include <sys/param.h>

#include "mysql/errmsg.h"  // mysql

/**
 * @class mysql_db_impl
 * 
 * @brief implement of mysql_db
 */
class mysql_db_impl
{
  friend class mysql_db;
private:
  mysql_db_impl()
    : my_conn_ok_(false),
    my_port_(0),
    mysql_(NULL)
  {
    this->my_host_[MAXHOSTNAMELEN - 1] = '\0';
    this->my_uesr_[NAME_MAX]           = '\0';
    this->my_passwd_[NAME_MAX]         = '\0';
    this->my_db_name_[NAME_MAX]        = '\0';
  }
  ~mysql_db_impl()
  { this->close(); }
  int open(const short port,
           const char *host,
           const char *user,
           const char *passwd,
           const char *db_name)
  {
    this->my_port_ = port;
    ::strncpy(this->my_host_, host, MAXHOSTNAMELEN - 1);
    ::strncpy(this->my_uesr_, user, NAME_MAX);
    ::strncpy(this->my_passwd_, passwd, NAME_MAX);
    ::strncpy(this->my_db_name_, db_name, NAME_MAX);

    return this->connect();
  }
  void close()
  {
    if (this->mysql_ != NULL)
      mysql_close(this->mysql_);
    this->mysql_ = NULL;
  }
  int connect()
  {
    this->close();

    this->mysql_ = mysql_init(NULL);

    int timeout = 2;
    mysql_options(this->mysql_, MYSQL_OPT_READ_TIMEOUT, (char *)&timeout); 
    mysql_options(this->mysql_, MYSQL_OPT_CONNECT_TIMEOUT, (char *)&timeout); 

    if (!mysql_real_connect(this->mysql_, 
                            this->my_host_,
                            this->my_uesr_,
                            this->my_passwd_,
                            this->my_db_name_,
                            this->my_port_, 
                            NULL, 
                            0))
    {
      this->close();
      return -1;
    }

    this->my_conn_ok_ = true;
    return 0;
  }

  const char *mysql_strerror() 
  {
    if (this->mysql_ == NULL) return NULL;
    return mysql_error(this->mysql_); 
  }
  int mysql_err() 
  {
    if (this->mysql_ == NULL) return -1;
    return mysql_errno(this->mysql_); 
  }

  MYSQL_RES *query(const char *sql, const int sql_len)
  {
    int retry_count = 1;
    MYSQL_RES *res = NULL;
    do
    {
      if (this->my_conn_ok_ || this->connect() == 0)
      {
        int result = mysql_real_query(this->mysql_, sql, sql_len);
        if (result != 0)
        {
          unsigned int err = mysql_errno(this->mysql_);
          if (err == CR_SERVER_LOST || err == CR_SERVER_GONE_ERROR)
            this->my_conn_ok_ = false;
        }else
        {
          res = mysql_store_result(this->mysql_); 
          if (res == NULL)
          {
            unsigned int err = mysql_errno(this->mysql_);
            if (err == CR_SERVER_LOST || err == CR_SERVER_GONE_ERROR)
              this->my_conn_ok_ = false;
          }
        }
      }
    }while (!this->my_conn_ok_ && retry_count-- > 0);

    return res;
  }
  int store(const char *sql, const int sql_len)
  {
    int result = 0;
    int retry_count = 1;
    do
    {
      if (this->my_conn_ok_ || this->connect() == 0)
      {
        result = mysql_real_query(this->mysql_, sql, sql_len);
        if (result != 0)
        {
          unsigned int err = mysql_errno(this->mysql_);
          if (err == CR_SERVER_LOST || err == CR_SERVER_GONE_ERROR)
            this->my_conn_ok_ = false;
        }
      }
    }while (!this->my_conn_ok_ && retry_count-- > 0);

    return result == 0 ? 0 : -1;
  }
  MYSQL *mysql_ptr()
  { return this->mysql_; }
  void ping()
  {
    if (this->my_conn_ok_ || this->connect() == 0)
      mysql_ping(this->mysql_);
  }
private:
  bool my_conn_ok_;
  
  short my_port_;

  MYSQL *mysql_;

  char my_host_[MAXHOSTNAMELEN];     // 64
  char my_uesr_[NAME_MAX + 1];       // 255 + 1
  char my_passwd_[NAME_MAX + 1];     // 255 + 1
  char my_db_name_[NAME_MAX + 1];    // 255 + 1
};
// -------------------------------------------------------------------------
mysql_db::mysql_db()
: impl_(new mysql_db_impl())
{ }
mysql_db::~mysql_db()
{ delete this->impl_; }
int mysql_db::open(const short port,
                   const char *host,
                   const char *user,
                   const char *passwd,
                   const char *db_name)
{ return this->impl_->open(port, host, user, passwd, db_name); }
void mysql_db::close()
{ this->impl_->close(); }
int mysql_db::mysql_err()
{ return this->impl_->mysql_err(); }
const char *mysql_db::mysql_strerror()
{ return this->impl_->mysql_strerror(); }
MYSQL_RES *mysql_db::query(const char *sql, const int sql_len)
{ return this->impl_->query(sql, sql_len); }
int mysql_db::store(const char *sql, const int sql_len)
{ return this->impl_->store(sql, sql_len); }
MYSQL *mysql_db::mysql_ptr()
{ return this->impl_->mysql_ptr(); }
void mysql_db::ping()
{ return this->impl_->ping(); }

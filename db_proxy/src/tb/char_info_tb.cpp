#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "char_info.h"
#include "proxy_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    void read_sql_row(char_info *data, MYSQL_ROW &row)
    {
      data->char_id_     = ::atoi(row[char_info::IDX_CHAR_ID]);
      ::strncpy(data->account_, row[char_info::IDX_ACCOUNT], sizeof(data->account_) - 1);
      ::strncpy(data->name_, row[char_info::IDX_NAME], sizeof(data->name_) - 1);
      data->career_      = ::atoi(row[char_info::IDX_CAREER]);
      data->exp_         = ::atol(row[char_info::IDX_EXP]);
      data->lvl_         = ::atoi(row[char_info::IDX_LVL]);
      data->coin_        = ::atoi(row[char_info::IDX_COIN]);
      data->b_diamond_   = ::atoi(row[char_info::IDX_B_DIAMOND]);
      data->hp_          = ::atoi(row[char_info::IDX_HP]);
      data->mp_          = ::atoi(row[char_info::IDX_MP]);
      data->zhan_li_     = ::atoi(row[char_info::IDX_ZHAN_LI]);
      data->dir_         = ::atoi(row[char_info::IDX_DIR]);
      data->coord_x_     = ::atoi(row[char_info::IDX_COORD_X]);
      data->coord_y_     = ::atoi(row[char_info::IDX_COORD_Y]);
      data->scene_id_    = ::atoi(row[char_info::IDX_SCENE_ID]);
      data->scene_cid_   = ::atoi(row[char_info::IDX_SCENE_CID]);
      data->last_coord_x_ = ::atoi(row[char_info::IDX_LAST_COORD_X]);
      data->last_coord_y_ = ::atoi(row[char_info::IDX_LAST_COORD_Y]);
      data->last_scene_cid_ = ::atoi(row[char_info::IDX_LAST_SCENE_CID]);
      data->pk_mode_     = ::atoi(row[char_info::IDX_PK_MODE]);
      data->sin_val_     = ::atoi(row[char_info::IDX_SIN_VAL]);
      data->sin_ol_time_ = ::atoi(row[char_info::IDX_SIN_OL_TIME]);
      data->pkg_cap_     = ::atoi(row[char_info::IDX_PKG_CAP]);
      data->storage_cap_ = ::atoi(row[char_info::IDX_STORAGE_CAP]);
      data->cur_title_   = ::atoi(row[char_info::IDX_CUR_TITLE]);
      ::strncpy(data->ip_, row[char_info::IDX_IP], sizeof(data->ip_) - 1);
      data->ol_time_     = ::atoi(row[char_info::IDX_OL_TIME]);
      data->c_time_      = ::atoi(row[char_info::IDX_C_TIME]);
      data->in_time_     = ::atoi(row[char_info::IDX_IN_TIME]);
      data->out_time_    = ::atoi(row[char_info::IDX_OUT_TIME]);
    }
    char_info data_;
  };
  class tb_msg_insert : public tb_msg_handler
  {
    class sql_insert : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_INSERT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.insert_sql(bf, bf_len); }
    };
  public:
    tb_msg_insert(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(char_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(char_info));

      sql_insert *sql = new sql_insert();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(char_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_CHAR_INFO, new tb_msg_insert(RES_INSERT_CHAR_INFO));

  class tb_msg_get : public tb_msg_handler
  {
    class sql_get : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;
        mblock *mb = mblock_pool::instance()->alloc(sizeof(char_info));
        char_info *p = (char_info *)mb->data();
        p->reset();
        this->read_sql_row(p, row);
        mb->wr_ptr(sizeof(char_info));
        ar->push_back(mb);
      }
    };
  public:
    tb_msg_get(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_get *sql = new sql_get();
      in_stream is(msg, len);
      is >> db_sid >> sql->data_.char_id_;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get(REQ_GET_CHAR_INFO, new tb_msg_get(RES_GET_CHAR_INFO));
  static tb_msg_maper tm_get_new(REQ_GET_NEW_CHAR_INFO, new tb_msg_get(RES_GET_NEW_CHAR_INFO));

  class tb_msg_update : public tb_msg_handler
  {
    class sql_update : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_UPDATE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.update_sql(bf, bf_len); }
    };
  public:
    tb_msg_update(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(char_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(char_info));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(char_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_CHAR_INFO, new tb_msg_update(RES_UPDATE_CHAR_INFO));

  class tb_msg_search : public tb_msg_handler
  {
    class sql_search : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_search_sql(bf, bf_len); }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = NULL;
        while((row = mysql_fetch_row(res)) != NULL)
        {
          mblock *mb = mblock_pool::instance()->alloc(sizeof(int));
          *(int *)mb->data() = ::atoi(row[0]);
          mb->wr_ptr(sizeof(int));
          ar->push_back(mb);
        }
      }
    };
  public:
    tb_msg_search(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      sql_search *sql = new sql_search();
      in_stream is(msg, len);
      stream_istr name(sql->data_.name_, sizeof(sql->data_.name_));
      is >> db_sid >> name;

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_search(REQ_GET_SEARCH_INFO, new tb_msg_search(RES_GET_SEARCH_INFO));
}

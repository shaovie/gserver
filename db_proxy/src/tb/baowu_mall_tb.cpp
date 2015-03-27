#include "db_async_store.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "proxy_obj.h"
#include "baowu_mall_info.h"
#include "sys_log.h"
#include "istream.h"
#include "message.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    baowu_mall_info data_;
  };

  class tb_msg_get : public tb_msg_handler
  {
    class sql_get : public sql
    {
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return;
        if (row != NULL)
        {
          mblock *mb = mblock_pool::instance()->alloc(sizeof(baowu_mall_info));
          baowu_mall_info *bmi = (baowu_mall_info *)mb->data();
          bmi->reset();
          this->read_sql_row(bmi, row);
          mb->wr_ptr(sizeof(baowu_mall_info));
          ar->push_back(mb);
        }
      }
      void read_sql_row(baowu_mall_info *data, MYSQL_ROW &row)
      {
        data->char_id_      = ::atoi(row[baowu_mall_info::IDX_CHAR_ID]);
        data->refresh_time_ = ::atoi(row[baowu_mall_info::IDX_REFRESH_TIME]);

        char str[128] = {0};
        ::strncpy(str, row[baowu_mall_info::IDX_ITEM_INFO], sizeof(str) - 1);
        char *tok_p = NULL;
        char *token = NULL;
        int i = 0;
        for (token = ::strtok_r(str, ",", &tok_p);
             token != NULL && i < BAOWU_MALL_ITEM_COUNT;
             token = ::strtok_r(NULL, ",", &tok_p), ++i)
        {
          ::sscanf(token, "%d:%d",
                   &data->item_info_[i][0], &data->item_info_[i][1]);
        }
      }
    };
  public:
    tb_msg_get(const int res) : resp_id_(res) { }
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
  static tb_msg_maper tm_get(REQ_GET_BAOWU_MALL_INFO, new tb_msg_get(RES_GET_BAOWU_MALL_INFO));

  class tb_msg_update : public tb_msg_handler
  {
    class sql_update : public sql
    {
      virtual int opt_type() { return sql_opt::SQL_UPDATE; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.update_sql(bf, bf_len); }
    };
  public:
    tb_msg_update(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(baowu_mall_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(baowu_mall_info));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(baowu_mall_info));
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_BAOWU_MALL_INFO, new tb_msg_update(RES_UPDATE_BAOWU_MALL_INFO));
}

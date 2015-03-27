#include "db_async_store.h"
#include "daily_clean_info.h"
#include "mblock_pool.h"
#include "tb_msg_map.h"
#include "proxy_obj.h"
#include "sys_log.h"
#include "message.h"
#include "istream.h"
#include "icache.h"

static ilog_obj *s_log = sys_log::instance()->get_ilog("base");
static ilog_obj *e_log = err_log::instance()->get_ilog("base");

namespace
{
  static icache<int, daily_clean_info, obj_pool_std_allocator<daily_clean_info> > s_cache(CACHE_OBJ_SIZE);

  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    daily_clean_info data_;
  };
  //=
  class tb_msg_get : public tb_msg_handler
  {
    class sql_get : public sql
    {
    public:
      virtual int opt_type() { return sql_opt::SQL_SELECT; }
      virtual int build_sql(char *bf, const int bf_len)
      { return this->data_.select_sql(bf, bf_len); }

      void read_sql_row(daily_clean_info *data, MYSQL_ROW &row)
      {
        data->char_id_              = ::atoi(row[daily_clean_info::IDX_CHAR_ID]);
        data->clean_time_           = ::atoi(row[daily_clean_info::IDX_CLEAN_TIME]);
        data->guild_free_jb_cnt_    = ::atoi(row[daily_clean_info::IDX_GUILD_FREE_JB_CNT]);
        data->guild_cost_jb_cnt_    = ::atoi(row[daily_clean_info::IDX_GUILD_COST_JB_CNT]);
        data->guild_promote_skill_cnt_ = ::atoi(row[daily_clean_info::IDX_GUILD_PROMOTE_SKILL_CNT]);
        data->ol_award_get_idx_     = ::atoi(row[daily_clean_info::IDX_OL_AWARD_GET_IDX]);
        data->login_award_if_get_   = ::atoi(row[daily_clean_info::IDX_LOGIN_AWARD_IF_GET]);
        data->daily_task_done_cnt_  = ::atoi(row[daily_clean_info::IDX_DAILY_TASK_DONE_CNT]);
        data->guild_task_done_cnt_  = ::atoi(row[daily_clean_info::IDX_GUILD_TASK_DONE_CNT]);
        data->ol_award_start_time_  = ::atoi(row[daily_clean_info::IDX_OL_AWARD_START_TIME]);
        data->con_login_days_       = ::atoi(row[daily_clean_info::IDX_CON_LOGIN_DAYS]);
        data->jing_ji_cnt_          = ::atoi(row[daily_clean_info::IDX_JING_JI_CNT]);
        data->free_refresh_jj_cnt_  = ::atoi(row[daily_clean_info::IDX_FREE_REFRESH_JJ_CNT]);
        data->if_mc_rebate_         = ::atoi(row[daily_clean_info::IDX_IF_MC_REBATE]);
        data->worship_cnt_          = ::atoi(row[daily_clean_info::IDX_WORSHIP_CNT]);
        data->goods_lucky_turn_cnt_ = ::atoi(row[daily_clean_info::IDX_GOODS_LUCKY_TURN_CNT]);
        data->daily_task_done_cnt_  = ::atoi(row[daily_clean_info::IDX_DAILY_TASK_DONE_CNT]);
        data->if_got_goods_lucky_draw_ = ::atoi(row[daily_clean_info::IDX_IF_GOT_GOODS_LUCKY_DRAW]);
        data->free_relive_cnt_      = ::atoi(row[daily_clean_info::IDX_FREE_RELIVE_CNT]);
        data->dxc_enter_cnt_        = ::atoi(row[daily_clean_info::IDX_DXC_ENTER_CNT]);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;
        mblock *mb = mblock_pool::instance()->alloc(sizeof(daily_clean_info));
        daily_clean_info *p = (daily_clean_info *)mb->data();
        p->reset();
        this->read_sql_row(p, row);
        s_cache.insert(p->char_id_, p);
        mb->wr_ptr(sizeof(daily_clean_info));
        ar->push_back(mb);
      }
    };
  public:
    tb_msg_get(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      int char_id = 0;
      in_stream is(msg, len);
      is >> db_sid >> char_id;

      // use cache
      daily_clean_info *info = s_cache.find(char_id);
      if (info != NULL)
      {
        // 如果重载了 operator << (out_stream &os, const T *) 要注意了
        return po->proc_result(db_sid,
                               0,
                               this->resp_id_,
                               DB_ASYNC_OK,
                               sizeof(daily_clean_info),
                               (char *)info);
      }

      sql_get *sql = new sql_get();
      sql->data_.char_id_ = char_id;
      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_get(REQ_GET_DAILY_CLEAN_INFO, new tb_msg_get(RES_GET_DAILY_CLEAN_INFO));

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
    tb_msg_insert(const int res) : resp_id_(res) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(daily_clean_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(daily_clean_info));

      sql_insert *sql = new sql_insert();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(daily_clean_info));

      // use cache
      s_cache.insert(sql->data_.char_id_, &(sql->data_));

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_insert(REQ_INSERT_DAILY_CLEAN_INFO, new tb_msg_insert(RES_INSERT_DAILY_CLEAN_INFO));

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
      char bf[sizeof(daily_clean_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(daily_clean_info));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(daily_clean_info));

      // use cache
      s_cache.insert(sql->data_.char_id_, &(sql->data_));

      db_async_store::instance()->do_sql(po->proxy_id(),
                                         db_sid,
                                         0,
                                         this->resp_id_,
                                         sql);
      return 0;
    }
    int resp_id_;
  };
  static tb_msg_maper tm_update(REQ_UPDATE_DAILY_CLEAN_INFO, new tb_msg_update(RES_UPDATE_DAILY_CLEAN_INFO));
}

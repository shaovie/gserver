#include "db_async_store.h"
#include "char_extra_info.h"
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
  static icache<int, char_extra_info, obj_pool_std_allocator<char_extra_info> > s_cache(CACHE_OBJ_SIZE);

  class sql : public sql_opt
  {
  public:
    virtual int id() { return this->data_.char_id_; }
    char_extra_info data_;
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

      void read_sql_row(char_extra_info *data, MYSQL_ROW &row)
      {
        data->char_id_             = ::atoi(row[char_extra_info::IDX_CHAR_ID]);
        data->yes_or_no_           = ::atoi(row[char_extra_info::IDX_YES_OR_NO]);
        data->zx_value_            = ::atoi(row[char_extra_info::IDX_ZX_VALUE]);
        data->huo_yue_score_       = ::atoi(row[char_extra_info::IDX_HUO_YUE_SCORE]);
        data->get_lvl_award_idx_   = ::atoi(row[char_extra_info::IDX_GET_LVL_AWARD_IDX]);
        data->jing_ji_max_rank_    = ::atoi(row[char_extra_info::IDX_JING_JI_MAX_RANK]);
        data->last_jing_ji_time_   = ::atoi(row[char_extra_info::IDX_LAST_JING_JI_TIME]);
        data->jing_ji_score_       = ::atoi(row[char_extra_info::IDX_JING_JI_SCORE]);
        data->got_ti_li_award_time_ = ::atoi(row[char_extra_info::IDX_GOT_TI_LI_AWARD_TIME]);
        data->ti_li_               = ::atoi(row[char_extra_info::IDX_TI_LI]);
        data->ti_li_time_          = ::atoi(row[char_extra_info::IDX_TI_LI_TIME]);
        data->check_sys_mail_time_ = ::atoi(row[char_extra_info::IDX_CHECK_SYS_MAIL_TIME]);
        data->water_tree_time_     = ::atoi(row[char_extra_info::IDX_WATER_TREE_TIME]);
        data->worship_time_        = ::atoi(row[char_extra_info::IDX_WORSHIP_TIME]);
        data->jing_li_             = ::atoi(row[char_extra_info::IDX_JING_LI]);
        data->jing_li_time_        = ::atoi(row[char_extra_info::IDX_JING_LI_TIME]);
        data->fa_bao_dj_           = ::atoi(row[char_extra_info::IDX_FA_BAO_DJ]);
        data->wt_goods_cnt_        = ::atoi(row[char_extra_info::IDX_WT_GOODS_CNT]);
        data->lucky_turn_goods_cnt_= ::atoi(row[char_extra_info::IDX_LUCKY_TURN_GOODS_CNT]);
        data->daily_goods_lucky_draw_cnt_= ::atoi(row[char_extra_info::IDX_DAILY_GOODS_LUCKY_DRAW_CNT]);
        data->xszc_honor_          = ::atoi(row[char_extra_info::IDX_XSZC_HONOR]);
        data->total_mstar_         = ::atoi(row[char_extra_info::IDX_TOTAL_MSTAR]);
        data->guild_scp_time_      = ::atoi(row[char_extra_info::IDX_GUILD_SCP_TIME]);
        ::strncpy(data->fa_bao_,
                  row[char_extra_info::IDX_FA_BAO], sizeof(data->fa_bao_) - 1);
        ::strncpy(data->seven_day_login_,
                  row[char_extra_info::IDX_SEVEN_DAY_LOGIN], sizeof(data->seven_day_login_) - 1);
      }
      virtual void read_sql_result(MYSQL_RES *res, async_result *ar)
      {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row == NULL) return ;
        mblock *mb = mblock_pool::instance()->alloc(sizeof(char_extra_info));
        char_extra_info *ci = (char_extra_info *)mb->data();
        ci->reset();
        this->read_sql_row(ci, row);
        s_cache.insert(ci->char_id_, ci);
        mb->wr_ptr(sizeof(char_extra_info));
        ar->push_back(mb);
      }
    };
  public:
    tb_msg_get(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      int char_id = 0;
      in_stream is(msg, len);
      is >> db_sid >> char_id;

      // use cache
      char_extra_info *info = s_cache.find(char_id);
      if (info != NULL)
      {
        // 如果重载了 operator << (out_stream &os, const T *) 要注意了
        return po->proc_result(db_sid,
                               0,
                               this->resp_id_,
                               DB_ASYNC_OK,
                               sizeof(char_extra_info),
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
  static tb_msg_maper tm_get(REQ_GET_CHAR_EXTRA_INFO, new tb_msg_get(RES_GET_CHAR_EXTRA_INFO));

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
      char bf[sizeof(char_extra_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(char_extra_info));

      sql_insert *sql = new sql_insert();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(char_extra_info));

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
  static tb_msg_maper tm_insert(REQ_INSERT_CHAR_EXTRA_INFO, new tb_msg_insert(RES_INSERT_CHAR_EXTRA_INFO));

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
    tb_msg_update(const int resp_id) : resp_id_(resp_id) { }
    virtual int handle_msg(proxy_obj *po, const char *msg, const int len)
    {
      int db_sid = -1;
      char bf[sizeof(char_extra_info) + 4];
      stream_istr si(bf, sizeof(bf));

      in_stream is(msg, len);
      is >> db_sid >> si;
      assert(si.str_len() == sizeof(char_extra_info));

      sql_update *sql = new sql_update();
      ::memcpy((char *)&sql->data_, si.str(), sizeof(char_extra_info));

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
  static tb_msg_maper tm_update(REQ_UPDATE_CHAR_EXTRA_INFO, new tb_msg_update(RES_UPDATE_CHAR_EXTRA_INFO));
}

#ifndef CLIENT_MGR_H_
#define CLIENT_MGR_H_

extern uint64_t g_total_conn_success_times;
extern uint64_t g_total_conn_failed_times;
extern uint64_t g_total_rcv_pkg_cnt;

class client_mgr
{
public:
  client_mgr() : cur_conn_cnt_(0) { }
  virtual ~client_mgr() { }

  void on_conn_failed() { g_total_conn_failed_times++; }
  void on_conn_success()
  {
    g_total_conn_success_times++;
    this->cur_conn_cnt_++;
  }
  void on_conn_close() { this->cur_conn_cnt_--; }
  void on_rcv_pkg()    { g_total_rcv_pkg_cnt++; }
protected:
  int cur_conn_cnt_;
};

#endif

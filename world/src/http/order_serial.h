// -*- C++ -*-

//========================================================================
/**
 * Author   : cliffordsun 
 * Date     : 2013-10-16 15:32
 */
//========================================================================

#ifndef ORDER_SERIAL_H_
#define ORDER_SERIAL_H_

// Lib header
#include <singleton.h>

#include "time_util.h"

#define ORDER_SERIAL_LEN 30

class order_serial : public singleton<order_serial>
{
  friend class singleton<order_serial>;
public:
  order_serial() : stream_pool_(0) { }

  char *alloc(const int char_id)
  {
    char *os_id = new char[ORDER_SERIAL_LEN + 1];
    if (this->stream_pool_ == 999999999) this->stream_pool_ = 0;
    int len = ::snprintf(os_id, ORDER_SERIAL_LEN + 1,
                         "%010d%010d%09d",
                         char_id, time_util::now, this->stream_pool_++);
    os_id[len] = '\0';
    this->order_serial_ids_.push_back(os_id);
    return os_id;
  }
  bool find(const char *os_id)
  { 
    ilist_node<char *> *iter = this->order_serial_ids_.head();
    for (; iter != NULL; iter = iter->next_)
    {
      if (::strcmp(iter->value_, os_id) == 0)
        return true;
    }
    return false;
  }
  void destroy(const char *os_id)
  {
    ilist_node<char *> *iter = this->order_serial_ids_.head();
    for (; iter != NULL; iter = iter->next_)
    {
      char *value = iter->value_;
      if (::strcmp(value, os_id) == 0)
      {
        this->order_serial_ids_.remove(iter->value_);
        delete value;
        break;
      }
    }
  }
private:
  int stream_pool_;
  ilist<char *> order_serial_ids_;
};
#endif //ORDER_SERIAL_H_

// -*- C++ -*-

//========================================================================
/**
 * Author   : jsonwu
 * Date     : 2013-12-30 14:40
 */
//========================================================================

#ifndef CHANNEL_H_
#define CHANNEL_H_

// Forward declarations
class out_stream;
class channel_impl;

/**
 * @class channel
 * 
 * @brief
 */
class channel
{
public:
  channel();
  virtual ~channel();

  bool empty();

  void join(const int);
  void quit(const int);

  void broadcast(const int msg_id, out_stream *body);
protected:
  channel_impl *impl_;
};
#endif // CHANNEL_H_

// -*- C++ -*-

//========================================================================
/**
 * Author   : jsonwu 
 * Date     : 2013-06-12 15:09
 */
//========================================================================

#ifndef CHANNEL_MGR_H_
#define CHANNEL_MGR_H_

#include "singleton.h"

// Forward declarations
class out_stream;
class channel_mgr_impl;

/**
 * @class channel_mgr
 * 
 * @brief
 */
class channel_mgr : public singleton<channel_mgr>
{
  friend class singleton<channel_mgr>;
public:
  /*just for guild, team, dynamic change*/
  void join(const int channel, const int idx, const int char_id);
  void quit(const int type, const int idx, const int char_id);

  void post(const int type,  
           const int idx,
           const int msg_id, 
           out_stream *body);
private:
  channel_mgr();
  channel_mgr_impl *impl_;
};
#endif // CHANNEL_MGR_H_


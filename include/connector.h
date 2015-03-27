// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 01:24
 */
//========================================================================

#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include "socket.h"
#include "inet_address.h"
#include "ev_handler.h"

/**
 * @class nonblocking_connect_handler
 *
 * @brief 
 */
template<typename svc_handler_t> class connector;

template<typename svc_handler_t>
class nonblocking_connect_handler : public ev_handler
{
  friend class connector<svc_handler_t>;
public:
  nonblocking_connect_handler(connector<svc_handler_t> &cn, svc_handler_t *h)
    : connector_(cn),
    svc_handler_(h)
  { }

  int close(svc_handler_t *&sh)
  {
    if (this->svc_handler_ == NULL) return -1;

    sh = this->svc_handler_;
    this->svc_handler_ = NULL;

    this->get_reactor()->cancel_timer(this->timer_id(), 1);
    this->timer_id(-1);

    this->get_reactor()->remove_handler(sh,
                                        ev_handler::all_events_mask
                                        | ev_handler::dont_call);
    return 0;
  }

  inline int get_handle(void) const
  { return this->svc_handler_->get_handle(); }
  inline void set_handle(const int handle)
  { this->svc_handler_->set_handle(handle); }

  // called by reactor when asynchronous connections fail. 
  virtual int handle_input(const int )
  {
    svc_handler_t *sh = NULL;
    this->close(sh);

    if (sh != NULL)
      sh->close(ev_handler::connect_mask);

    delete this; 
    return 0;
  }

  // called by reactor when asynchronous connections succeed. 
  virtual int handle_output(const int handle)
  {
    svc_handler_t *sh = NULL;
    this->close(sh);

    if (sh != NULL)
      this->connector_.init_svc_handler(sh, handle);
    delete this;
    return 0;
  }

  // called if a connection times out before completing.
  virtual int handle_timeout(const time_value &tv)
  {
    this->timer_id(-1);

    svc_handler_t *sh = NULL;
    this->close(sh);

    if (sh != NULL && sh->handle_timeout(tv) == -1)
      sh->handle_close(sh->get_handle(), ev_handler::timer_mask);

    delete this; 
    return 0;
  }
  virtual int handle_close(const int , reactor_mask )
  {
    svc_handler_t *sh = NULL;
    this->close(sh);

    if (sh != NULL)
      sh->close(ev_handler::connect_mask);

    delete this; 
    return 0;
  }
private:
  //
  connector<svc_handler_t> &connector_;
  //
  svc_handler_t *svc_handler_;
};
/**
 * @class connector
 *
 * @brief 
 */
template<typename svc_handler_t>
class connector : public ev_handler
{
  friend class nonblocking_connect_handler<svc_handler_t>;
public:
  connector() { }

  inline int open(reactor *r)
  {
    this->set_reactor(r);
    return 0;
  }

  int connect(svc_handler_t *sh, 
              const inet_address &remote_addr, 
              const time_value *timeout,
              const size_t rcvbuf_size = 0)
  {
    if (sh == NULL) return -1;

    sh->set_reactor(this->get_reactor());

    int result = this->connect_i(sh, remote_addr, rcvbuf_size);

    if (result == 0)
    {
      if (sh->open((void*)this) != 0)
      {
        sh->close(); 
        return -1;
      }
    }else if (errno == EINPROGRESS)
    {
      if (timeout && *timeout == time_value::zero)
      {
        sh->close(ev_handler::connect_mask);
        return -1;
      }
      result = this->nonblocking_connect(sh, timeout);
    }else
    {
      sh->close(ev_handler::connect_mask);
    }
    return result;
  }
protected:
  int connect_i(svc_handler_t *sh,
                const inet_address &remote_addr,
                const size_t recvbuf_size)
  {
    int handle = sh->get_handle();
    if (handle == -1)
    {
      handle = ::socket(AF_INET, SOCK_STREAM, 0);
      if (handle == -1) return -1;
    }

    // all of sockets create by framwork are nonblocked.
    socket::set_nonblock(handle);

    if (recvbuf_size != 0 
        && socket::set_rcvbuf(handle, recvbuf_size) == -1)
    {
      ::close(handle);
      return -1;
    }
    //
    sh->set_handle(handle);

    return ::connect(handle, 
                     reinterpret_cast<sockaddr *>(remote_addr.get_addr()),
                     remote_addr.get_addr_size());
  }
  int nonblocking_connect(svc_handler_t *sh, const time_value *timeout)
  {
    nonblocking_connect_handler<svc_handler_t> *nbch = 
      new nonblocking_connect_handler<svc_handler_t>(*this, sh);

    int mask = ev_handler::connect_mask;
    if (this->get_reactor()->register_handler(nbch, mask) == -1)
    {
      nbch->handle_close(-1, mask);
      return -1;
    }
    nbch->set_reactor(this->get_reactor());

    if (timeout != NULL)
    {
      int timer_id = this->get_reactor()->schedule_timer(nbch, *timeout);
      if (timer_id == -1)
      {
        this->get_reactor()->remove_handler(nbch, mask); // will call hbch->handle_close
        return -1;
      }
      nbch->timer_id(timer_id);
    }

    return 0;
  }
  void init_svc_handler(svc_handler_t *sh, const int handle)
  {
    sh->set_handle(handle);
    inet_address peer_addr;
    if (socket::get_remote_addr(handle, peer_addr) != -1)
    {
      sh->set_remote_addr(peer_addr);
      if (sh->open((void*)this) == 0)
        return ;
    }
    sh->close(ev_handler::connect_mask);
  }
};
#endif // CONNECTOR_H_


// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 01:24
 */
//========================================================================

#ifndef ACCEPTOR_H_
#define ACCEPTOR_H_

#include "socket.h"
#include "reactor.h"
#include "ev_handler.h"
#include "inet_address.h"

#include <errno.h>

/**
 * @class acceptor

 * @brief 
 */
template<class SVC_HANDLER>
class acceptor : public ev_handler
{
public:
  acceptor() : listen_fd_(-1) { }

  int open(const inet_address &local_addr,
           reactor *r,
           const size_t rcvbuf_size = 0,
           const int backlog = 256)
  {
    if (this->open_i(local_addr, rcvbuf_size, backlog) == -1) 
      return -1;

    int result = r->register_handler(this, ev_handler::accept_mask);
    if (result != -1)
      this->set_reactor(r);
    else
    {
      ::close(this->listen_fd_);
      this->listen_fd_ = -1;
    }
    return result;
  }

  virtual int get_handle(void) const
  { return this->listen_fd_; }

  virtual int close(void)
  { return this->handle_close(-1, ev_handler::all_events_mask); }
protected:
  int accept_i(inet_address *remote_addr)
  {
    int new_handle = -1;
    int len = remote_addr->get_addr_size();
    do
    {
      new_handle = ::accept(this->listen_fd_,
                            (sockaddr *)remote_addr->get_addr(), 
                            (socklen_t *)&len);
    } while (new_handle == -1 && errno == EINTR);
    return new_handle;
  }

  int open_i(const inet_address &local_addr,
             const size_t rcvbuf_size,
             const int backlog)
  {
    this->listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (this->listen_fd_ == -1)
      return -1;

    int error = 0;
    if (rcvbuf_size != 0)
      socket::set_rcvbuf(this->listen_fd_, rcvbuf_size);

    if (socket::reuseaddr(this->listen_fd_, 1) != 0)
      error = 1;

    sockaddr_in local_inet_address;
    std::memset(reinterpret_cast<void *>(&local_inet_address),
                0,
                sizeof(local_inet_address));
    local_inet_address = *reinterpret_cast<sockaddr_in *>(local_addr.get_addr());
    if (::bind(this->listen_fd_,
               reinterpret_cast<sockaddr *>(&local_inet_address),
               sizeof(local_inet_address)) == -1)
      error = 1;

    if (error != 0 || ::listen(this->listen_fd_, backlog) == -1)
    {
      ::close(this->listen_fd_);
      this->listen_fd_ = -1;
      return -1;
    }

    socket::set_nonblock(this->listen_fd_);
    return 0;
  }
  virtual int make_svc_handler(SVC_HANDLER *&sh)
  {
    if (sh == NULL)
      sh = new SVC_HANDLER();
    return 0;
  }
  virtual int handle_close(const int , reactor_mask )
  {
    if (this->get_reactor() != NULL)
    {
      this->get_reactor()->remove_handler(this,
                                          ev_handler::accept_mask
                                          | ev_handler::dont_call);
      this->set_reactor(NULL);
      // Shut down the listen socket to recycle the handles.
      ::close(this->listen_fd_);
      this->listen_fd_ = -1;
    }
    return 0;
  }
  virtual int handle_input(const int )
  {
    do
    {
      int new_handle = this->accept_i(&(this->shared_remote_addr_));
      if (new_handle == -1)
        break;
      //
      SVC_HANDLER *sh = NULL;
      if (this->make_svc_handler(sh) == -1)
        break;

      socket::set_nonblock(new_handle);
      sh->set_handle(new_handle);
      sh->set_remote_addr(this->shared_remote_addr_);

      // Activate the SVC_HANDLER
      sh->set_reactor(this->get_reactor());

      if (sh->open((void *)this) == -1)
        sh->close(); 
    } while (true);

    return 0;
  }
private:
  int listen_fd_;

  inet_address shared_remote_addr_;
};

#endif // ACCEPTOR_H_


// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-28 01:52
 */
//========================================================================

#ifndef I_SOCKET_H_
#define I_SOCKET_H_

#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>

#include "inet_address.h"

/**
 * @class socket
 * 
 * @brief 
 */
class socket
{
public:
  //= IO operation
  static int recv(const int handle, void *buff, const size_t len)
  {
    int ret = 0;
    do
    {
      ret = ::recv(handle, buff, len, 0);
    }while (ret == -1 && errno == EINTR);
    return ret;
  }
  static int send(const int handle, const void *buff, const size_t len)
  {
    int ret = 0;
    do
    {
      ret = ::send(handle, buff, len, 0);
    }while (ret == -1 && errno == EINTR);
    return ret;
  }
  static int close(const int handle)
  { return ::close(handle); }

  //= 
  static inline int get_local_addr(const int handle,
                                   inet_address &local_addr)
  {
    int l = local_addr.get_addr_size();
    struct sockaddr *addr = 
      reinterpret_cast<struct sockaddr*>(local_addr.get_addr());
    if (::getsockname(handle, addr, (socklen_t *)&l) == -1)
      return -1;
    return 0;
  }
  static inline int get_remote_addr(const int handle,
                                    inet_address &remote_addr)
  {
    int l = remote_addr.get_addr_size();
    struct sockaddr *addr = 
      reinterpret_cast<struct sockaddr*> (remote_addr.get_addr());
    if (::getpeername(handle, addr, (socklen_t *)&l) == -1)
      return -1;
    return 0;
  }
  static inline int reuseaddr(const int handle, const int val)
  {
    return ::setsockopt(handle,
                        SOL_SOCKET,
                        SO_REUSEADDR, 
                        (const void*)&val,
                        sizeof(val));
  }
  static inline int set_nonblock(const int handle)
  {
    int flag = ::fcntl(handle, F_GETFL, 0);
    if (flag == -1) return -1;
    if (flag & O_NONBLOCK) // already nonblock
      return 0;
    return ::fcntl(handle, F_SETFL, flag | O_NONBLOCK);
  }
  static inline int set_block(const int handle)
  {
    int flag = ::fcntl(handle, F_GETFL, 0);
    if (flag == -1) return -1;
    if (flag & O_NONBLOCK) // already nonblock
      return ::fcntl(handle, F_SETFL, flag & (~O_NONBLOCK));
    return 0;
  }
  static inline int set_rcvbuf(const int handle, const size_t size)
  {
    if (size == 0) return -1;
    return ::setsockopt(handle,
                        SOL_SOCKET, 
                        SO_RCVBUF, 
                        (const void*)&size, 
                        sizeof(size));
  }
  static inline int set_sndbuf(const int handle, const size_t size)
  {
    if (size == 0) return -1;
    return ::setsockopt(handle,
                        SOL_SOCKET,
                        SO_SNDBUF, 
                        (const void*)&size, 
                        sizeof(size));
  }
  static inline int set_nodelay(const int handle)
  {
    int flag = 1;
    return ::setsockopt(handle,
                        IPPROTO_TCP,
                        TCP_NODELAY, 
                        (void *)&flag, 
                        sizeof(flag));
  }
  static inline int getsock_error(const int handle, int &err)
  {
    socklen_t len = sizeof(int);
    return ::getsockopt(handle, SOL_SOCKET, SO_ERROR, &err, &len);
  }
};

#endif // I_SOCKET_H_


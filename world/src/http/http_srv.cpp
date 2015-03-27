#include "http_srv.h"
#include "sys_log.h"
#include "util.h"

// Lib header
#include <ctype.h>
#include <stdio.h>
#include <cstring>
#include "mblock.h"

static ilog_obj *e_log = err_log::instance()->get_ilog("base");
static ilog_obj *s_log = sys_log::instance()->get_ilog("base");

namespace help
{
  inline static int get_uri(const char *req, char *uri)
  {
    const char *uri_begin = req + ::strspn(req, " \t");
    size_t len = ::strcspn(uri_begin, " \t");
    if (len == 0) return -1;
    ::strncpy(uri, uri_begin, len);
    return 0;
  }
  inline static void get_interface(const char *uri, char *interface)
  {
    const char *interface_b = uri + 1;  // skip '/'

    size_t interface_len = ::strcspn(interface_b, "?");
    ::strncpy(interface, interface_b, interface_len);
  }
};
http_srv::http_srv(const char *client_type)
: client(2048, client_type)
{ }
http_srv::~http_srv()
{ }
int http_srv::dispatch_msg(const int /*id*/,
                           const int /*res*/,
                           const char * /*msg*/,
                           const int /*len*/)
{
  return -1;
}
int http_srv::handle_data()
{
  char *req = this->recv_buff_->rd_ptr();
  if (this->recv_buff_->space() < 1)
  {
    e_log->rinfo("http request is too large [%s]", req);
    return -1;
  }
  this->recv_buff_->set_eof();

  // handle roughly
  char *header_end = ::strstr(req, "\r\n\r\n");
  if (header_end == NULL) 
  {
    e_log->rinfo("uncomplete http header[%s]", req);
    return -1;
  }
  header_end += 4;
  int http_head_len = header_end - req;
  char *content_length_p = util::get_http_value(req, 
                                                "Content-Length",
                                                sizeof("Content-Length") - 1,
                                                NULL);
  int content_length = 0;
  if (content_length_p != NULL)
  {
    content_length = ::atoi(content_length_p);
    if (this->recv_buff_->length() - http_head_len < content_length)
      return 0;
  }

  if ((req[0] == 'G' || req[0] == 'g')
      && (req[1] == 'E' || req[1] == 'e')
      && (req[2] == 'T' || req[2] == 't')
     )
  {
    return this->handle_get_method();
  }else if ((req[0] == 'P' || req[0] == 'p')
            && (req[1] == 'O' || req[1] == 'o')
            && (req[2] == 'S' || req[2] == 's')
            && (req[3] == 'T' || req[3] == 't')
           )
  {
    if (content_length <= 0)
    {
      e_log->rinfo("post request [%s] content length is 0", req);
      return -1;
    }
    return this->handle_post_method();
  }
  e_log->rinfo("unsupportable![%s]", req);
  return -1;
}
int http_srv::handle_get_method()
{
  char uri[2048] = {0};
  char *req = this->recv_buff_->rd_ptr();
  if (this->recv_buff_->length() < 16) 
    return -1;

  if (help::get_uri(req + sizeof("get"), uri) == -1)
    return -1;
  s_log->rinfo("GET %s", uri);

  char *param_begin = ::strchr(uri, '?');
  if (param_begin != NULL)
    ++param_begin;

  char interface[128] = {0};
  help::get_interface(uri, interface);

  return this->dispatch_get_interface(interface, param_begin);
}
int http_srv::dispatch_get_interface(char * /*interface*/, 
                                     char * /*param_begin*/)
{ 
  return -1;
}
int http_srv::handle_post_method()
{
  char uri[2048] = {0};
  char *req = this->recv_buff_->rd_ptr();
  if (this->recv_buff_->length() < 16) 
    return -1;

  if (help::get_uri(req + sizeof("post"), uri) == -1)
    return -1;

  // handle roughly
  char *header_end = ::strstr(req, "\r\n\r\n");
  if (header_end == NULL || *(header_end + 4) == '\0')
  {
    e_log->rinfo("uncomplete http header[%s]", req);
    return -1;
  }
  s_log->rinfo("POST %s %s", uri, header_end + 4);

  char interface[128] = {0};
  help::get_interface(uri, interface);

  return this->dispatch_post_interface(interface, header_end + 4);
}
int http_srv::dispatch_post_interface(char * /*interface*/, 
                                      char * /*body*/)
{ 
  return -1;
}

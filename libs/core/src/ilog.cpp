#include "ilog.h"

// Lib header

#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#define MAX_LEN_OF_ONE_RECORD             2047
#define MAX_NAME_LEN                      255

static const char *ilog_type_list[] =
{
  "SHUTDOWN",     // LT_SHUTDOWN == 1L << index
  "TRACE",	      // LT_TRACE    == 1L << index
  "DEBUG",        // LT_DEBUG    == 1L << index
  "WNING",	      // LT_WNING    == 1L << index
  "ERROR",	      // LT_ERROR    == 1L << index
  "RINFO",	      // LT_RINFO    == 1L << index
  "FATAL",        // LT_FATAL    == 1L << index
  "ALLS"
};

#define LL_SHUT_DOWN_BIT   0
#define LL_TRACE_BIT       1
#define LL_DEBUG_BIT       2
#define LL_WNING_BIT       3
#define LL_ERROR_BIT       4
#define LL_RINFO_BIT       5
#define LL_FATAL_BIT       6
#define LL_ALLS_BITS       7

#define BASE_MODULE_NAME   "base"

enum 
{
  // = Note, this first argument *must* start at 1!

  // Shutdown the logger
  LL_SHUTDOWN = 1L << LL_SHUT_DOWN_BIT,
  LL_TRACE	  = 1L << LL_TRACE_BIT,	
  LL_DEBUG	  = 1L << LL_DEBUG_BIT,
  LL_WNING	  = 1L << LL_WNING_BIT,
  LL_ERROR    = 1L << LL_ERROR_BIT,
  LL_RINFO	  = 1L << LL_RINFO_BIT,
  LL_FATAL	  = 1L << LL_FATAL_BIT,
  LL_ALLS     = 1L << LL_ALLS_BITS,

  LL_ALL_TYPE = LL_TRACE \
                | LL_DEBUG 
                | LL_WNING 
                | LL_ERROR 
                | LL_RINFO 
                | LL_FATAL
};

namespace help
{
  char *strstrip_all(const char *in_str, char *out_str, int len)
  {
    if (in_str == NULL || out_str == NULL) 
      return NULL;

    char *s = out_str;
    for (int i = 0; *in_str && i < len; ++in_str)
    {
      if (isspace((int)*in_str) == 0)
        *s++ = *in_str;
    }
    return s;
  }
  int split_log_type(const char *value_p)
  {
    char *tok_p = NULL;
    char *token = NULL;
    char bf[512] = {0};
    ::strncpy(bf, value_p, sizeof(bf));
    int l_type = 0;
    for (token = ::strtok_r(bf, "|", &tok_p); 
         token != NULL;
         token = ::strtok_r(NULL, ",", &tok_p))
    {
      for (size_t j = 0;
           j < sizeof(ilog_type_list) / sizeof(ilog_type_list[0]);
           ++j)
      {
        if (::strcmp(token, ilog_type_list[j]) == 0)
          l_type |= 1L << j;
      }
    }
    if (l_type & LL_ALLS) l_type |= LL_ALL_TYPE;
    return l_type;
  }
}
/**
 * @class ilog_output
 * 
 * @brief 
 */
class ilog_output
{
public:
  ilog_output();

  int open(const char *dir,
           const char *fname,
           const int roller,
           const int max_fsize);

  inline void build_name(char *real_name);
  int create_file(void);
  void close(void);
  int log(const char *log_msg, const int len);
  int check_file_size(void);
  int rolloever_files();
private:
  int log_handle_;
  int curr_file_size_;
  int max_file_size_;
  int roller_;

  char file_name_[MAX_NAME_LEN + 1];
  char log_dir_[MAX_NAME_LEN + 1];
};
ilog_output::ilog_output() :
  log_handle_(-1),
  curr_file_size_(0),
  roller_(0)
{
  this->file_name_[MAX_NAME_LEN] = '\0';
  this->log_dir_[MAX_NAME_LEN]   = '\0';
}
int ilog_output::open(const char *dir,
                      const char *fname,
                      const int roller,
                      const int max_fsize)
{
  ::strncpy(this->file_name_, fname, MAX_NAME_LEN);
  ::strncpy(this->log_dir_, dir, MAX_NAME_LEN);
  this->roller_         = roller;
  this->max_file_size_  = max_fsize;

  return this->create_file();
}
inline void ilog_output::build_name(char *real_name)
{
  ::strncpy(real_name, this->log_dir_, MAX_NAME_LEN);
  ::strncat(real_name, "/", MAX_NAME_LEN - ::strlen(this->log_dir_));
  ::strncat(real_name, this->file_name_, MAX_NAME_LEN - ::strlen(this->log_dir_) - 1);
}
int ilog_output::create_file(void)
{
  // Check and create dir
  if (::mkdir(this->log_dir_, 0755) == -1)
  {
    if (errno != EEXIST) return -1;
    errno = 0;
  }
  // Get file size. If the file is exsit and its size is out of 
  // configure, then create a new one
  this->close();

  char real_name[MAX_NAME_LEN + 1] = {0};
  this->build_name(real_name);

  struct stat bf;
  ::memset(&bf, 0, sizeof(bf));
  if (::stat(real_name, &bf) == 0)
  {
    this->curr_file_size_ = bf.st_size;
    this->check_file_size();
  }
  // Open file
  this->log_handle_ = ::open(real_name, O_CREAT | O_RDWR | O_APPEND, 0644);
  if (this->log_handle_ == -1) // create file failed
    return -1;

  return 0;
}
void ilog_output::close()
{
  if (this->log_handle_ != -1)
    ::close(this->log_handle_);
  this->log_handle_ = -1;
}
int ilog_output::log(const char *log_msg, const int len)
{
  if (this->log_handle_ == -1)
    return -1;

  // 1. Check file size
  if (this->check_file_size())
  {
    this->close();
    char real_name[MAX_NAME_LEN + 1] = {0};
    this->build_name(real_name);
    this->log_handle_ = ::open(real_name, O_CREAT | O_RDWR | O_APPEND, 0644);
    if (this->log_handle_ == -1)
      return -1;
  }

  // 3. Record the log msg
  int result = 0;
  do
  {
    if ((result = ::write(this->log_handle_, log_msg, len)) <= 0)
    {
      if (errno == ENOSPC)
        return 0;
      else if (errno == EINTR)
        continue;
      this->close();
      return -1;
    }
  }while (0);
  this->curr_file_size_ += result;
  return result;
}
int ilog_output::check_file_size(void)
{
  if (this->max_file_size_ > 0 && this->curr_file_size_ >= this->max_file_size_)
  {
    // Backup file, switch file
    if (this->roller_ > 0)
      this->rolloever_files();
    else
    {
      char real_name[MAX_NAME_LEN + 1] = {0};
      this->build_name(real_name);
      ::unlink(real_name);
    }

    this->curr_file_size_ = 0;
    return 1;
  }
  return 0;
}
int ilog_output::rolloever_files()
{
  char real_name[MAX_NAME_LEN + 1] = {0};
  this->build_name(real_name);

  char src[MAX_NAME_LEN + 1] = {0};
  char tgt[MAX_NAME_LEN + 1] = {0};
  for (int i = this->roller_ - 1; 
       i >= 1; 
       --i)
  {
    ::snprintf(src, MAX_NAME_LEN, 
               "%s.%d",
               real_name,
               i);
    ::snprintf(tgt, MAX_NAME_LEN, 
               "%s.%d",
               real_name,
               i + 1);
    ::rename(src, tgt);
    ::memset(src, 0, sizeof(MAX_NAME_LEN));
    ::memset(tgt, 0, sizeof(MAX_NAME_LEN));
  }
  ::memset(tgt, 0, sizeof(MAX_NAME_LEN));
  ::snprintf(tgt, MAX_NAME_LEN, 
             "%s.%d",
             real_name,
             1);
  ::rename(real_name, tgt);
  return 0;
}
// ---------------------------------------------------------------------------
/**
 * @class ilog_obj_impl
 * 
 * @brief implement of ilog
 */
class ilog_obj_impl
{
  friend class ilog;
  friend class ilog_obj;
  friend class ilog_impl;
private:
  ilog_obj_impl(const char *m_name, int l_type, ilog_impl *impl);

  //
  int log_type_;

  ilog_impl *ilog_impl_;
  ilog_obj  *next_;

  char m_name_[MAX_NAME_LEN + 1];
};
ilog_obj_impl::ilog_obj_impl(const char *m_name, int l_type, ilog_impl *impl) :
  log_type_(l_type),
  ilog_impl_(impl),
  next_(NULL)
{
  ::strncpy(this->m_name_, m_name, MAX_NAME_LEN);
  this->m_name_[MAX_NAME_LEN] = '\0';
}
// -----------------------------------------------------------------------
/**
 * @class ilog_impl
 * 
 * @brief implement of ilog
 */
class ilog_impl
{
  friend class ilog;
  friend class ilog_obj;
  friend class ilog_obj_impl;
private:
  ilog_impl()
  {
    log_record_ = new char[MAX_LEN_OF_ONE_RECORD + 1];
    log_record_[MAX_LEN_OF_ONE_RECORD] = '\0';
    ilog_output_ = new ilog_output();
    ilog_obj_list_ = NULL;
    log_time_[sizeof(log_time_) - 1] = '\0';
  }
  //
  int init(const char *path);

  //
  int load_config(const char *);

  // 
  ilog_obj *get_ilog(const char *module_name);

  void update_all_ilog_obj();

  ilog_obj *find_ilog_obj(const char *module_name);
  void insert_ilog_obj(ilog_obj *pio);

  // log
  int log(const int ll, const char *m_name, const char *format, va_list &va_ptr);

  int output(const char *r, const int len);
private:
  // One log record buffer
  char *log_record_;

  ilog_output *ilog_output_;

  ilog_obj *ilog_obj_list_;

  char log_time_[24];
};
int ilog_impl::init(const char *path)
{
  if (this->load_config(path) != 0)
    return -1;
  return 0;
}
int ilog_impl::load_config(const char *path)
{
  FILE *f = ::fopen(path, "r");
  if (f == NULL) 
  {
    fprintf(stderr, "Error: ilog - open %s failed!\n", path);
    return -1;
  }

  char dir[MAX_NAME_LEN + 1] = {0};
  char fname[MAX_NAME_LEN + 1] = {0};

  int roller  = 5;
  int max_fsize  = 0;

  char line_bf[512] = {0};
  bool find_base = false;
  while (fgets(line_bf, sizeof(line_bf), f))
  {
    char line[512] = {0};
    ::memset(line, '\0', sizeof(line));
    help::strstrip_all(line_bf, line, sizeof(line));
    if (line[0] == '#') continue;

    if (::strstr(line, "dir="))
    {
      ::strncpy(dir, (line + sizeof("dir=") - 1), MAX_NAME_LEN);
    }else if (::strstr(line, "file_name="))
    {
      ::strncpy(fname, (line + sizeof("file_name=") - 1), MAX_NAME_LEN);
    }else if (::strstr(line, "roller="))
    {
      roller = ::atoi(line + sizeof("roller=") - 1);
    }else if (::strstr(line, "file_size="))
    {
      char *p = line + sizeof("file_size=") - 1;
      max_fsize = ::atoi(p);
      if (::strrchr(p, 'k') || ::strrchr(p, 'K'))
        max_fsize *= 1024;
      else if (::strrchr(p, 'm') || ::strrchr(p, 'M'))
        max_fsize *= 1024 * 1024;
      if (max_fsize <= 0)
        max_fsize = 10 * 1024 * 1024;
    }else // module
    {
      if (::strstr(line, BASE_MODULE_NAME"="))
        find_base = true;
      int l_type = 0;
      char *sp = strchr(line, '=');
      if (sp == NULL)
        continue;
      //
      l_type = help::split_log_type(sp + 1);

      // update or insert new one
      *sp = '\0';

      char *m_name = line;
      ilog_obj *io = this->find_ilog_obj(m_name);
      if (io != NULL)
        io->impl_->log_type_ = l_type;
      else
        this->insert_ilog_obj(new ilog_obj(new ilog_obj_impl(m_name, l_type, this)));
    }
  }
  fclose(f);

  if (!find_base)
  {
    fprintf(stderr, "Error: ilog - not found '%s' module!\n", BASE_MODULE_NAME);
    return -1;
  }
  this->update_all_ilog_obj();
  return this->ilog_output_->open(dir, fname, roller, max_fsize);
}
void ilog_impl::update_all_ilog_obj()
{
  ilog_obj *base = this->find_ilog_obj(BASE_MODULE_NAME);

  ilog_obj *itor = this->ilog_obj_list_;
  for (; itor != NULL; itor = itor->impl_->next_)
    itor->impl_->log_type_ |= base->impl_->log_type_;
}
ilog_obj* ilog_impl::get_ilog(const char *m_name)
{
  ilog_obj *base = this->find_ilog_obj(BASE_MODULE_NAME);
  // don't check base != NULL
  //
  if (base && ::strcmp(base->impl_->m_name_, m_name) == 0)
    return base;

  // find ok
  ilog_obj *p_obj = this->find_ilog_obj(m_name);
  if (base && p_obj)
  {
    p_obj->impl_->log_type_ |= base->impl_->log_type_;
    return p_obj;
  }

  // new one
  int log_type = 0;
  if (base) log_type |= base->impl_->log_type_;
  ilog_obj *pio = new ilog_obj(new ilog_obj_impl(m_name, log_type, this));
  this->insert_ilog_obj(pio);

  return pio;
}
ilog_obj *ilog_impl::find_ilog_obj(const char *m_name)
{
  ilog_obj *itor = this->ilog_obj_list_;
  for (; itor != NULL; itor = itor->impl_->next_)
  {
    if (::strcmp(itor->impl_->m_name_, m_name) == 0)
      return itor;
  }
  return NULL;
}
void ilog_impl::insert_ilog_obj(ilog_obj *pio)
{
  pio->impl_->next_ = this->ilog_obj_list_;
  this->ilog_obj_list_ = pio;
}
int ilog_impl::log(const int lt, const char *m_name, const char *format, va_list &va_ptr) 
{
  int len = 0;
  struct timeval now;
  gettimeofday(&now, NULL);

  struct tm tm_;
  ::localtime_r(&(now.tv_sec), &tm_);
  ::strftime(this->log_time_, sizeof(this->log_time_), "%Y-%m-%d %H:%M:%S", &tm_);

  int ret = ::snprintf(this->log_record_,
                       MAX_LEN_OF_ONE_RECORD,
                       "[%s.%03d]<%s><%s>: ",
                       this->log_time_,
                       (int)((now.tv_usec + 999) / 1000),
                       ilog_type_list[lt],
                       m_name);
  if (ret > MAX_LEN_OF_ONE_RECORD)
    ret = MAX_LEN_OF_ONE_RECORD - 1;
  len += ret;
  if (len < MAX_LEN_OF_ONE_RECORD)
  {
    ret = ::vsnprintf(this->log_record_ + len,
                      MAX_LEN_OF_ONE_RECORD - len,
                      format,
                      va_ptr);
    if (ret < 0) return -1;
    /**
     * check overflow or not
     * Note : snprintf and vnprintf return value is the number of characters
     *(not including the trailing ’\0’) which would have been  written  to 
     * the  final  string  if enough space had been available.
     */
    if (ret > (MAX_LEN_OF_ONE_RECORD - len))
      ret = MAX_LEN_OF_ONE_RECORD - len - 1;
    // vsnprintf return the length of <va_ptr> actual
    len += ret;
  }
  this->log_record_[len] = '\n';
  this->log_record_[len + 1] = '\0';
  //
  return this->output(this->log_record_, len + 1);
}
int ilog_impl::output(const char *r, const int len)
{ return this->ilog_output_->log(r, len); }
// ---------------------------------------------------------------------------
ilog::ilog() : impl_(new ilog_impl()) { }
int ilog::init(const char *path)
{ return this->impl_->init(path); }
ilog_obj* ilog::get_ilog(const char *m_name)
{ return this->impl_->get_ilog(m_name); }
// ---------------------------------------------------------------------------
ilog_obj::ilog_obj(ilog_obj_impl *i) : impl_(i) { }
int ilog_obj::log(const char *record, const int len)
{ return this->impl_->ilog_impl_->output(record, len); }
// == Special log method
# define SHORT_CODE(LB, LT)  if (!(this->impl_->log_type_ & LT) ||       \
                                 (this->impl_->log_type_ & LL_SHUTDOWN)) \
return 0;                                                         \
va_list va; va_start(va, format);                                 \
int ret = this->impl_->ilog_impl_->log(LB, this->impl_->m_name_, format, va);   \
va_end(va);                                                       \
return ret

int ilog_obj::trace(const char *format, ...)    { SHORT_CODE(LL_TRACE_BIT, LL_TRACE); }
int ilog_obj::debug(const char *format, ...)    { SHORT_CODE(LL_DEBUG_BIT, LL_DEBUG); }
int ilog_obj::wning(const char *format, ...)    { SHORT_CODE(LL_WNING_BIT, LL_WNING); }
int ilog_obj::error(const char *format, ...)    { SHORT_CODE(LL_ERROR_BIT, LL_ERROR); }
int ilog_obj::rinfo(const char *format, ...)    { SHORT_CODE(LL_RINFO_BIT, LL_RINFO); }
int ilog_obj::fatal(const char *format, ...)    { SHORT_CODE(LL_FATAL_BIT, LL_FATAL); }

#undef SHORT_CODE

#include "behavior_log.h"
#include "load_json.h"

// Lib header
#include "mblock.h"
#include "obj_pool.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#define ONCE_WRITE_DISK_BLOCK_SIZE          4096
#define MAX_NAME_LEN                        255

/**
 * @class log_record_mb_allocator
 * 
 * @brief 
 */
class log_record_mb_allocator
{
public:
  inline mblock *alloc() { return new mblock(ONCE_WRITE_DISK_BLOCK_SIZE); }
  inline void release(mblock *p) { delete p; }
};
/**
 * @class log_store
 * 
 * @brief store player behavior log
 */
class log_store : public load_json
{
public:
  log_store(const char *path) :
    last_flush_log_time_(0),
    mblock_(NULL),
    w_mblock_head_(NULL),
    w_mblock_tail_(NULL),
    bits_map_(NULL),
    mblock_pool_()
  { 
    ::strncpy(this->f_name_, path, sizeof(this->f_name_)-1);
    this->mblock_ = this->mblock_pool_.alloc();
    this->bits_map_ = new char[MAX_BEHAVIOR_LOG_ID + 1];
    for (int i = 0; i <= MAX_BEHAVIOR_LOG_ID; ++i)
      this->bits_map_[i] = 1;
  }
  int load_config(const char *cfg_root)
  {
    Json::Value root;
    if (this->load_json_cfg(cfg_root, BEHAVIOR_LOG_CFG_PATH, root) != 0)
      return -1;

    char *bits = new char[MAX_BEHAVIOR_LOG_ID + 1];
    for (int i = 0; i <= MAX_BEHAVIOR_LOG_ID; ++i)
      bits[i] = 1;
    for (Json::Value::iterator iter = root.begin();
         iter != root.end();
         ++iter)
    {
      int id = ::atoi(iter.key().asCString());
      if (id > 0 && id <= MAX_BEHAVIOR_LOG_ID)
        bits[id] = (*iter).asInt();
    }
    if (this->bits_map_ != NULL)
      delete [] this->bits_map_;
    this->bits_map_ = bits;

    return 0;
  }
  bool bit_off(const int id)
  {
    if (id > 0 && id <= MAX_BEHAVIOR_LOG_ID)
      return this->bits_map_[id] != 1;
    return true;
  }
  void put(const int id, const int now, const char *format, va_list &va_ptr)
  {
    int ret = ::snprintf(this->log_record_,
                         MAX_LEN_OF_ONE_RECORD, 
                         "%d|%d|",
                         id, now);
    ret += ::vsnprintf(this->log_record_ + ret,
                       MAX_LEN_OF_ONE_RECORD - ret,
                       format,
                       va_ptr);
    if (ret == -1) return ;
    this->log_record_[ret++] = '\n';

    if (ret > this->mblock_->space())
    {
      if (this->w_mblock_head_ == NULL)
        this->w_mblock_head_ = this->w_mblock_tail_ = this->mblock_;
      else
      {
        this->w_mblock_tail_->next(this->mblock_);
        this->w_mblock_tail_ = this->mblock_;
      }
      this->mblock_ = this->mblock_pool_.alloc();
    }
    this->mblock_->copy(this->log_record_, ret);
  }
  void flush(const int now)
  {
    if (now - this->last_flush_log_time_ <= 3)
      return ;
    this->last_flush_log_time_ = now;

    if (this->w_mblock_head_ == NULL 
        && this->mblock_->length() == 0) 
      return ;

    int fd = ::open(this->f_name_, O_CREAT | O_RDWR | O_APPEND, 0644); 
    for (mblock *itor = this->w_mblock_head_; itor != NULL; )
    {
      mblock *mb = itor;
      itor = itor->next();
      ::write(fd, mb->rd_ptr(), mb->length());

      mb->reset();
      mb->next(NULL);
      this->mblock_pool_.release(mb);
    }
    ::write(fd, this->mblock_->rd_ptr(), this->mblock_->length());
    ::close(fd);

    this->mblock_->reset();
    this->w_mblock_head_ = this->w_mblock_tail_ = NULL;
  }
private:
  int last_flush_log_time_;
  mblock *mblock_;
  mblock *w_mblock_head_;
  mblock *w_mblock_tail_;

  char *bits_map_;
  obj_pool<mblock, log_record_mb_allocator> mblock_pool_;

  char f_name_[MAX_NAME_LEN + 1];
  char log_record_[MAX_LEN_OF_ONE_RECORD + 1];
};
// ------------------------------------------------------------------------------
behavior_log::behavior_log() : log_store_(NULL) { }
int behavior_log::open(const char *cfg_root)
{
  if (::mkdir(BEHAVIOR_LOG_DIR, 0755) == -1 && errno != EEXIST)
    return -1;
  char full_name[MAX_NAME_LEN + 1] = {0};
  ::snprintf(full_name, MAX_NAME_LEN, "%s/%s", BEHAVIOR_LOG_DIR, "behavior.log");
  this->log_store_ = new log_store(full_name);
  return this->log_store_->load_config(cfg_root);
}
int behavior_log::reload_config(const char *cfg_root)
{ return this->log_store_->load_config(cfg_root); }
void behavior_log::store(const int id, 
                         const int now,
                         const char *format, ...)
{ 
  if (this->log_store_->bit_off(id))
    return ;
  va_list va; 
  va_start(va, format);
  this->log_store_->put(id, now, format, va); 
  va_end(va); 
}
void behavior_log::flush(const int now)
{ return this->log_store_->flush(now); }


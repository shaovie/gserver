// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-09-19 10:47
 */
//========================================================================

#ifndef MBLOCK_POOL_H_
#define MBLOCK_POOL_H_

#include "guard.h"
#include "mblock.h"
#include "obj_pool.h"
#include "singleton.h"
#include "thread_mutex.h"

// Forward declarations

/**
 * @class mblock_pool
 * 
 * @brief
 */
class mblock_pool : public singleton<mblock_pool>
{
  friend class singleton<mblock_pool>;
public:
  mblock *alloc(const int size)
  {
#ifdef DB_PROXY
    guard<thread_mutex> g(this->mb_pool_mtx_);
#endif

#define ALLOC_S(S) do {\
  if (this->mb_##S##_free_list_ == NULL)  \
    this->mb_##S##_free_list_ = new mblock(S);  \
  mblock *p = this->mb_##S##_free_list_;  \
  this->mb_##S##_free_list_ = this->mb_##S##_free_list_->next();  \
  p->next(NULL); \
  return p;} while (0)

    if (size <= 4)           ALLOC_S(4);       //      1 ~ 4
    else if (size <= 8)      ALLOC_S(8);       //      5 ~ 8
    else if (size <= 16)     ALLOC_S(16);      //      9 ~ 16
    else if (size <= 32)     ALLOC_S(32);      //     17 ~ 32                
    else if (size <= 64)     ALLOC_S(64);      //     33 ~ 64
    else if (size <= 128)    ALLOC_S(128);     //     65 ~ 128                 
    else if (size <= 256)    ALLOC_S(256);     //    129 ~ 256
    else if (size <= 512)    ALLOC_S(512);     //    257 ~ 512
    else if (size <= 1024)   ALLOC_S(1024);    //    513 ~ 1024
    else if (size <= 2048)   ALLOC_S(2048);    //   1025 ~ 2048
    else if (size <= 4096)   ALLOC_S(4096);    //   2049 ~ 4096
    else if (size <= 8192)   ALLOC_S(8192);    //   4097 ~ 8192
    else if (size <= 16384)  ALLOC_S(16384);   //   8193 ~ 16384
    else if (size <= 32768)  ALLOC_S(32768);   //  16385 ~ 32768
    return new mblock(size);                   //  more ...
#undef ALLOC_S
  }                                                                   
  void release(mblock *mb)
  {
#ifdef DB_PROXY
    guard<thread_mutex> g(this->mb_pool_mtx_);
#endif

#define RELEASE_S(S) {\
  mb->next(this->mb_##S##_free_list_);  \
  this->mb_##S##_free_list_ = mb; \
  return ;}

    mb->reset();
    mb->prev(NULL);
    mb->next(NULL);
    switch(mb->size())
    {
    case 4:       RELEASE_S(4);
    case 8:       RELEASE_S(8);
    case 16:      RELEASE_S(16);
    case 32:      RELEASE_S(32);
    case 64:      RELEASE_S(64);
    case 128:     RELEASE_S(128);
    case 512:     RELEASE_S(512);
    case 1024:    RELEASE_S(1024);
    case 2048:    RELEASE_S(2048);
    case 4096:    RELEASE_S(4096);
    case 8192:    RELEASE_S(8192);
    case 16384:   RELEASE_S(16384);
    case 32768:   RELEASE_S(32768);
    default: mb->release();
    }
#undef RELEASE_S
  }
private:
  mblock_pool() :
    mb_4_free_list_(NULL),
    mb_8_free_list_(NULL),
    mb_16_free_list_(NULL),
    mb_32_free_list_(NULL),
    mb_64_free_list_(NULL),
    mb_128_free_list_(NULL),
    mb_256_free_list_(NULL),
    mb_512_free_list_(NULL),
    mb_1024_free_list_(NULL),
    mb_2048_free_list_(NULL),
    mb_4096_free_list_(NULL),
    mb_8192_free_list_(NULL),
    mb_16384_free_list_(NULL),
    mb_32768_free_list_(NULL)
  { }

  mblock *mb_4_free_list_;
  mblock *mb_8_free_list_;
  mblock *mb_16_free_list_;
  mblock *mb_32_free_list_;
  mblock *mb_64_free_list_;
  mblock *mb_128_free_list_;
  mblock *mb_256_free_list_;
  mblock *mb_512_free_list_;
  mblock *mb_1024_free_list_;
  mblock *mb_2048_free_list_;
  mblock *mb_4096_free_list_;
  mblock *mb_8192_free_list_;
  mblock *mb_16384_free_list_;
  mblock *mb_32768_free_list_;
#ifdef DB_PROXY
  thread_mutex mb_pool_mtx_;  // !!
#endif
};

#endif // MBLOCK_POOL_H_


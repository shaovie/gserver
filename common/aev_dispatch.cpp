#include "aev_dispatch.h"
#include "obj_pool.h"
#include "mblock.h"
#include "singleton.h"
#include "mblock_pool.h"

// -------------------------------------------------------------------------------
aev_dispatch::aev_dispatch()
{ }
aev_dispatch::~aev_dispatch()
{
  while (!this->aev_list_.empty())
  {
    aev_node en = this->aev_list_.pop_front();
    if (en.mb_ != NULL)
      mblock_pool::instance()->release(en.mb_);
  }
}
void aev_dispatch::post_aev(const int aev_id, mblock *mb)
{
  aev_node en(aev_id, mb);
  this->aev_list_.push_back(en);
}
void aev_dispatch::handle_aev_queue()
{
  if (this->aev_list_.empty()) return ;

  while (!this->aev_list_.empty())
  {
    aev_node en = this->aev_list_.pop_front();

    this->dispatch_aev(en.id_, en.mb_);
    if (en.mb_ != NULL)
      mblock_pool::instance()->release(en.mb_);
  }
}


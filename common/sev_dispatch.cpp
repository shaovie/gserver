#include "sev_dispatch.h"
#include "sev.h"

// Lib header

sev_notify::sev_notify() :
  min_ev_id_(MAX_SEV_ID)
{
  sev_dispatch_map_ = new ilist<sev_dispatch *>[MAX_SEV_ID]();
}
void sev_notify::notify(const int ev_id, mblock *mb)
{
  if (ev_id < 0 || ev_id >= MAX_SEV_ID)
    return ;

  if (this->sev_dispatch_map_[ev_id].empty())
    return ;

  ilist_node<sev_dispatch *> *itor = this->sev_dispatch_map_[ev_id].head();
  while (itor != NULL) // 此种迭代可以避免节点在迭代中自己清除自己
  {
    ilist_node<sev_dispatch *> *next = itor->next_;
    itor->value_->dispatch_sev(ev_id, mb);
    itor = next;
  }
}
void sev_notify::attach(const int ev_id, sev_dispatch *sd)
{
  if (ev_id < 0 || ev_id >= MAX_SEV_ID)
    return ;
  if (ev_id < this->min_ev_id_)
    this->min_ev_id_ = ev_id;
  this->sev_dispatch_map_[ev_id].push_back(sd);
}
void sev_notify::deattach(const int ev_id, sev_dispatch *sd)
{
  if (ev_id < 0 || ev_id >= MAX_SEV_ID)
    return ;
  this->sev_dispatch_map_[ev_id].remove(sd);
}
void sev_notify::deattach(sev_dispatch *sd)
{
  for (int i = this->min_ev_id_; i < MAX_SEV_ID; ++i)
    this->sev_dispatch_map_[i].remove(sd);
}

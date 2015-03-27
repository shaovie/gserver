#include "cache_module.h"
#include "player_obj.h"
#include "package_module.h"
#include "sys_log.h"
#include "icache.h"

// Lib header

static ilog_obj *s_log = sys_log::instance()->get_ilog("cache");
static ilog_obj *e_log = err_log::instance()->get_ilog("cache");

class equip_list
{
public:
  equip_list()
  { this->elist_ = new ilist<item_obj *>; }
  equip_list(ilist<item_obj *> *elist) :
    elist_(elist)
  { }
  equip_list(equip_list &obj)
  { *this = obj; }
  ~equip_list()
  {
    if (this->elist_ == NULL) return;
    while (!this->elist_->empty())
      package_module::release_item(this->elist_->pop_front());
    delete this->elist_;
  }
  const equip_list &operator = (const equip_list &obj)
  {
    if (this != &obj)
    {
      if (this->elist_ == NULL)
        this->elist_ = new ilist<item_obj*>;

      ilist_node<item_obj*> *myobj = this->elist_->head();
      ilist_node<item_obj*> *opobj = (obj.elist_ == NULL ? NULL : obj.elist_->head());

      for (;
           myobj != NULL && opobj != NULL;
           myobj = myobj->next_, opobj = opobj->next_)
        *myobj->value_ = *opobj->value_;

      while (myobj != NULL)
      {
        ilist_node<item_obj*> *next = myobj->next_;
        this->elist_->remove(myobj->value_);
        package_module::release_item(myobj->value_);
        myobj = next;
      }

      for (; opobj != NULL; opobj = opobj->next_)
      {
        item_obj *equip = package_module::alloc_new_item();
        *equip = *opobj->value_;
        this->elist_->push_back(equip);
      }
    }
    return *this;
  }

  ilist<item_obj*> *elist_;
};
static icache<int, equip_list, obj_pool_std_allocator<equip_list> > s_player_equip_cache(5000);

void cache_module::equip_cache_add_player(const int char_id,
                                          ilist<item_obj*> *elist)
{
  equip_list list(elist);
  s_player_equip_cache.insert(char_id, &list);
}
void cache_module::equip_cache_update_player(player_obj *player)
{
  equip_list *obj = s_player_equip_cache.find(player->id());
  if (obj != NULL)
  {
    equip_list list;
    package_module::find_all_item_in_pkg(player, PKG_EQUIP, *list.elist_);

    *obj = list;
    // 从player身上取出的item_obj 不能擅自release
    list.elist_->clear();
  }
}
ilist<item_obj*> *cache_module::get_player_equip(const int char_id)
{
  equip_list *obj = s_player_equip_cache.find(char_id);
  if (obj == NULL) return NULL;
  return obj->elist_;
}
void cache_module::on_char_logout(player_obj *player)
{
  cache_module::equip_cache_update_player(player);
}

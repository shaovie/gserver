#include "fighter_mgr.h"

// Lib header
#include <tr1/unordered_map>

class fighter_mgr_impl
{
public:
  typedef std::tr1::unordered_map<int, char_obj *> char_obj_map_t;
  typedef std::tr1::unordered_map<int, char_obj *>::iterator char_obj_map_itor;

  fighter_mgr_impl()
    : char_obj_map_(4096)
  { }
  void insert(const int char_id, char_obj *obj)
  { this->char_obj_map_.insert(std::make_pair(char_id, obj)); }
  char_obj *find(const int char_id)
  {
    char_obj_map_itor itor = this->char_obj_map_.find(char_id);
    return itor == this->char_obj_map_.end() ? NULL : itor->second;
  }
  void remove(const int char_id)
  { this->char_obj_map_.erase(char_id); }
private:
  char_obj_map_t char_obj_map_;
};
fighter_mgr::fighter_mgr()
: impl_(new fighter_mgr_impl())
{ }
void fighter_mgr::insert(const int char_id, char_obj *obj)
{ this->impl_->insert(char_id, obj); }
char_obj *fighter_mgr::find(const int char_id)
{ return this->impl_->find(char_id); }
void fighter_mgr::remove(const int char_id)
{ this->impl_->remove(char_id); }

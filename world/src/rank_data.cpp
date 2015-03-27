#include "rank_data.h"
#include "all_char_info.h"
#include "guild_module.h"
#include "istream.h"

void rank_data::build_info(out_stream &os)
{
  os << this->char_id_;
  char_brief_info *cbi =
    all_char_info::instance()->get_char_brief_info(this->char_id_);
  if (cbi == NULL)
    os << char(0) << stream_ostr("null", 4) << char(CAREER_LI_LIANG) << stream_ostr("", 0);
  else
  {
    os << cbi->vip_ << stream_ostr(cbi->name_, ::strlen(cbi->name_)) << cbi->career_;
    int guild_id = guild_module::get_guild_id(this->char_id_);
    char *guild_name = guild_module::get_guild_name(guild_id);
    if (guild_name == NULL)
      os << stream_ostr("", 0);
    else
      os << stream_ostr(guild_name, ::strlen(guild_name));
  }
  os << this->value_;
}
void mstar_rank_data::build_info(out_stream &os)
{
  os << this->char_id_;
  char_brief_info *cbi =
    all_char_info::instance()->get_char_brief_info(this->char_id_);
  if (cbi == NULL)
    os << char(0) << stream_ostr("null", 4) << char(CAREER_LI_LIANG) << int(0) << stream_ostr("", 0);
  else
  {
    os << cbi->vip_ << stream_ostr(cbi->name_, ::strlen(cbi->name_)) << cbi->career_ << cbi->zhan_li_;
    int guild_id = guild_module::get_guild_id(this->char_id_);
    char *guild_name = guild_module::get_guild_name(guild_id);
    if (guild_name == NULL)
      os << stream_ostr("", 0);
    else
      os << stream_ostr(guild_name, ::strlen(guild_name));
  }
  os << this->value_;
}

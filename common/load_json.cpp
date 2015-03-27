#include "load_json.h"
#include "sys_log.h"

// Lib header
#include <fstream>

static ilog_obj *e_log = err_log::instance()->get_ilog("config");

int load_json::load_json_cfg(const char *cfg_root,
                             const char *file,
                             Json::Value &root)
{
  std::string path = std::string(cfg_root) + "/" + file;
  std::ifstream fin(path.c_str());
  if (!fin)
  {
    e_log->error("open [%s] failed!", path.c_str());
    return -1;
  }

  Json::Reader j_reader;
  if (!j_reader.parse(fin, root, false))
  {
    e_log->error("parse [%s] failed![%s]",
                 path.c_str(),
                 j_reader.getFormatedErrorMessages().c_str());
    return -1;
  }
  fin.close();

  return 0;
}

// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-07-09 16:19
 */
//========================================================================

#ifndef LOAD_JSON_H_
#define LOAD_JSON_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <json/json.h>

// Forward declarations

/**
 * @class load_json
 * 
 * @brief
 */
class load_json
{
public:
  load_json() {}

  int load_json_cfg(const char *cfg_root,
                    const char *file,
                    Json::Value &root);
};

#endif // LOAD_JSON_H_


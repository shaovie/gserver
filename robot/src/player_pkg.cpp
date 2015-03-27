/*
** Lua binding: player
** Generated automatically by tolua++-1.0.92 on Mon Dec 30 16:21:41 2013.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_player_open (lua_State* tolua_S);

#include "snap_obj.h"
#include "player.h"
#include "time_util.h"

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_snap_obj_mgr (lua_State* tolua_S)
{
 snap_obj_mgr* self = (snap_obj_mgr*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_player (lua_State* tolua_S)
{
 player* self = (player*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_snap_obj (lua_State* tolua_S)
{
 snap_obj* self = (snap_obj*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"player");
 tolua_usertype(tolua_S,"int64_t");
 tolua_usertype(tolua_S,"snap_obj_mgr");
 tolua_usertype(tolua_S,"out_stream");
 tolua_usertype(tolua_S,"client");
 tolua_usertype(tolua_S,"snap_obj");
 tolua_usertype(tolua_S,"time_util");
}

/* get function: now of class  time_util */
#ifndef TOLUA_DISABLE_tolua_get_time_util_now
static int tolua_get_time_util_now(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)time_util::now);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: now of class  time_util */
#ifndef TOLUA_DISABLE_tolua_set_time_util_now
static int tolua_set_time_util_now(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  time_util::now = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: send_request of class  client */
#ifndef TOLUA_DISABLE_tolua_player_client_send_request00
static int tolua_player_client_send_request00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"client",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,3,"out_stream",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  client* self = (client*)  tolua_tousertype(tolua_S,1,0);
  const int msg_id = ((const int)  tolua_tonumber(tolua_S,2,0));
  out_stream* os = ((out_stream*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'send_request'", NULL);
#endif
  {
   int tolua_ret = (int)  self->send_request(msg_id,os);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_request'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: send_respond of class  client */
#ifndef TOLUA_DISABLE_tolua_player_client_send_respond00
static int tolua_player_client_send_respond00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"client",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,4,"out_stream",1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  client* self = (client*)  tolua_tousertype(tolua_S,1,0);
  const int msg_id = ((const int)  tolua_tonumber(tolua_S,2,0));
  const int res = ((const int)  tolua_tonumber(tolua_S,3,0));
  out_stream* os = ((out_stream*)  tolua_tousertype(tolua_S,4,NULL));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'send_respond'", NULL);
#endif
  {
   int tolua_ret = (int)  self->send_respond(msg_id,res,os);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_respond'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: send_respond_ok of class  client */
#ifndef TOLUA_DISABLE_tolua_player_client_send_respond_ok00
static int tolua_player_client_send_respond_ok00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"client",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,3,"out_stream",1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  client* self = (client*)  tolua_tousertype(tolua_S,1,0);
  const int msg_id = ((const int)  tolua_tonumber(tolua_S,2,0));
  out_stream* os = ((out_stream*)  tolua_tousertype(tolua_S,3,NULL));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'send_respond_ok'", NULL);
#endif
  {
   int tolua_ret = (int)  self->send_respond_ok(msg_id,os);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_respond_ok'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: send_respond_err of class  client */
#ifndef TOLUA_DISABLE_tolua_player_client_send_respond_err00
static int tolua_player_client_send_respond_err00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"client",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  client* self = (client*)  tolua_tousertype(tolua_S,1,0);
  const int msg_id = ((const int)  tolua_tonumber(tolua_S,2,0));
  const int res = ((const int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'send_respond_err'", NULL);
#endif
  {
   int tolua_ret = (int)  self->send_respond_err(msg_id,res);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_respond_err'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: send_msg of class  client */
#ifndef TOLUA_DISABLE_tolua_player_client_send_msg00
static int tolua_player_client_send_msg00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"client",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,4,"out_stream",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  client* self = (client*)  tolua_tousertype(tolua_S,1,0);
  const int msg_id = ((const int)  tolua_tonumber(tolua_S,2,0));
  const int res = ((const int)  tolua_tonumber(tolua_S,3,0));
  out_stream* tolua_var_1 = ((out_stream*)  tolua_tousertype(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'send_msg'", NULL);
#endif
  {
   int tolua_ret = (int)  self->send_msg(msg_id,res,tolua_var_1);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'send_msg'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_player_snap_obj_new00
static int tolua_player_snap_obj_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"snap_obj",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   snap_obj* tolua_ret = (snap_obj*)  Mtolua_new((snap_obj)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"snap_obj");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_player_snap_obj_new00_local
static int tolua_player_snap_obj_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"snap_obj",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   snap_obj* tolua_ret = (snap_obj*)  Mtolua_new((snap_obj)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"snap_obj");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: dir_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_get_snap_obj_dir_
static int tolua_get_snap_obj_dir_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dir_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->dir_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: dir_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_set_snap_obj_dir_
static int tolua_set_snap_obj_dir_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dir_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->dir_ = ((char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: coord_x_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_get_snap_obj_coord_x_
static int tolua_get_snap_obj_coord_x_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'coord_x_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->coord_x_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: coord_x_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_set_snap_obj_coord_x_
static int tolua_set_snap_obj_coord_x_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'coord_x_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->coord_x_ = ((short)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: coord_y_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_get_snap_obj_coord_y_
static int tolua_get_snap_obj_coord_y_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'coord_y_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->coord_y_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: coord_y_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_set_snap_obj_coord_y_
static int tolua_set_snap_obj_coord_y_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'coord_y_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->coord_y_ = ((short)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: obj_id_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_get_snap_obj_obj_id_
static int tolua_get_snap_obj_obj_id_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'obj_id_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->obj_id_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: obj_id_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_set_snap_obj_obj_id_
static int tolua_set_snap_obj_obj_id_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'obj_id_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->obj_id_ = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: obj_cid_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_get_snap_obj_obj_cid_
static int tolua_get_snap_obj_obj_cid_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'obj_cid_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->obj_cid_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: obj_cid_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_set_snap_obj_obj_cid_
static int tolua_set_snap_obj_obj_cid_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'obj_cid_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->obj_cid_ = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: scene_id_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_get_snap_obj_scene_id_
static int tolua_get_snap_obj_scene_id_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'scene_id_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->scene_id_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: scene_id_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_set_snap_obj_scene_id_
static int tolua_set_snap_obj_scene_id_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'scene_id_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->scene_id_ = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: scene_cid_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_get_snap_obj_scene_cid_
static int tolua_get_snap_obj_scene_cid_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'scene_cid_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->scene_cid_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: scene_cid_ of class  snap_obj */
#ifndef TOLUA_DISABLE_tolua_set_snap_obj_scene_cid_
static int tolua_set_snap_obj_scene_cid_(lua_State* tolua_S)
{
  snap_obj* self = (snap_obj*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'scene_cid_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->scene_cid_ = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  snap_obj_mgr */
#ifndef TOLUA_DISABLE_tolua_player_snap_obj_mgr_new00
static int tolua_player_snap_obj_mgr_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"snap_obj_mgr",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   snap_obj_mgr* tolua_ret = (snap_obj_mgr*)  Mtolua_new((snap_obj_mgr)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"snap_obj_mgr");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  snap_obj_mgr */
#ifndef TOLUA_DISABLE_tolua_player_snap_obj_mgr_new00_local
static int tolua_player_snap_obj_mgr_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"snap_obj_mgr",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   snap_obj_mgr* tolua_ret = (snap_obj_mgr*)  Mtolua_new((snap_obj_mgr)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"snap_obj_mgr");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: insert of class  snap_obj_mgr */
#ifndef TOLUA_DISABLE_tolua_player_snap_obj_mgr_insert00
static int tolua_player_snap_obj_mgr_insert00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"snap_obj_mgr",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,3,"snap_obj",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  snap_obj_mgr* self = (snap_obj_mgr*)  tolua_tousertype(tolua_S,1,0);
  const int obj_id = ((const int)  tolua_tonumber(tolua_S,2,0));
  snap_obj* so = ((snap_obj*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'insert'", NULL);
#endif
  {
   self->insert(obj_id,so);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'insert'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: find of class  snap_obj_mgr */
#ifndef TOLUA_DISABLE_tolua_player_snap_obj_mgr_find00
static int tolua_player_snap_obj_mgr_find00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"snap_obj_mgr",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  snap_obj_mgr* self = (snap_obj_mgr*)  tolua_tousertype(tolua_S,1,0);
  const int obj_id = ((const int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'find'", NULL);
#endif
  {
   snap_obj* tolua_ret = (snap_obj*)  self->find(obj_id);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"snap_obj");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'find'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  player */
#ifndef TOLUA_DISABLE_tolua_player_player_new00
static int tolua_player_player_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"player",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* ac = ((const char*)  tolua_tostring(tolua_S,2,0));
  const char* name = ((const char*)  tolua_tostring(tolua_S,3,0));
  {
   player* tolua_ret = (player*)  Mtolua_new((player)(ac,name));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"player");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  player */
#ifndef TOLUA_DISABLE_tolua_player_player_new00_local
static int tolua_player_player_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"player",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const char* ac = ((const char*)  tolua_tostring(tolua_S,2,0));
  const char* name = ((const char*)  tolua_tostring(tolua_S,3,0));
  {
   player* tolua_ret = (player*)  Mtolua_new((player)(ac,name));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"player");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  player */
#ifndef TOLUA_DISABLE_tolua_player_player_delete00
static int tolua_player_player_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"player",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: lua_schedule_timer of class  player */
#ifndef TOLUA_DISABLE_tolua_player_player_lua_schedule_timer00
static int tolua_player_player_lua_schedule_timer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"player",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'lua_schedule_timer'", NULL);
#endif
  {
   int tolua_ret = (int)  self->lua_schedule_timer();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'lua_schedule_timer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: career_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_career_
static int tolua_get_player_career_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'career_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->career_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: career_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_career_
static int tolua_set_player_career_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'career_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->career_ = ((char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: dir_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_dir_
static int tolua_get_player_dir_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dir_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->dir_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: dir_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_dir_
static int tolua_set_player_dir_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'dir_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->dir_ = ((char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: x_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_x_
static int tolua_get_player_x_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->x_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: x_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_x_
static int tolua_set_player_x_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x_ = ((short)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: y_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_y_
static int tolua_get_player_y_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'y_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->y_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: y_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_y_
static int tolua_set_player_y_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'y_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->y_ = ((short)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: lvl_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_lvl_
static int tolua_get_player_lvl_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'lvl_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->lvl_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: lvl_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_lvl_
static int tolua_set_player_lvl_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'lvl_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->lvl_ = ((short)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: char_id_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_char_id_
static int tolua_get_player_char_id_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'char_id_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->char_id_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: char_id_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_char_id_
static int tolua_set_player_char_id_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'char_id_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->char_id_ = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: scene_cid_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_scene_cid_
static int tolua_get_player_scene_cid_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'scene_cid_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->scene_cid_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: scene_cid_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_scene_cid_
static int tolua_set_player_scene_cid_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'scene_cid_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->scene_cid_ = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: last_heart_beat_time_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_last_heart_beat_time_
static int tolua_get_player_last_heart_beat_time_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'last_heart_beat_time_'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->last_heart_beat_time_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: last_heart_beat_time_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_last_heart_beat_time_
static int tolua_set_player_last_heart_beat_time_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'last_heart_beat_time_'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->last_heart_beat_time_ = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: exp_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_exp_
static int tolua_get_player_exp_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'exp_'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->exp_,"int64_t");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: exp_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_exp_
static int tolua_set_player_exp_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'exp_'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"int64_t",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->exp_ = *((int64_t*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: name_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_name_
static int tolua_get_player_name_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name_'",NULL);
#endif
  tolua_pushstring(tolua_S,(const char*)self->name_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: name_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_name_
static int tolua_set_player_name_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'name_'",NULL);
  if (!tolua_istable(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
 strncpy((char*)
self->name_,(const char*)tolua_tostring(tolua_S,2,0),32-1);
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: account_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_account_
static int tolua_get_player_account_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'account_'",NULL);
#endif
  tolua_pushstring(tolua_S,(const char*)self->account_);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: account_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_account_
static int tolua_set_player_account_(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'account_'",NULL);
  if (!tolua_istable(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
 strncpy((char*)
self->account_,(const char*)tolua_tostring(tolua_S,2,0),32-1);
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: player_snap_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_player_snap__ptr
static int tolua_get_player_player_snap__ptr(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'player_snap_'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->player_snap_,"snap_obj_mgr");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: player_snap_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_player_snap__ptr
static int tolua_set_player_player_snap__ptr(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'player_snap_'",NULL);
  if (!tolua_isusertype(tolua_S,2,"snap_obj_mgr",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->player_snap_ = ((snap_obj_mgr*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: monster_snap_ of class  player */
#ifndef TOLUA_DISABLE_tolua_get_player_monster_snap__ptr
static int tolua_get_player_monster_snap__ptr(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'monster_snap_'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->monster_snap_,"snap_obj_mgr");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: monster_snap_ of class  player */
#ifndef TOLUA_DISABLE_tolua_set_player_monster_snap__ptr
static int tolua_set_player_monster_snap__ptr(lua_State* tolua_S)
{
  player* self = (player*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'monster_snap_'",NULL);
  if (!tolua_isusertype(tolua_S,2,"snap_obj_mgr",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->monster_snap_ = ((snap_obj_mgr*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_player_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_cclass(tolua_S,"time_util","time_util","",NULL);
  tolua_beginmodule(tolua_S,"time_util");
   tolua_variable(tolua_S,"now",tolua_get_time_util_now,tolua_set_time_util_now);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"client","client","",NULL);
  tolua_beginmodule(tolua_S,"client");
   tolua_function(tolua_S,"send_request",tolua_player_client_send_request00);
   tolua_function(tolua_S,"send_respond",tolua_player_client_send_respond00);
   tolua_function(tolua_S,"send_respond_ok",tolua_player_client_send_respond_ok00);
   tolua_function(tolua_S,"send_respond_err",tolua_player_client_send_respond_err00);
   tolua_function(tolua_S,"send_msg",tolua_player_client_send_msg00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"snap_obj","snap_obj","",tolua_collect_snap_obj);
  #else
  tolua_cclass(tolua_S,"snap_obj","snap_obj","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"snap_obj");
   tolua_function(tolua_S,"new",tolua_player_snap_obj_new00);
   tolua_function(tolua_S,"new_local",tolua_player_snap_obj_new00_local);
   tolua_function(tolua_S,".call",tolua_player_snap_obj_new00_local);
   tolua_variable(tolua_S,"dir_",tolua_get_snap_obj_dir_,tolua_set_snap_obj_dir_);
   tolua_variable(tolua_S,"coord_x_",tolua_get_snap_obj_coord_x_,tolua_set_snap_obj_coord_x_);
   tolua_variable(tolua_S,"coord_y_",tolua_get_snap_obj_coord_y_,tolua_set_snap_obj_coord_y_);
   tolua_variable(tolua_S,"obj_id_",tolua_get_snap_obj_obj_id_,tolua_set_snap_obj_obj_id_);
   tolua_variable(tolua_S,"obj_cid_",tolua_get_snap_obj_obj_cid_,tolua_set_snap_obj_obj_cid_);
   tolua_variable(tolua_S,"scene_id_",tolua_get_snap_obj_scene_id_,tolua_set_snap_obj_scene_id_);
   tolua_variable(tolua_S,"scene_cid_",tolua_get_snap_obj_scene_cid_,tolua_set_snap_obj_scene_cid_);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"snap_obj_mgr","snap_obj_mgr","",tolua_collect_snap_obj_mgr);
  #else
  tolua_cclass(tolua_S,"snap_obj_mgr","snap_obj_mgr","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"snap_obj_mgr");
   tolua_function(tolua_S,"new",tolua_player_snap_obj_mgr_new00);
   tolua_function(tolua_S,"new_local",tolua_player_snap_obj_mgr_new00_local);
   tolua_function(tolua_S,".call",tolua_player_snap_obj_mgr_new00_local);
   tolua_function(tolua_S,"insert",tolua_player_snap_obj_mgr_insert00);
   tolua_function(tolua_S,"find",tolua_player_snap_obj_mgr_find00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"player","player","client",tolua_collect_player);
  #else
  tolua_cclass(tolua_S,"player","player","client",NULL);
  #endif
  tolua_beginmodule(tolua_S,"player");
   tolua_function(tolua_S,"new",tolua_player_player_new00);
   tolua_function(tolua_S,"new_local",tolua_player_player_new00_local);
   tolua_function(tolua_S,".call",tolua_player_player_new00_local);
   tolua_function(tolua_S,"delete",tolua_player_player_delete00);
   tolua_function(tolua_S,"lua_schedule_timer",tolua_player_player_lua_schedule_timer00);
   tolua_variable(tolua_S,"career_",tolua_get_player_career_,tolua_set_player_career_);
   tolua_variable(tolua_S,"dir_",tolua_get_player_dir_,tolua_set_player_dir_);
   tolua_variable(tolua_S,"x_",tolua_get_player_x_,tolua_set_player_x_);
   tolua_variable(tolua_S,"y_",tolua_get_player_y_,tolua_set_player_y_);
   tolua_variable(tolua_S,"lvl_",tolua_get_player_lvl_,tolua_set_player_lvl_);
   tolua_variable(tolua_S,"char_id_",tolua_get_player_char_id_,tolua_set_player_char_id_);
   tolua_variable(tolua_S,"scene_cid_",tolua_get_player_scene_cid_,tolua_set_player_scene_cid_);
   tolua_variable(tolua_S,"last_heart_beat_time_",tolua_get_player_last_heart_beat_time_,tolua_set_player_last_heart_beat_time_);
   tolua_variable(tolua_S,"exp_",tolua_get_player_exp_,tolua_set_player_exp_);
   tolua_variable(tolua_S,"name_",tolua_get_player_name_,tolua_set_player_name_);
   tolua_variable(tolua_S,"account_",tolua_get_player_account_,tolua_set_player_account_);
   tolua_variable(tolua_S,"player_snap_",tolua_get_player_player_snap__ptr,tolua_set_player_player_snap__ptr);
   tolua_variable(tolua_S,"monster_snap_",tolua_get_player_monster_snap__ptr,tolua_set_player_monster_snap__ptr);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_player (lua_State* tolua_S) {
 return tolua_player_open(tolua_S);
};
#endif


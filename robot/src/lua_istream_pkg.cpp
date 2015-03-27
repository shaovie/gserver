/*
** Lua binding: lua_istream
** Generated automatically by tolua++-1.0.92 on Mon Dec 30 16:21:40 2013.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_lua_istream_open (lua_State* tolua_S);

#include "lua_istream.h"

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_in_stream (lua_State* tolua_S)
{
 in_stream* self = (in_stream*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_out_stream (lua_State* tolua_S)
{
 out_stream* self = (out_stream*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_int64_t (lua_State* tolua_S)
{
 int64_t* self = (int64_t*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"in_stream");
 tolua_usertype(tolua_S,"lua_out_stream");
 tolua_usertype(tolua_S,"out_stream");
 tolua_usertype(tolua_S,"lua_in_stream");
 tolua_usertype(tolua_S,"int64_t");
}

/* method: new of class  in_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_in_stream_new00
static int tolua_lua_istream_in_stream_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"in_stream",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  char* bf = ((char*)  tolua_tostring(tolua_S,2,0));
  unsigned const int len = ((unsigned const int)  tolua_tonumber(tolua_S,3,0));
  {
   in_stream* tolua_ret = (in_stream*)  Mtolua_new((in_stream)(bf,len));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"in_stream");
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

/* method: new_local of class  in_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_in_stream_new00_local
static int tolua_lua_istream_in_stream_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"in_stream",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  char* bf = ((char*)  tolua_tostring(tolua_S,2,0));
  unsigned const int len = ((unsigned const int)  tolua_tonumber(tolua_S,3,0));
  {
   in_stream* tolua_ret = (in_stream*)  Mtolua_new((in_stream)(bf,len));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"in_stream");
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

/* method: length of class  in_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_in_stream_length00
static int tolua_lua_istream_in_stream_length00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"in_stream",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  in_stream* self = (in_stream*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'length'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->length();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'length'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: read_char of class  lua_in_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_in_stream_read_char00
static int tolua_lua_istream_lua_in_stream_read_char00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_in_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"in_stream",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  in_stream* is = ((in_stream*)  tolua_tousertype(tolua_S,2,0));
  {
   char tolua_ret = (char)  lua_in_stream::read_char(*is);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'read_char'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: read_short of class  lua_in_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_in_stream_read_short00
static int tolua_lua_istream_lua_in_stream_read_short00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_in_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"in_stream",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  in_stream* is = ((in_stream*)  tolua_tousertype(tolua_S,2,0));
  {
   short tolua_ret = (short)  lua_in_stream::read_short(*is);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'read_short'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: read_int of class  lua_in_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_in_stream_read_int00
static int tolua_lua_istream_lua_in_stream_read_int00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_in_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"in_stream",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  in_stream* is = ((in_stream*)  tolua_tousertype(tolua_S,2,0));
  {
   int tolua_ret = (int)  lua_in_stream::read_int(*is);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'read_int'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: read_double of class  lua_in_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_in_stream_read_double00
static int tolua_lua_istream_lua_in_stream_read_double00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_in_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"in_stream",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  in_stream* is = ((in_stream*)  tolua_tousertype(tolua_S,2,0));
  {
   double tolua_ret = (double)  lua_in_stream::read_double(*is);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'read_double'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: read_int64_t of class  lua_in_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_in_stream_read_int64_t00
static int tolua_lua_istream_lua_in_stream_read_int64_t00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_in_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"in_stream",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  in_stream* is = ((in_stream*)  tolua_tousertype(tolua_S,2,0));
  {
   int64_t tolua_ret = (int64_t)  lua_in_stream::read_int64_t(*is);
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((int64_t)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"int64_t");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(int64_t));
     tolua_pushusertype(tolua_S,tolua_obj,"int64_t");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'read_int64_t'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: read_string of class  lua_in_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_in_stream_read_string00
static int tolua_lua_istream_lua_in_stream_read_string00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_in_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"in_stream",0,&tolua_err)) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  in_stream* is = ((in_stream*)  tolua_tousertype(tolua_S,2,0));
  const int len = ((const int)  tolua_tonumber(tolua_S,3,0));
  {
   char* tolua_ret = (char*)  lua_in_stream::read_string(*is,len);
   tolua_pushstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'read_string'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: delete of class  out_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_out_stream_delete00
static int tolua_lua_istream_out_stream_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"out_stream",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  out_stream* self = (out_stream*)  tolua_tousertype(tolua_S,1,0);
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

/* method: new_out_stream of class  lua_out_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_out_stream_new_out_stream00
static int tolua_lua_istream_lua_out_stream_new_out_stream00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_out_stream",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   out_stream tolua_ret = (out_stream)  lua_out_stream::new_out_stream();
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((out_stream)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"out_stream");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(out_stream));
     tolua_pushusertype(tolua_S,tolua_obj,"out_stream");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new_out_stream'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: write_char of class  lua_out_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_out_stream_write_char00
static int tolua_lua_istream_lua_out_stream_write_char00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_out_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"out_stream",0,&tolua_err)) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  out_stream* os = ((out_stream*)  tolua_tousertype(tolua_S,2,0));
  const char value = ((const char)  tolua_tonumber(tolua_S,3,0));
  {
   lua_out_stream::write_char(*os,value);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'write_char'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: write_short of class  lua_out_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_out_stream_write_short00
static int tolua_lua_istream_lua_out_stream_write_short00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_out_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"out_stream",0,&tolua_err)) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  out_stream* os = ((out_stream*)  tolua_tousertype(tolua_S,2,0));
  const short value = ((const short)  tolua_tonumber(tolua_S,3,0));
  {
   lua_out_stream::write_short(*os,value);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'write_short'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: write_int of class  lua_out_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_out_stream_write_int00
static int tolua_lua_istream_lua_out_stream_write_int00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_out_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"out_stream",0,&tolua_err)) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  out_stream* os = ((out_stream*)  tolua_tousertype(tolua_S,2,0));
  const int value = ((const int)  tolua_tonumber(tolua_S,3,0));
  {
   lua_out_stream::write_int(*os,value);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'write_int'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: write_double of class  lua_out_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_out_stream_write_double00
static int tolua_lua_istream_lua_out_stream_write_double00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_out_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"out_stream",0,&tolua_err)) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  out_stream* os = ((out_stream*)  tolua_tousertype(tolua_S,2,0));
  const double value = ((const double)  tolua_tonumber(tolua_S,3,0));
  {
   lua_out_stream::write_double(*os,value);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'write_double'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: write_int64_t of class  lua_out_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_out_stream_write_int64_t00
static int tolua_lua_istream_lua_out_stream_write_int64_t00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_out_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"out_stream",0,&tolua_err)) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !tolua_isusertype(tolua_S,3,"const int64_t",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  out_stream* os = ((out_stream*)  tolua_tousertype(tolua_S,2,0));
  const int64_t value = *((const int64_t*)  tolua_tousertype(tolua_S,3,0));
  {
   lua_out_stream::write_int64_t(*os,value);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'write_int64_t'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: write_string of class  lua_out_stream */
#ifndef TOLUA_DISABLE_tolua_lua_istream_lua_out_stream_write_string00
static int tolua_lua_istream_lua_out_stream_write_string00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"lua_out_stream",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"out_stream",0,&tolua_err)) ||
     !tolua_isstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  out_stream* os = ((out_stream*)  tolua_tousertype(tolua_S,2,0));
  const char* bf = ((const char*)  tolua_tostring(tolua_S,3,0));
  {
   lua_out_stream::write_string(*os,bf);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'write_string'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_lua_istream_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"in_stream","in_stream","",tolua_collect_in_stream);
  #else
  tolua_cclass(tolua_S,"in_stream","in_stream","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"in_stream");
   tolua_function(tolua_S,"new",tolua_lua_istream_in_stream_new00);
   tolua_function(tolua_S,"new_local",tolua_lua_istream_in_stream_new00_local);
   tolua_function(tolua_S,".call",tolua_lua_istream_in_stream_new00_local);
   tolua_function(tolua_S,"length",tolua_lua_istream_in_stream_length00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"lua_in_stream","lua_in_stream","",NULL);
  tolua_beginmodule(tolua_S,"lua_in_stream");
   tolua_function(tolua_S,"read_char",tolua_lua_istream_lua_in_stream_read_char00);
   tolua_function(tolua_S,"read_short",tolua_lua_istream_lua_in_stream_read_short00);
   tolua_function(tolua_S,"read_int",tolua_lua_istream_lua_in_stream_read_int00);
   tolua_function(tolua_S,"read_double",tolua_lua_istream_lua_in_stream_read_double00);
   tolua_function(tolua_S,"read_int64_t",tolua_lua_istream_lua_in_stream_read_int64_t00);
   tolua_function(tolua_S,"read_string",tolua_lua_istream_lua_in_stream_read_string00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"out_stream","out_stream","",tolua_collect_out_stream);
  #else
  tolua_cclass(tolua_S,"out_stream","out_stream","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"out_stream");
   tolua_function(tolua_S,"delete",tolua_lua_istream_out_stream_delete00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"lua_out_stream","lua_out_stream","",NULL);
  tolua_beginmodule(tolua_S,"lua_out_stream");
   tolua_function(tolua_S,"new_out_stream",tolua_lua_istream_lua_out_stream_new_out_stream00);
   tolua_function(tolua_S,"write_char",tolua_lua_istream_lua_out_stream_write_char00);
   tolua_function(tolua_S,"write_short",tolua_lua_istream_lua_out_stream_write_short00);
   tolua_function(tolua_S,"write_int",tolua_lua_istream_lua_out_stream_write_int00);
   tolua_function(tolua_S,"write_double",tolua_lua_istream_lua_out_stream_write_double00);
   tolua_function(tolua_S,"write_int64_t",tolua_lua_istream_lua_out_stream_write_int64_t00);
   tolua_function(tolua_S,"write_string",tolua_lua_istream_lua_out_stream_write_string00);
  tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_lua_istream (lua_State* tolua_S) {
 return tolua_lua_istream_open(tolua_S);
};
#endif


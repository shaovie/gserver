// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw <shaovie@gmail.com>
 * Date     : 2009-05-29 02:24
 */
//========================================================================

#ifndef UNUSED_ARG_H_
#define UNUSED_ARG_H_

// = unused_arg
template <typename T1> 
inline void unused_args(const T1&) { }
template <typename T1, typename T2> 
inline void unused_args(const T1&, const T2&) { }
template <typename T1, typename T2, typename T3> 
inline void unused_args(const T1&, const T2&, const T3&) { }
template <typename T1, typename T2, typename T3, typename T4> 
inline void unused_args(const T1&, const T2&, const T3&, const T4&) { }

#endif // NDK_UNUSED_ARG_H_

// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-04-27 17:24
 */
//========================================================================

#ifndef MACROS_H_
#define MACROS_H_

// Bit operation macros 
#define BIT_ENABLED(WORD, BIT)  (((WORD) & (BIT)) != 0)
#define BIT_DISABLED(WORD, BIT) (((WORD) & (BIT)) == 0)

#define SET_BITS(WORD, BITS) (WORD |= (BITS))
#define CLR_BITS(WORD, BITS) (WORD &= ~(BITS))

#endif // MACROS_H_


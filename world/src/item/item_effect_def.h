// -*- C++ -*-

//========================================================================
/**
 * Author   : cuisw
 * Date     : 2012-09-26 11:47
 */
//========================================================================

#ifndef ITEM_EFFECT_DEF_H_
#define ITEM_EFFECT_DEF_H_

// Forward declarations

#define MAX_EFFECT_NAME_LEN   32

/**
 * @class item_effect
 *
 * @brief
 */
class item_effect
{
public:
  enum
  {
    CSM_EFFECT_NULL = 0,
    CSM_EFFECT_RECOVER_HP,
    CSM_EFFECT_RECOVER_MP,
    CSM_EFFECT_RECOVER_HP_PER,
    CSM_EFFECT_RECOVER_MP_PER,
    CSM_EFFECT_RECOVER_TL,
    CSM_EFFECT_GET_COIN,
    CSM_EFFECT_GIFT_ALL,
    CSM_EFFECT_SIN_DECREASE,
    CSM_EFFECT_BACK_TOWN,
    CSM_EFFECT_RANDOM_TRANSFER,
    CSM_EFFECT_GIFT_ONE,
    CSM_EFFECT_ZHANXING_FU,
    CSM_EFFECT_VIP_EXP,
    CSM_EFFECT_BOSS,
    CSM_EFFECT_ACTIVITY_BOX,
    CSM_EFFECT_ACTIVITY_KEY,
    EFFECT_END
  };
  static int match_id(const char *);

  int id;
  const char *desc;
};

#endif // ITEM_EFFECT_DEF_H_


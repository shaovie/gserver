#include "item_effect_def.h"

#include <cstring>

item_effect item_effect_list[] = {
  {item_effect::CSM_EFFECT_NULL,          "nullb"},
  {item_effect::CSM_EFFECT_RECOVER_HP,    "recover_sm"},
  {item_effect::CSM_EFFECT_RECOVER_MP,    "recover_fl"},
  {item_effect::CSM_EFFECT_RECOVER_HP_PER,"recover_sm_p"},
  {item_effect::CSM_EFFECT_RECOVER_MP_PER,"recover_fl_p"},
  {item_effect::CSM_EFFECT_RECOVER_TL,    "recover_tl"},
  {item_effect::CSM_EFFECT_GET_COIN,      "get_coin"},
  {item_effect::CSM_EFFECT_GIFT_ALL,      "gift_all"},
  {item_effect::CSM_EFFECT_SIN_DECREASE,  "sin_decrease"},
  {item_effect::CSM_EFFECT_BACK_TOWN,     "back_town"},
  {item_effect::CSM_EFFECT_RANDOM_TRANSFER,"random_transfer"},
  {item_effect::CSM_EFFECT_GIFT_ONE,      "gift_one"},
  {item_effect::CSM_EFFECT_ZHANXING_FU,   "zhanxing_fu"},
  {item_effect::CSM_EFFECT_VIP_EXP,       "vip_exp"},
  {item_effect::CSM_EFFECT_BOSS,          "boss"},
  {item_effect::CSM_EFFECT_ACTIVITY_BOX,  "bao_xiang"},
  {item_effect::CSM_EFFECT_ACTIVITY_KEY,  "bao_xiang_key"},
  {item_effect::EFFECT_END,               "nulle"}
};

int item_effect::match_id(const char *name)
{
  for (int i = 0; i < item_effect::EFFECT_END; ++i)
  {
    if (::strncmp(item_effect_list[i].desc,
                  name,
                  MAX_EFFECT_NAME_LEN) == 0)
      return item_effect_list[i].id;
  }
  return -1;
}
